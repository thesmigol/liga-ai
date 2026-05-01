#ifndef HTML_TEMPLATES_H
#define HTML_TEMPLATES_H

const char HTML_CONFIG_PAGE[] = R"(
<!DOCTYPE html>
<html lang='pt-BR'>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Configurar Wi-Fi | ESP32</title>
  <style>
    :root {
      --bg1: #07203f;
      --bg2: #0f4c75;
      --panel: #ffffff;
      --ink: #11263f;
      --muted: #4a6079;
      --accent: #f2994a;
      --accent2: #2d9cdb;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      display: grid;
      place-items: center;
      font-family: 'Segoe UI', Tahoma, sans-serif;
      color: var(--ink);
      background: radial-gradient(circle at 20% 20%, #1b6ca8 0%, var(--bg2) 45%, var(--bg1) 100%);
      padding: 16px;
    }
    .card {
      width: min(100%, 460px);
      background: var(--panel);
      border-radius: 18px;
      box-shadow: 0 20px 45px rgba(0, 0, 0, 0.28);
      padding: 26px;
    }
    h1 { margin: 0 0 8px; font-size: 1.5rem; }
    p { margin: 0 0 18px; color: var(--muted); }
    label { display: block; font-size: 0.86rem; color: var(--muted); margin: 8px 0 4px; }
    input {
      width: 100%;
      padding: 12px;
      border-radius: 10px;
      border: 1px solid #c9d3dd;
      font-size: 1rem;
      outline: none;
    }
    input:focus { border-color: var(--accent2); box-shadow: 0 0 0 3px rgba(45, 156, 219, 0.15); }
    button {
      width: 100%;
      border: 0;
      border-radius: 12px;
      margin-top: 16px;
      padding: 12px;
      cursor: pointer;
      color: #fff;
      background: linear-gradient(135deg, var(--accent), #eb5757);
      font-weight: 700;
      font-size: 1rem;
    }
    .tip {
      margin-top: 16px;
      border-left: 4px solid var(--accent2);
      background: #eff8ff;
      border-radius: 10px;
      padding: 12px;
      font-size: 0.9rem;
      color: var(--muted);
    }
  </style>
</head>
<body>
  <section class='card'>
    <h1>Configurar Wi-Fi</h1>
    <p>Informe a rede para o ESP32 conectar e liberar o painel de controle.</p>
    <form action='/salvar' method='POST'>
      <label for='ssid'>Nome da rede (SSID)</label>
      <input id='ssid' name='ssid' type='text' required autofocus>
      <label for='pass'>Senha</label>
      <input id='pass' name='pass' type='password' required>
      <button type='submit'>Salvar e Reiniciar</button>
    </form>
    <div class='tip'>
      Se esta tela abriu, o ESP32 nao conseguiu conectar nas redes prioritarias e entrou em modo configuracao.
    </div>
  </section>
</body>
</html>
)";

const char HTML_HOME_PAGE[] = R"(
<!DOCTYPE html>
<html lang='pt-BR'>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Home | ESP32 Auto PC</title>
  <style>
    :root {
      --bg: #0b132b;
      --ink: #e9f0ff;
      --muted: #b8c6e3;
      --panel: #121b36;
      --ok: #27ae60;
      --off: #eb5757;
      --warn: #f2c94c;
      --line: #5c6ea8;
      --chip: #2d9cdb;
      --transistor: #9b51e0;
      --resistor: #f2994a;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      background: radial-gradient(circle at 25% 15%, #1c2d61 0%, #0f1d45 45%, var(--bg) 100%);
      color: var(--ink);
      font-family: 'Segoe UI', Tahoma, sans-serif;
      min-height: 100vh;
      padding: 18px;
    }
    .shell {
      max-width: 1080px;
      margin: 0 auto;
      display: grid;
      gap: 14px;
      grid-template-columns: 1.1fr 0.9fr;
    }
    .panel {
      background: rgba(18, 27, 54, 0.88);
      border: 1px solid rgba(146, 168, 218, 0.25);
      border-radius: 16px;
      padding: 16px;
      box-shadow: 0 16px 32px rgba(0, 0, 0, 0.28);
    }
    h1 { margin: 0 0 8px; font-size: 1.3rem; }
    .muted { color: var(--muted); font-size: 0.92rem; }
    .grid {
      margin-top: 12px;
      display: grid;
      gap: 10px;
      grid-template-columns: repeat(2, 1fr);
    }
    .metric {
      background: rgba(255, 255, 255, 0.05);
      border-radius: 12px;
      padding: 10px;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
    .metric .k { color: var(--muted); font-size: 0.8rem; }
    .metric .v { margin-top: 3px; font-size: 1.05rem; font-weight: 700; }
    .badge {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 6px 10px;
      border-radius: 999px;
      font-weight: 700;
      font-size: 0.8rem;
      margin-top: 6px;
    }
    .ok { background: rgba(39, 174, 96, 0.18); color: #8de4b3; border: 1px solid rgba(39, 174, 96, 0.35); }
    .off { background: rgba(235, 87, 87, 0.18); color: #ffb4b4; border: 1px solid rgba(235, 87, 87, 0.35); }
    .warn { background: rgba(242, 201, 76, 0.18); color: #ffe8a3; border: 1px solid rgba(242, 201, 76, 0.35); }
    button {
      margin-top: 12px;
      border: 0;
      border-radius: 12px;
      padding: 11px 14px;
      font-weight: 700;
      color: #fff;
      cursor: pointer;
      background: linear-gradient(135deg, #f2994a, #eb5757);
    }
    .log {
      margin-top: 10px;
      font-size: 0.85rem;
      color: var(--muted);
      min-height: 1.2em;
    }
    .status-banner {
      margin-top: 12px;
      padding: 14px 16px;
      border-radius: 14px;
      background: linear-gradient(135deg, rgba(45, 156, 219, 0.2), rgba(39, 174, 96, 0.16));
      border: 1px solid rgba(141, 228, 179, 0.28);
      color: #eef7ff;
      font-size: 1rem;
      font-weight: 700;
      letter-spacing: 0.2px;
    }
    .status-sub {
      display: block;
      margin-top: 4px;
      font-size: 0.82rem;
      color: #b8c6e3;
      font-weight: 500;
    }
    .components {
      margin-top: 12px;
      display: grid;
      gap: 10px;
      grid-template-columns: repeat(2, minmax(0, 1fr));
    }
    .component-card {
      border-radius: 14px;
      padding: 10px;
      background: rgba(255, 255, 255, 0.05);
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
    .component-title {
      margin-top: 8px;
      font-size: 0.84rem;
      color: var(--muted);
      text-align: center;
    }
    .simbox {
      margin-top: 12px;
      padding: 10px 12px;
      border-radius: 12px;
      background: rgba(45, 156, 219, 0.14);
      border: 1px solid rgba(45, 156, 219, 0.35);
      color: #bde3ff;
      font-size: 0.88rem;
    }
    svg { width: 100%; height: auto; margin-top: 10px; }
    .wire { stroke: var(--line); stroke-width: 3; fill: none; }
    .lbl { fill: #d7e3ff; font-size: 13px; }
    .chip { fill: var(--chip); }
    .transistor { fill: var(--transistor); }
    .resistor { fill: var(--resistor); }

    @media (max-width: 860px) {
      .shell { grid-template-columns: 1fr; }
      .grid { grid-template-columns: 1fr; }
      .components { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <main class='shell'>
    <section class='panel'>
      <h1>Painel do ESP32</h1>
      <p class='muted'>Monitoramento do estado do PC e acionamento via 2N2222.</p>
      <div class='simbox'>
        Modo de teste: o GPIO23 acende um LED com resistor de 1K e o estado do PC pode ser simulado sem abrir o computador.
      </div>
      <div class='status-banner' id='statusBanner'>Iniciando...
        <span class='status-sub' id='statusSub'>Aguardando telemetria do firmware.</span>
      </div>

      <div class='grid'>
        <div class='metric'><div class='k'>Rede atual</div><div class='v' id='wifiSsid'>-</div></div>
        <div class='metric'><div class='k'>IP do ESP32</div><div class='v' id='wifiIp'>-</div></div>
        <div class='metric'><div class='k'>Estado do PC</div><div class='v' id='pcState'>-</div></div>
        <div class='metric'><div class='k'>Auto power</div><div class='v' id='autoPower'>-</div></div>
      </div>

      <div id='pcBadge' class='badge warn'>Aguardando status...</div>
      <div>
        <button id='btnPower' type='button'>Acionar botao Power do PC</button>
        <button id='btnToggleSim' type='button' style='margin-left: 8px; background: linear-gradient(135deg, #2d9cdb, #27ae60);'>Alternar PC simulado</button>
      </div>
      <div class='log' id='log'></div>
    </section>

    <section class='panel'>
      <h1>Mapa da Ligacao</h1>
      <p class='muted'>Esquema visual simplificado do que esta acontecendo no hardware.</p>
      <div class='components'>
        <div class='component-card'>
          <svg viewBox='0 0 220 120' aria-label='Desenho do ESP32' role='img'>
            <rect x='22' y='16' width='176' height='88' rx='12' fill='#1f3b66' stroke='#7fb7ff' stroke-width='2'/>
            <rect x='42' y='30' width='45' height='60' rx='4' fill='#2d9cdb'/>
            <rect x='133' y='30' width='45' height='60' rx='4' fill='#2d9cdb'/>
            <text x='110' y='28' fill='#e9f0ff' font-size='12' text-anchor='middle'>ESP32</text>
            <text x='110' y='74' fill='#e9f0ff' font-size='10' text-anchor='middle'>GPIO23 / GPIO34 / GND</text>
          </svg>
          <div class='component-title'>ESP32 WROOM-32</div>
        </div>
        <div class='component-card'>
          <svg viewBox='0 0 220 120' aria-label='Desenho do resistor 1K' role='img'>
            <line x1='18' y1='60' x2='60' y2='60' stroke='#d7e3ff' stroke-width='4'/>
            <polyline points='60,60 74,44 88,76 102,44 116,76 130,44 144,76 158,60' fill='none' stroke='#f2994a' stroke-width='4' stroke-linejoin='round'/>
            <line x1='158' y1='60' x2='202' y2='60' stroke='#d7e3ff' stroke-width='4'/>
            <rect x='78' y='24' width='62' height='24' rx='8' fill='#f2994a' opacity='0.2'/>
            <text x='109' y='41' fill='#ffe8c4' font-size='12' text-anchor='middle'>1K 1/4W 5%</text>
          </svg>
          <div class='component-title'>Resistor 1K 1/4W 5%</div>
        </div>
        <div class='component-card'>
          <svg viewBox='0 0 220 120' aria-label='Desenho do transistor 2N2222' role='img'>
            <circle cx='110' cy='60' r='28' fill='#9b51e0' opacity='0.25' stroke='#d6b3ff' stroke-width='2'/>
            <line x1='72' y1='60' x2='48' y2='60' stroke='#d7e3ff' stroke-width='4'/>
            <line x1='110' y1='88' x2='110' y2='108' stroke='#d7e3ff' stroke-width='4'/>
            <line x1='148' y1='60' x2='172' y2='60' stroke='#d7e3ff' stroke-width='4'/>
            <text x='110' y='56' fill='#f2e8ff' font-size='12' text-anchor='middle'>2N2222</text>
            <text x='110' y='74' fill='#f2e8ff' font-size='10' text-anchor='middle'>B  C  E</text>
            <path d='M96 70 L124 50' stroke='#f2e8ff' stroke-width='2'/>
            <path d='M96 50 L124 70' stroke='#f2e8ff' stroke-width='2'/>
          </svg>
          <div class='component-title'>Transistor NPN 2N2222</div>
        </div>
        <div class='component-card'>
          <svg viewBox='0 0 220 120' aria-label='Desenho de jumper wires' role='img'>
            <path d='M20 84 C48 24, 88 24, 112 64 S176 104, 202 42' fill='none' stroke='#27ae60' stroke-width='5' stroke-linecap='round'/>
            <path d='M20 42 C52 90, 88 92, 112 54 S176 10, 202 70' fill='none' stroke='#2d9cdb' stroke-width='5' stroke-linecap='round'/>
            <circle cx='20' cy='84' r='5' fill='#d7e3ff'/>
            <circle cx='202' cy='42' r='5' fill='#d7e3ff'/>
            <circle cx='20' cy='42' r='5' fill='#d7e3ff'/>
            <circle cx='202' cy='70' r='5' fill='#d7e3ff'/>
          </svg>
          <div class='component-title'>Jumpers / fios de ligação</div>
        </div>
      </div>
      <svg viewBox='0 0 640 360' role='img' aria-label='Diagrama ESP32, resistor e transistor'>
        <rect x='30' y='80' width='190' height='180' rx='16' class='chip'/>
        <text x='52' y='110' class='lbl'>ESP32</text>
        <text x='52' y='138' class='lbl'>GPIO POWER_OUT</text>
        <text x='52' y='166' class='lbl'>GPIO PC_STATUS</text>
        <text x='52' y='194' class='lbl'>GND COMUM</text>

        <rect x='275' y='118' width='88' height='32' rx='8' class='resistor'/>
        <text x='292' y='139' class='lbl'>1K</text>

        <circle cx='430' cy='200' r='42' class='transistor'/>
        <text x='400' y='205' class='lbl'>2N2222</text>

        <rect x='508' y='90' width='100' height='180' rx='12' fill='#3d4f80'/>
        <text x='525' y='118' class='lbl'>PC</text>
        <text x='525' y='146' class='lbl'>PWR SW</text>
        <text x='525' y='174' class='lbl'>STATUS</text>

        <path d='M220 136 H274' class='wire'/>
        <path d='M363 134 H395 V175' class='wire'/>
        <path d='M470 200 H508' class='wire'/>
        <path d='M470 222 H508' class='wire'/>
        <path d='M180 194 H430 V242' class='wire'/>
        <path d='M220 166 H508' class='wire'/>
      </svg>
    </section>
  </main>

  <script>
    const wifiSsid = document.getElementById('wifiSsid');
    const wifiIp = document.getElementById('wifiIp');
    const pcState = document.getElementById('pcState');
    const autoPower = document.getElementById('autoPower');
    const pcBadge = document.getElementById('pcBadge');
    const log = document.getElementById('log');
    const statusBanner = document.getElementById('statusBanner');
    const statusSub = document.getElementById('statusSub');
    const btnPower = document.getElementById('btnPower');
    const btnToggleSim = document.getElementById('btnToggleSim');

    function setBadge(status) {
      pcBadge.classList.remove('ok', 'off', 'warn');
      if (status === 'ON') {
        pcBadge.classList.add('ok');
        pcBadge.textContent = 'PC ligado';
      } else {
        pcBadge.classList.add('off');
        pcBadge.textContent = 'PC desligado';
      }
    }

    async function updateStatus() {
      try {
        const r = await fetch('/status');
        const s = await r.json();
        wifiSsid.textContent = s.ssid || '-';
        wifiIp.textContent = s.ip || '-';
        pcState.textContent = s.pc_on ? 'Ligado' : 'Desligado';
        autoPower.textContent = s.auto_power_enabled ? 'Ativo' : 'Inativo';
        statusBanner.childNodes[0].nodeValue = s.status_operacional || 'Sem status';
        statusSub.textContent = s.modo_ap ? 'Hotspot ativo e em busca do Wi-Fi salvo.' : 'Conectado na rede. A Home acompanha o estado do PC.';
        if (s.simulation_mode) {
          log.textContent = s.pc_on ? 'Simulacao ativa: PC virtual ligado.' : 'Simulacao ativa: PC virtual desligado.';
        }
        setBadge(s.pc_on ? 'ON' : 'OFF');
      } catch (e) {
        log.textContent = 'Falha ao atualizar status.';
      }
    }

    btnPower.addEventListener('click', async () => {
      btnPower.disabled = true;
      log.textContent = 'Acionando pulso de power...';
      try {
        await fetch('/power/toggle', { method: 'POST' });
        log.textContent = 'Pulso enviado com sucesso.';
        updateStatus();
      } catch (e) {
        log.textContent = 'Falha ao enviar pulso.';
      } finally {
        setTimeout(() => { btnPower.disabled = false; }, 900);
      }
    });

    btnToggleSim.addEventListener('click', async () => {
      btnToggleSim.disabled = true;
      try {
        const current = pcState.textContent === 'Ligado' ? 'off' : 'on';
        await fetch('/pc/simulate', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'state=' + current
        });
        log.textContent = 'Estado simulado atualizado.';
        updateStatus();
      } catch (e) {
        log.textContent = 'Falha ao alternar simulacao.';
      } finally {
        setTimeout(() => { btnToggleSim.disabled = false; }, 700);
      }
    });

    updateStatus();
    setInterval(updateStatus, 1000);
  </script>
</body>
</html>
)";

const char HTML_SUCCESS_PAGE[] = R"(
<!DOCTYPE html>
<html lang='pt-BR'>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>Wi-Fi salvo</title>
  <style>
    body {
      margin: 0;
      min-height: 100vh;
      display: grid;
      place-items: center;
      font-family: 'Segoe UI', Tahoma, sans-serif;
      background: linear-gradient(150deg, #0f4c75, #3282b8);
      color: #fff;
      text-align: center;
      padding: 18px;
    }
    .box {
      width: min(100%, 420px);
      background: rgba(0,0,0,0.25);
      border-radius: 14px;
      padding: 28px;
      border: 1px solid rgba(255,255,255,0.2);
    }
    h1 { margin: 0 0 8px; }
    p { margin: 0; opacity: .9; }
  </style>
</head>
<body>
  <div class='box'>
    <h1>Wi-Fi salvo</h1>
    <p>Reiniciando o ESP32 para aplicar a configuracao...</p>
  </div>
</body>
</html>
)";

#endif
