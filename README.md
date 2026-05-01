# ESP32 Auto PC — Controle de Power para PC via ESP32

Projeto para ligar automaticamente um PC (ou simular) usando um ESP32 e um transistor 2N2222, com fallback para um Access Point de configuração via web e persistência de credenciais em EEPROM.

**O que é / Para que serve**
- Este projeto permite que um ESP32 acione fisicamente o botão power de um PC (via 2N2222) para religar o computador após falta de energia, ou por comando remoto via web.
- Inclui modo de simulação para testar com LEDs sem precisar conectar ao PC real.

**Principais recursos**
- Reconexão automática em múltiplas redes Wi‑Fi (prioridade entre duas SSIDs e credenciais salvas em EEPROM).
- Modo AP para configurar/alterar credenciais via interface web (`/`), com endpoint para salvar (`/salvar`).
- Endpoint `/status` que retorna JSON com estado atual (Wi‑Fi, IP, PC ligado, modo de simulação, etc.).
- Auto power-on: ao detectar que o ESP32 reiniciou depois da falta de energia, tenta ligar o PC automaticamente após delay configurável.
- Modo `SIMULATION_MODE` para testes sem hardware sensível.

**Arquivos principais**
- [startpc.ino](startpc.ino) — firmware principal (Wi‑Fi, servidor web, EEPROM, lógica de power).
- [html_templates.h](html_templates.h) — templates HTML usados pela interface web (página de configuração e painel principal).
- [wifi_config.h.example](wifi_config.h.example) — exemplo de arquivo com as credenciais (copie para `wifi_config.h`).
- [wifi_config.h](wifi_config.h) — arquivo local com suas credenciais (NÃO comitar). Está em `.gitignore`.
- [pinout_esp32_pc_power.puml](pinout_esp32_pc_power.puml) — diagrama PlantUML do mapeamento de pinos e ligação ao 2N2222.

**Hardware necessário**
- ESP32 (qualquer placa compatível).
- Transistor NPN 2N2222.
- Resistores: 1k para limitar corrente da GPIO ao transistor; resistor de pull se necessário.
- Fios, protoboard.
- (Opcional para testes) LED + resistor para `SIMULATION_MODE`.

Esquema de ligação (resumo)
- `GPIO23` -> resistor 1k -> base do 2N2222.
- Emissor do 2N2222 -> GND.
- Coletor do 2N2222 -> um dos terminais do conector do botão power do PC.
- Outro terminal do conector do botão power -> 5V ou Vsys conforme o botão do chassi (verifique o manual do gabinete). **NÃO** aplique 5V direto em GPIO.
- `GPIO34` é leitura do estado do PC (se disponível) — entrada somente leitura.

> Veja o diagrama detalhado em [pinout_esp32_pc_power.puml](pinout_esp32_pc_power.puml).

Segurança e aviso importante
- Trabalhar com a placa mãe e com o circuito do botão power envolve riscos. Desconecte a fonte antes de alterar fios no conector do botão.
- Nunca aplique 5V diretamente em um pino do ESP32.
- Se tiver dúvidas, teste primeiro com `SIMULATION_MODE = true` usando apenas um LED no `GPIO23`.

Configuração e instalação
1. Faça uma cópia do arquivo de exemplo de credenciais:

```bash
# No Windows / Linux (terminal):
cp wifi_config.h.example wifi_config.h
```

2. Abra `wifi_config.h` e substitua `WIFI_SSID1`, `WIFI_PASS1`, `WIFI_SSID2`, `WIFI_PASS2` pelas suas redes.
3. Mantenha `wifi_config.h` fora do repositório — ele já está listado em `.gitignore`.

Compilar e gravar
- Abra `startpc` no Arduino IDE (ou use `arduino-cli`) e selecione a placa ESP32 correta e a porta COM.
- Clique em `Upload`.

Exemplo com `arduino-cli` (ajuste `--fqbn` e `-p` conforme sua placa/porta):

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 .
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32 .
```

Como testar (modo simulação)
- Deixe `SIMULATION_MODE` como `true` em `startpc.ino` para usar o LED no `GPIO23` em vez do 2N2222 e um estado de PC virtual.
- Ao ligar o ESP32, abra a rede Wi‑Fi `ESP32-Config` (se o dispositivo entrou em AP) e acesse `http://192.168.4.1/`.
- Na interface web você pode salvar SSID/senha, ver status em tempo real (a UI faz polling em `/status`) e acionar o power manualmente.

Uso em produção (hardware real)
- Verifique as ligações do 2N2222 e confirme a polaridade do conector do botão power do gabinete.
- Teste primeiro com dispositivos que não corram risco (ex.: LED ou uma carga baixa) antes de conectar ao power switch.
- Quando estiver seguro, defina `SIMULATION_MODE = false` e carregue o firmware.

Como funciona (resumo técnico)
- Na inicialização o ESP32 tenta conectar nas redes em prioridade: `WIFI_SSID1`, `WIFI_SSID2`, depois em redes salvas na EEPROM.
- Se falhar, cria um Access Point (`ESP32-Config`) com página para configurar novas credenciais.
- Quando o Wi‑Fi volta após uma queda, o ESP32 espera um tempo de estabilização e então, se o PC estiver desligado, envia um pulso curto (`POWER_PULSE_MS`) ao `POWER_OUT_PIN` para acionar o botão via transistor.
- As credenciais salvas via web são gravadas na EEPROM (endereços definidos em `startpc.ino`).

Endpoints HTTP importantes
- `GET /` — página principal (ou página de configuração em modo AP).
- `POST /salvar` — salva SSID/senha na EEPROM.
- `GET /status` — retorna JSON com estado atual.
- `POST /power/toggle` — aciona o pulso de power.
- `POST /pc/simulate` — alterna o estado de PC em `SIMULATION_MODE`.
