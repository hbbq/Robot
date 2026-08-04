#ifdef DEVICE_ROBOT

#include "CalibrationWebServer.h"

#include <Arduino.h>
#include <WiFi.h>

#include <BehaviorController.h>
#include <CalibrationBehavior.h>
#include <CalibrationRequestStore.h>
#include <DriveController.h>
#include <MotionControllerConfig.h>
#include <ReadinessController.h>

#include <cmath>
#include <cstdlib>

namespace
{
    const char CalibrationPage[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Robot motion calibration</title><style>
body{font-family:system-ui,sans-serif;max-width:720px;margin:auto;padding:18px;background:#101719;color:#e3eceb}h1{font-size:1.5rem}section{background:#182326;padding:14px;margin:12px 0;border-radius:10px}button,input{font:inherit;padding:9px;margin:4px}button{background:#315b60;color:#fff;border:0;border-radius:6px}button:disabled{opacity:.35}.stop{background:#a63232;font-weight:700;font-size:1.15rem;width:100%}.grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}.status{display:grid;grid-template-columns:repeat(2,1fr);gap:6px}.value{color:#8ed0c8}label{display:block;margin-top:7px}input{width:9rem;background:#eef5f4;border:0;border-radius:4px}.error{color:#ff9d8d}table{width:100%;border-collapse:collapse}td{padding:5px;border-bottom:1px solid #304044}
</style></head><body><h1>Robot motion calibration</h1>
<section class="status"><div>Ready: <b id="ready">?</b></div><div>Mode: <b id="mode">?</b></div><div>Motion: <b id="motion">?</b></div><div>Busy: <b id="busy">?</b></div></section>
<section><div class="grid"><button onclick="session('enter')">Enter Calibration</button><button onclick="session('exit')">Exit Calibration</button></div><button class="stop" onclick="stopNow()">STOP</button><p id="message"></p></section>
<section><h2>Movement</h2><label>Distance (m) <input id="distance" type="number" min="0.01" step="0.01" value="1.00"></label><div class="grid"><button class="move" onclick="move('forward','distance')">Forward</button><button class="move" onclick="move('backward','distance')">Backward</button></div><label>Angle (degrees) <input id="angle" type="number" min="1" step="1" value="90"></label><div class="grid"><button class="move" onclick="move('turnLeft','angle')">Turn Left</button><button class="move" onclick="move('turnRight','angle')">Turn Right</button></div></section>
<section><h2>Current configuration</h2><table><tr><td>moveSpeed</td><td id="moveSpeed" class="value"></td></tr><tr><td>turnSpeed</td><td id="turnSpeed" class="value"></td></tr><tr><td>millisecondsPerMeter</td><td id="msMeter" class="value"></td></tr><tr><td>millisecondsPerDegree</td><td id="msDegree" class="value"></td></tr></table></section>
<section><h2>Suggested calibration</h2><label>Actual distance (m) <input id="actualDistance" type="number" min="0.01" step="0.01"></label><p>Suggested millisecondsPerMeter: <b id="suggestMeter">-</b></p><label>Actual angle (degrees) <input id="actualAngle" type="number" min="0.1" step="0.1"></label><p>Suggested millisecondsPerDegree: <b id="suggestDegree">-</b></p></section>
<script>
let state={};const msg=document.getElementById('message');
async function post(url,data={}){try{const body=new URLSearchParams(data);const r=await fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});const t=await r.text();msg.textContent=t;msg.className=r.ok?'':'error';await status()}catch(e){msg.textContent='Connection lost';msg.className='error'}}
function session(action){post('/api/calibration/session',{action})}function stopNow(){post('/api/calibration/stop')}
function move(action,id){post('/api/calibration/motion',{action,value:document.getElementById(id).value})}
async function status(){try{const r=await fetch('/api/calibration',{cache:'no-store'});state=await r.json();for(const k of ['ready','mode','motion','busy'])document.getElementById(k).textContent=state[k];document.getElementById('moveSpeed').textContent=state.moveSpeed;document.getElementById('turnSpeed').textContent=state.turnSpeed;document.getElementById('msMeter').textContent=state.millisecondsPerMeter;document.getElementById('msDegree').textContent=state.millisecondsPerDegree;document.querySelectorAll('.move').forEach(b=>b.disabled=!state.ready||state.mode!=='calibration'||state.busy);calculate()}catch(e){document.getElementById('ready').textContent='offline'}}
function suggested(current,commanded,actual){return current>0&&commanded>0&&actual>0?Math.round(current*commanded/actual):'-'}
function calculate(){document.getElementById('suggestMeter').textContent=suggested(state.millisecondsPerMeter,+document.getElementById('distance').value,+document.getElementById('actualDistance').value);document.getElementById('suggestDegree').textContent=suggested(state.millisecondsPerDegree,+document.getElementById('angle').value,+document.getElementById('actualAngle').value)}
document.querySelectorAll('input').forEach(i=>i.addEventListener('input',calculate));setInterval(()=>{status();if(state.mode==='calibration')post('/api/calibration/session',{action:'keepalive'})},1000);status();
</script></body></html>)HTML";

    const char* modeText(RobotMode mode)
    {
        switch (mode)
        {
            case RobotMode::Idle: return "idle";
            case RobotMode::Autonomous: return "autonomous";
            case RobotMode::RemoteControl: return "remote";
            case RobotMode::Calibration: return "calibration";
        }
        return "unknown";
    }

    const char* motionText(RobotMotion motion)
    {
        switch (motion)
        {
            case RobotMotion::Stopped: return "stopped";
            case RobotMotion::Forward: return "forward";
            case RobotMotion::Backward: return "backward";
            case RobotMotion::TurningLeft: return "turning-left";
            case RobotMotion::TurningRight: return "turning-right";
            case RobotMotion::Curve: return "curve";
        }
        return "unknown";
    }
}

CalibrationWebServer::CalibrationWebServer(
    CalibrationRequestStore& requestStore,
    CalibrationBehavior& calibrationBehavior,
    ReadinessController& readiness,
    BehaviorController& behaviorController,
    DriveController& driveController,
    const MotionControllerConfig& motionConfig,
    const CalibrationBehaviorConfig& behaviorConfig,
    const CalibrationWebServerConfig& serverConfig)
    : _requestStore(requestStore),
      _calibrationBehavior(calibrationBehavior),
      _readiness(readiness),
      _behaviorController(behaviorController),
      _driveController(driveController),
      _motionConfig(motionConfig),
      _behaviorConfig(behaviorConfig),
      _serverConfig(serverConfig),
      _server(serverConfig.port)
{
}

void CalibrationWebServer::begin()
{
    configureRoutes();
}

void CalibrationWebServer::update(bool wifiConnected)
{
    if (!wifiConnected)
    {
        if (_running)
        {
            _server.stop();
            _running = false;
            Serial.println("[CalibrationWeb] Stopped");
        }
        return;
    }

    if (!_running)
    {
        _server.begin();
        _running = true;
        Serial.printf(
            "[CalibrationWeb] http://%s:%u/\n",
            WiFi.localIP().toString().c_str(),
            _serverConfig.port);
    }

    _server.handleClient();
}

void CalibrationWebServer::configureRoutes()
{
    if (_routesConfigured)
    {
        return;
    }

    _server.on("/", HTTP_GET, [this]()
    {
        _server.sendHeader("Cache-Control", "no-store");
        _server.send_P(200, "text/html", CalibrationPage);
    });
    _server.on("/api/calibration", HTTP_GET, [this]()
    {
        handleStatus();
    });
    _server.on("/api/calibration/session", HTTP_POST, [this]()
    {
        handleSession();
    });
    _server.on("/api/calibration/motion", HTTP_POST, [this]()
    {
        handleMotion();
    });
    _server.on("/api/calibration/stop", HTTP_POST, [this]()
    {
        handleStop();
    });
    _server.onNotFound([this]()
    {
        _server.send(404, "text/plain", "Not found");
    });

    _routesConfigured = true;
}

void CalibrationWebServer::handleStatus()
{
    const RobotMode mode = _behaviorController.currentMode();
    String response;
    response.reserve(280);
    response += "{\"ready\":";
    response += _readiness.isReady() ? "true" : "false";
    response += ",\"mode\":\"";
    response += modeText(mode);
    response += "\",\"motion\":\"";
    response += motionText(_driveController.getMotion());
    response += "\",\"busy\":";
    response += (mode == RobotMode::Calibration &&
                 _calibrationBehavior.isBusy()) ? "true" : "false";
    response += ",\"moveSpeed\":";
    response += String(_motionConfig.moveSpeed, 3);
    response += ",\"turnSpeed\":";
    response += String(_motionConfig.turnSpeed, 3);
    response += ",\"millisecondsPerMeter\":";
    response += String(_motionConfig.millisecondsPerMeter, 1);
    response += ",\"millisecondsPerDegree\":";
    response += String(_motionConfig.millisecondsPerDegree, 2);
    response += "}";

    _server.sendHeader("Cache-Control", "no-store");
    _server.send(200, "application/json", response);
}

void CalibrationWebServer::handleSession()
{
    const String action = _server.arg("action");

    if (action == "enter")
    {
        if (!_readiness.isReady())
        {
            _server.send(503, "text/plain", "Robot is not ready");
            return;
        }
        _requestStore.requestEnter();
    }
    else if (action == "exit")
    {
        _requestStore.requestExit();
    }
    else if (action == "keepalive")
    {
        if (_behaviorController.currentMode() != RobotMode::Calibration)
        {
            _server.send(409, "text/plain", "Calibration is not active");
            return;
        }
        _requestStore.requestKeepAlive();
    }
    else
    {
        _server.send(400, "text/plain", "Invalid session action");
        return;
    }

    _server.send(202, "text/plain", "Accepted");
}

void CalibrationWebServer::handleMotion()
{
    if (!_readiness.isReady())
    {
        _server.send(503, "text/plain", "Robot is not ready");
        return;
    }

    if (_behaviorController.currentMode() != RobotMode::Calibration)
    {
        _server.send(409, "text/plain", "Calibration is not active");
        return;
    }

    if (_calibrationBehavior.isBusy())
    {
        _server.send(409, "text/plain", "Calibration movement is busy");
        return;
    }

    float value = 0.0f;
    if (!parsePositiveValue(value))
    {
        _server.send(400, "text/plain", "Invalid positive value");
        return;
    }

    const String action = _server.arg("action");
    CalibrationMotionCommand command;
    float maximum = 0.0f;

    if (action == "forward")
    {
        command = CalibrationMotionCommand::Forward;
        maximum = _behaviorConfig.maximumDistanceMeters;
    }
    else if (action == "backward")
    {
        command = CalibrationMotionCommand::Backward;
        maximum = _behaviorConfig.maximumDistanceMeters;
    }
    else if (action == "turnLeft")
    {
        command = CalibrationMotionCommand::TurnLeft;
        maximum = _behaviorConfig.maximumTurnDegrees;
    }
    else if (action == "turnRight")
    {
        command = CalibrationMotionCommand::TurnRight;
        maximum = _behaviorConfig.maximumTurnDegrees;
    }
    else
    {
        _server.send(400, "text/plain", "Invalid motion action");
        return;
    }

    if (value > maximum)
    {
        _server.send(400, "text/plain", "Value exceeds safe calibration limit");
        return;
    }

    if (!_requestStore.requestMotion(command, value))
    {
        _server.send(409, "text/plain", "Another request is pending");
        return;
    }

    _server.send(202, "text/plain", "Accepted");
}

void CalibrationWebServer::handleStop()
{
    _requestStore.requestStop();
    _server.send(202, "text/plain", "STOP accepted");
}

bool CalibrationWebServer::parsePositiveValue(float& value)
{
    if (!_server.hasArg("value"))
    {
        return false;
    }

    const String text = _server.arg("value");
    char* end = nullptr;
    value = std::strtof(text.c_str(), &end);

    return end != text.c_str() &&
        *end == '\0' &&
        std::isfinite(value) &&
        value > 0.0f;
}

#endif
