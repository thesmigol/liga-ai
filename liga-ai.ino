#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include "html_templates.h"
// Inclua seu arquivo local de credenciais (NAO comitar). Copie de wifi_config.h.example
#include "wifi_config.h"
#include "ota_config.h"

// Compatibilidade: se a configuracao nova ainda nao existir em wifi_config.h,
// mantemos o build funcional com valor vazio.
#ifndef PC_HTTP_BASE_URL
#define PC_HTTP_BASE_URL ""

#endif

// ===== Hardware (ESP32 + 1K + 2N2222 ou simulacao com LED) =====
#define LED_PIN 2
#define POWER_OUT_PIN 23

// Quando true, o projeto roda sem o 2N2222 e sem o PC real.
// GPIO23 acende um LED com resistor de 1K e o estado do PC vira virtual.
const bool SIMULATION_MODE = SIMULATION;

// ===== EEPROM =====
#define EEPROM_SIZE 256
#define SSID_ADDR 0
#define SSID_LEN 32
#define PASS_ADDR 32
#define PASS_LEN 64

// ===== Wi-Fi fixo (prioridade) =====
// Valores definidos em wifi_config.h (copie wifi_config.h.example -> wifi_config.h)
const char* SSID1 = WIFI_SSID1;
const char* PASS1 = WIFI_PASS1;
const char* SSID2 = WIFI_SSID2;
const char* PASS2 = WIFI_PASS2;

// ===== AP fallback =====
const char* AP_SSID = WIFI_SSID3;
const char* AP_PASS = WIFI_PASS3;
const char* PC_SERVICE_BASE_URL = PC_HTTP_BASE_URL;

// ===== Auto power-on apos retorno de energia =====
const bool AUTO_POWER_ON_AFTER_BOOT = true;
const unsigned long AUTO_POWER_DELAY_MS = 30000;
const unsigned long POWER_PULSE_MS = 450;
const unsigned long AP_RECONNECT_INTERVAL_MS = 60000;
const unsigned long WIFI_STABLE_BEFORE_POWER_MS = 5000;
const unsigned long PC_HTTP_POLL_INTERVAL_MS = 10000;
const unsigned long PC_HTTP_MONITOR_TIMEOUT_MS = 360000;

WebServer server(80);

bool conectado = false;
bool modo_ap = false;
bool auto_power_executado = false;
unsigned long boot_ms = 0;
unsigned long ultimo_reconnect_ms = 0;
bool pc_ligado_simulado = false;
String status_operacional = "Iniciando";
bool pc_http_online = false;
bool pc_http_monitoring = false;
unsigned long pc_http_monitor_started_ms = 0;
unsigned long pc_http_last_poll_ms = 0;
String pc_http_last_result = "nao iniciado";

String ssid_ativo = "-";
String ip_ativo = "-";

String lerEEPROMString(int startAddr, int maxLen);
void salvarEEPROMString(int startAddr, int maxLen, const String& value);
bool tentarConectar(const char* ssid, const char* pass, int maxTentativas);
bool conectarWiFi();
void criarAccessPoint();
bool pcLigado();
void acionarBotaoPower();
void verificarAutoPower();
void tentarReconectarNoWiFi();
void tentarLigarPcSeNecessario(const char* motivo);
bool pcServiceConfigurado();
String montarPcServiceUrl(const char* path);
bool enviarEventoPc(const char* eventType, const char* motivo);
bool consultarStatusPcService();
void monitorarStatusPcService();
void piscarLedStatus();
void inicializarOTA();

void handleRoot();
void handleSalvarWiFi();
void handleStatus();
void handlePowerToggle();
void handleSimulatePc();

void setup() {
  Serial.begin(115200);
  delay(700);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(POWER_OUT_PIN, OUTPUT);
  digitalWrite(POWER_OUT_PIN, LOW);

  EEPROM.begin(EEPROM_SIZE);

  boot_ms = millis();

  Serial.println("\n=== ESP32 Auto PC ===");
  Serial.println("Iniciando conectividade...");

  conectado = conectarWiFi();

  if (conectado) {
    if (pcServiceConfigurado()) {
      Serial.printf("Servico PC-side configurado em: %s\n", PC_SERVICE_BASE_URL);
      enviarEventoPc("wifi_connected", "boot_wifi_connected");
    }
    Serial.println("Wi-Fi conectado na inicializacao; tentando ligar PC se necessario.");
    tentarLigarPcSeNecessario("boot_wifi");
  }

  if (!conectado) {
    Serial.println("Falha ao conectar nas redes. Entrando em modo AP.");
    criarAccessPoint();
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/salvar", HTTP_POST, handleSalvarWiFi);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/power/toggle", HTTP_POST, handlePowerToggle);
  server.on("/pc/simulate", HTTP_POST, handleSimulatePc);
  server.begin();

  Serial.println("Servidor HTTP iniciado.");

  // Inicializar OTA se Wi-Fi conectado e OTA habilitado
  if (conectado && OTA_ENABLED) {
    inicializarOTA();
  }
}

void loop() {
  server.handleClient();

  // Handle OTA updates
  if (OTA_ENABLED && conectado) {
    ArduinoOTA.handle();
  }

  tentarReconectarNoWiFi();

  if (!modo_ap && WiFi.status() != WL_CONNECTED) {
    conectado = false;
    pc_http_online = false;
    pc_http_monitoring = false;
    Serial.println("Wi-Fi caiu. Voltando para AP de configuracao.");
    criarAccessPoint();
  }

  monitorarStatusPcService();
  verificarAutoPower();
  piscarLedStatus();
}

String lerEEPROMString(int startAddr, int maxLen) {
  char data[maxLen + 1];
  int i = 0;

  for (; i < maxLen; i++) {
    char c = char(EEPROM.read(startAddr + i));
    if (c == '\0' || c == (char)0xFF) {
      break;
    }
    data[i] = c;
  }

  data[i] = '\0';
  return String(data);
}

void salvarEEPROMString(int startAddr, int maxLen, const String& value) {
  for (int i = 0; i < maxLen; i++) {
    if (i < value.length()) {
      EEPROM.write(startAddr + i, value[i]);
    } else {
      EEPROM.write(startAddr + i, 0);
    }
  }
}

bool tentarConectar(const char* ssid, const char* pass, int maxTentativas) {
  if (ssid == nullptr || strlen(ssid) == 0) {
    return false;
  }

  Serial.printf("Tentando SSID: %s\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  int t = maxTentativas;
  while (t-- > 0 && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    ssid_ativo = WiFi.SSID();
    ip_ativo = WiFi.localIP().toString();
    conectado = true;
    modo_ap = false;
    Serial.printf("Conectado em %s | IP %s\n", ssid_ativo.c_str(), ip_ativo.c_str());
    return true;
  }

  WiFi.disconnect(true, true);
  return false;
}

bool conectarWiFi() {
  if (tentarConectar(SSID1, PASS1, 20)) {
    return true;
  }

  if (tentarConectar(SSID2, PASS2, 20)) {
    return true;
  }

  String ssidMem = lerEEPROMString(SSID_ADDR, SSID_LEN);
  String passMem = lerEEPROMString(PASS_ADDR, PASS_LEN);

  if (ssidMem.length() > 0) {
    Serial.printf("Tentando SSID salvo: %s\n", ssidMem.c_str());
    if (tentarConectar(ssidMem.c_str(), passMem.c_str(), 20)) {
      return true;
    }
  }

  return false;
}

void criarAccessPoint() {
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  ssid_ativo = AP_SSID;
  ip_ativo = WiFi.softAPIP().toString();
  conectado = false;
  modo_ap = true;
  ultimo_reconnect_ms = millis();
  status_operacional = "Em AP, tentando reconectar em 60s";

  Serial.printf("AP ativo: %s | IP %s\n", AP_SSID, ip_ativo.c_str());
}

bool pcLigado() {
  // Modo simulacao
  if (SIMULATION_MODE) {
    return pc_ligado_simulado;
  }

  // Monitora apenas via HTTP (sem GPIO 34)
  // O status real do PC vem do polling /status do PC-side
  return pc_http_online;
}

void acionarBotaoPower() {
  if (SIMULATION_MODE) {
    Serial.println("Simulacao: LED de power acionado no GPIO23");
  } else {
    Serial.println("Pulso no POWER_OUT_PIN (via 2N2222)");
  }

  digitalWrite(POWER_OUT_PIN, HIGH);
  delay(POWER_PULSE_MS);
  digitalWrite(POWER_OUT_PIN, LOW);

  if (SIMULATION_MODE) {
    pc_ligado_simulado = true;
  }
}

void verificarAutoPower() {
  if (!AUTO_POWER_ON_AFTER_BOOT || auto_power_executado) {
    return;
  }

  if (millis() - boot_ms < AUTO_POWER_DELAY_MS) {
    return;
  }

  if (!pcLigado()) {
    status_operacional = "PC desligado, ligando agora";
    Serial.println("PC desligado apos boot do ESP32. Auto power-on acionado.");
    acionarBotaoPower();
    status_operacional = "PC confirmado ligado";
  } else {
    status_operacional = "PC confirmado ligado";
    Serial.println("PC ja esta ligado. Auto power-on ignorado.");
  }

  auto_power_executado = true;
}

void tentarReconectarNoWiFi() {
  if (!modo_ap || conectado) {
    return;
  }

  unsigned long elapsed = millis() - ultimo_reconnect_ms;
  if (elapsed < AP_RECONNECT_INTERVAL_MS) {
    unsigned long remaining = (AP_RECONNECT_INTERVAL_MS - elapsed) / 1000;
    status_operacional = "Em AP, tentando reconectar em " + String(remaining) + "s";
    return;
  }

  ultimo_reconnect_ms = millis();
  status_operacional = "Tentando voltar para o Wi-Fi";
  Serial.println("Tentando reconectar nos Wi-Fis salvos...");

  if (conectarWiFi()) {
    modo_ap = false;
    conectado = true;
    status_operacional = "Wi-Fi voltou, ligando PC";
    Serial.println("Reconexao Wi-Fi bem-sucedida. Saindo do AP.");
    tentarLigarPcSeNecessario("reconexao_wifi");
  } else {
    status_operacional = "Falha na reconexao, mantendo hotspot ativo";
    Serial.println("Reconexao Wi-Fi falhou. Mantendo hotspot ativo.");
    criarAccessPoint();
  }
}

void tentarLigarPcSeNecessario(const char* motivo) {
  if (pcLigado()) {
    Serial.printf("PC ja estava ligado apos %s. Nenhum pulso enviado.\n", motivo);
    if (pcServiceConfigurado()) {
      enviarEventoPc("power_skipped_pc_already_on", motivo);
    }
    return;
  }

  Serial.printf("PC desligado apos %s. Aguardando estabilizar Wi-Fi antes de ligar...\n", motivo);
  delay(WIFI_STABLE_BEFORE_POWER_MS);

  if (!pcLigado()) {
    Serial.printf("Acionando power do PC apos %s.\n", motivo);
    acionarBotaoPower();
    if (pcServiceConfigurado()) {
      enviarEventoPc("power_pulse_sent", motivo);
      pc_http_monitoring = true;
      pc_http_online = false;
      pc_http_monitor_started_ms = millis();
      pc_http_last_poll_ms = 0;
      pc_http_last_result = "aguardando /status do PC-side";
      status_operacional = "Pulso enviado; monitorando /status do PC";
    }
  } else {
    Serial.printf("PC ligou sozinho antes do pulso apos %s.\n", motivo);
    if (pcServiceConfigurado()) {
      enviarEventoPc("pc_ligou_sem_pulso", motivo);
    }
  }
}

bool pcServiceConfigurado() {
  return strlen(PC_SERVICE_BASE_URL) > 0;
}

String montarPcServiceUrl(const char* path) {
  String base = String(PC_SERVICE_BASE_URL);
  base.trim();

  if (base.length() == 0) {
    return "";
  }

  String p = String(path);
  if (!p.startsWith("/")) {
    p = "/" + p;
  }

  if (base.endsWith("/")) {
    base.remove(base.length() - 1);
  }

  return base + p;
}

bool enviarEventoPc(const char* eventType, const char* motivo) {
  if (!conectado || modo_ap || !pcServiceConfigurado()) {
    return false;
  }

  String url = montarPcServiceUrl("/event");
  if (url.length() == 0) {
    return false;
  }

  String motivoSafe = String(motivo);
  motivoSafe.replace("\"", "'");

  String ssidSafe = ssid_ativo;
  ssidSafe.replace("\"", "'");

  String ipSafe = ip_ativo;
  ipSafe.replace("\"", "'");

  String payload = "{";
  payload += "\"source\":\"esp32-auto-pc\",";
  payload += "\"event_type\":\"" + String(eventType) + "\",";
  payload += "\"data\":{";
  payload += "\"motivo\":\"" + motivoSafe + "\",";
  payload += "\"ssid\":\"" + ssidSafe + "\",";
  payload += "\"ip\":\"" + ipSafe + "\",";
  payload += "\"simulation_mode\":" + String(SIMULATION_MODE ? "true" : "false");
  payload += "}";
  payload += "}";

  HTTPClient http;
  http.setConnectTimeout(1500);
  http.setTimeout(2500);

  if (!http.begin(url)) {
    pc_http_last_result = "falha ao iniciar POST /event";
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    if (httpCode == 200) {
      pc_http_last_result = "POST /event OK";
      http.end();
      return true;
    }

    pc_http_last_result = "POST /event HTTP " + String(httpCode);
    http.end();
    return false;
  }

  pc_http_last_result = "POST /event erro: " + HTTPClient::errorToString(httpCode);
  http.end();
  return false;
}

bool consultarStatusPcService() {
  if (!conectado || modo_ap || !pcServiceConfigurado()) {
    return false;
  }

  String url = montarPcServiceUrl("/status");
  if (url.length() == 0) {
    return false;
  }

  HTTPClient http;
  http.setConnectTimeout(1500);
  http.setTimeout(2500);

  if (!http.begin(url)) {
    pc_http_last_result = "falha ao iniciar GET /status";
    return false;
  }

  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == 200) {
      pc_http_last_result = "GET /status OK";
      http.end();
      return true;
    }

    pc_http_last_result = "GET /status HTTP " + String(httpCode);
    http.end();
    return false;
  }

  pc_http_last_result = "GET /status erro: " + HTTPClient::errorToString(httpCode);
  http.end();
  return false;
}

void monitorarStatusPcService() {
  if (!pc_http_monitoring) {
    return;
  }

  if (!conectado || modo_ap) {
    pc_http_monitoring = false;
    pc_http_online = false;
    pc_http_last_result = "monitor interrompido (sem Wi-Fi)";
    return;
  }

  unsigned long elapsed = millis() - pc_http_monitor_started_ms;
  if (elapsed > PC_HTTP_MONITOR_TIMEOUT_MS) {
    pc_http_monitoring = false;
    pc_http_online = false;
    pc_http_last_result = "timeout aguardando /status";
    status_operacional = "Timeout no monitoramento HTTP do PC";
    return;
  }

  if (millis() - pc_http_last_poll_ms < PC_HTTP_POLL_INTERVAL_MS) {
    return;
  }

  pc_http_last_poll_ms = millis();

  if (consultarStatusPcService()) {
    pc_http_online = true;
    pc_http_monitoring = false;
    status_operacional = "PC-side online confirmado via HTTP";
    Serial.println("PC-side respondeu /status com sucesso.");
    enviarEventoPc("pc_online_confirmed", "http_status_ok");
  } else {
    status_operacional = "Aguardando /status do PC-side";
  }
}

void piscarLedStatus() {
  if (modo_ap) {
    digitalWrite(LED_PIN, (millis() / 350) % 2);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}

void handleRoot() {
  if (modo_ap) {
    server.send(200, "text/html; charset=utf-8", HTML_CONFIG_PAGE);
    return;
  }

  server.send(200, "text/html; charset=utf-8", HTML_HOME_PAGE);
}

void handleSalvarWiFi() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  if (ssid.length() == 0 || pass.length() == 0) {
    server.send(400, "text/plain", "SSID/senha invalidos");
    return;
  }

  salvarEEPROMString(SSID_ADDR, SSID_LEN, ssid);
  salvarEEPROMString(PASS_ADDR, PASS_LEN, pass);
  EEPROM.commit();

  Serial.printf("Nova rede salva: %s\n", ssid.c_str());

  server.send(200, "text/html; charset=utf-8", HTML_SUCCESS_PAGE);
  delay(1200);
  ESP.restart();
}

void handleStatus() {
  bool pc_on = pcLigado();
  String json = "{";
  json += "\"ssid\":\"" + ssid_ativo + "\",";
  json += "\"ip\":\"" + ip_ativo + "\",";
  json += "\"pc_on\":" + String(pc_on ? "true" : "false") + ",";
  json += "\"auto_power_enabled\":" + String(AUTO_POWER_ON_AFTER_BOOT ? "true" : "false") + ",";
  json += "\"auto_power_executado\":" + String(auto_power_executado ? "true" : "false") + ",";
  json += "\"modo_ap\":" + String(modo_ap ? "true" : "false") + ",";
  json += "\"simulation_mode\":" + String(SIMULATION_MODE ? "true" : "false") + ",";
  json += "\"pc_http_base_url\":\"" + String(PC_SERVICE_BASE_URL) + "\",";
  json += "\"pc_http_online\":" + String(pc_http_online ? "true" : "false") + ",";
  json += "\"pc_http_monitoring\":" + String(pc_http_monitoring ? "true" : "false") + ",";
  json += "\"pc_http_last_result\":\"" + pc_http_last_result + "\",";
  json += "\"status_operacional\":\"" + status_operacional + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handlePowerToggle() {
  acionarBotaoPower();
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleSimulatePc() {
  if (!SIMULATION_MODE) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"simulation_disabled\"}");
    return;
  }

  String state = server.arg("state");
  if (state == "on") {
    pc_ligado_simulado = true;
  } else if (state == "off") {
    pc_ligado_simulado = false;
  } else {
    pc_ligado_simulado = !pc_ligado_simulado;
  }

  server.send(200, "application/json", "{\"ok\":true}");
}

void inicializarOTA() {
  if (!OTA_ENABLED) {
    return;
  }

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "firmware" : "filesystem";
    Serial.println("\n>>> Iniciando atualizacao OTA: " + type);
    status_operacional = "Atualizando OTA: " + type;
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n>>> OTA concluido com sucesso!");
    status_operacional = "OTA concluido. Reiniciando...";
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (OTA_DEBUG) {
      Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro OTA [%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
    status_operacional = "Erro em OTA";
  });

  ArduinoOTA.begin();
  Serial.printf("OTA pronto em http://%s.local:%u\n", OTA_HOSTNAME, OTA_PORT);
  Serial.printf("Hostname OTA: %s\n", OTA_HOSTNAME);
}
