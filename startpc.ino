#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include "html_templates.h"
// Inclua seu arquivo local de credenciais (NAO comitar). Copie de wifi_config.h.example
#include "wifi_config.h"

// ===== Hardware (ESP32 + 1K + 2N2222 ou simulacao com LED) =====
#define LED_PIN 2
#define POWER_OUT_PIN 23
#define PC_STATUS_PIN 34

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

// ===== Auto power-on apos retorno de energia =====
const bool AUTO_POWER_ON_AFTER_BOOT = true;
const unsigned long AUTO_POWER_DELAY_MS = 15000;
const unsigned long POWER_PULSE_MS = 450;
const unsigned long AP_RECONNECT_INTERVAL_MS = 60000;
const unsigned long WIFI_STABLE_BEFORE_POWER_MS = 3000;

WebServer server(80);

bool conectado = false;
bool modo_ap = false;
bool auto_power_executado = false;
unsigned long boot_ms = 0;
unsigned long ultimo_reconnect_ms = 0;
bool pc_ligado_simulado = false;
String status_operacional = "Iniciando";

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
void piscarLedStatus();

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

  // GPIO34 e somente entrada e nao tem pull-up interno.
  pinMode(PC_STATUS_PIN, INPUT);

  EEPROM.begin(EEPROM_SIZE);

  boot_ms = millis();

  Serial.println("\n=== ESP32 Auto PC ===");
  Serial.println("Iniciando conectividade...");

  conectado = conectarWiFi();

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
}

void loop() {
  server.handleClient();

  tentarReconectarNoWiFi();

  if (!modo_ap && WiFi.status() != WL_CONNECTED) {
    conectado = false;
    Serial.println("Wi-Fi caiu. Voltando para AP de configuracao.");
    criarAccessPoint();
  }

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
  if (SIMULATION_MODE) {
    return pc_ligado_simulado;
  }

  return digitalRead(PC_STATUS_PIN) == HIGH;
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
    return;
  }

  Serial.printf("PC desligado apos %s. Aguardando estabilizar Wi-Fi antes de ligar...\n", motivo);
  delay(WIFI_STABLE_BEFORE_POWER_MS);

  if (!pcLigado()) {
    Serial.printf("Acionando power do PC apos %s.\n", motivo);
    acionarBotaoPower();
  } else {
    Serial.printf("PC ligou sozinho antes do pulso apos %s.\n", motivo);
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
