// ===== Configuração OTA (Over-The-Air Updates) =====
// Este arquivo centraliza as configurações de atualização remota via OTA.
// OTA permite fazer upload de novo firmware via Wi-Fi sem desconectar fisicamente o USB.

#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

// ===== Ativar/desativar OTA =====
// Deixe true para permitir atualizações OTA. Desative se quiser economia de memória.
#define OTA_ENABLED true

// ===== Credenciais OTA =====
// Senhas para proteger o upload OTA. Use senhas fortes em produção.
// Formato: const char* OTA_PASSWORD = "sua_senha_aqui";
#define OTA_PASSWORD "12345678"

// ===== Nome do hostname para OTA =====
// Este nome aparecerá nas ferramentas de upload (arduino-cli, Visual Studio Code, etc.)
// Recomendação: use algo único na rede (ex.: "esp32-auto-pc")
#define OTA_HOSTNAME "esp32-auto-pc"

// ===== Portas =====
// Porta padrão OTA é 3232 (Arduino).
// Você pode usar outra porta se houver conflito.
#define OTA_PORT 3232

// ===== Tamanho máximo de atualização =====
// Limite de tamanho do firmware. ESP32 típico: 1.3 MB (1327104 bytes).
// Não altere a menos que saiba exatamente qual é o seu limite.
#define OTA_MAX_SIZE 1327104  // ~1.3 MB

// ===== Debug OTA =====
// Se true, imprime logs detalhados de progresso de OTA no Serial.
#define OTA_DEBUG true

#endif // OTA_CONFIG_H
