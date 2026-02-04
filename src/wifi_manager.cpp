#include "wifi_manager.h"

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

static const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>AirSense: Trustable Smoke Detector</title>
    <link href="https://fonts.googleapis.com/css2?family=Dancing+Script&display=swap" rel="stylesheet">
    <style>
        body {
            font-family: 'Dancing Script', cursive;
            background: #121212;
            color: #e0e0e0;
            margin: 0;
            padding: 0;
        }

        .container {
            max-width: 800px;
            margin: 60px auto;
            padding: 30px;
            background: #1e1e1e;
            border-radius: 10px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.5);
        }

        h1 {
            text-align: center;
            color: #4caf50;
            margin-bottom: 10px;
        }

        h2 {
            text-align: center;
            font-weight: normal;
            color: #aaa;
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-top: 15px;
            color: #ccc;
        }

        input {
            width: 100%;
            padding: 10px;
            margin-top: 5px;
            border: 1px solid #333;
            border-radius: 6px;
            background: #2a2a2a;
            color: #e0e0e0;
        }

        button {
            margin-top: 25px;
            width: 100%;
            padding: 12px;
            background: #4caf50;
            color: #fff;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            font-size: 16px;
        }

        button:hover {
            background: #43a047;
        }

        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 20px;
        }

        header .status {
            font-size: 14px;
            color: #aaa;
            text-align: right;
        }

        .section {
            margin-top: 20px;
            padding: 20px;
            background: #2a2a2a;
            border-radius: 8px;
        }

        .section h3 {
            color: #4caf50;
            margin-bottom: 10px;
        }

        .aqi {
            font-size: 24px;
            font-weight: bold;
            color: #4caf50;
        }

        .breach {
            font-size: 20px;
            font-weight: bold;
            color: #ff5252;
        }

        #dashboard {
            display: none;
        }
    </style>
</head>
<body>

    <div class="container" id="signin">
        <h1>AirSense</h1>
        <h2>Connect your device to start monitoring</h2>
        <form id="userID">
            <label for="ssid">Username</label>
            <input type="text" id="ssid" name="ssid" placeholder="Enter WiFi name" required>

            <label for="password">Password</label>
            <input type="password" id="password" name="password" placeholder="Enter WiFi password" required>

    <div id="errorMessage" style="display:none; color:#ff5252; margin-top:10px;"></div>


            <button type="submit">Connect Device →</button>
        </form>
    </div>

    <div class="container" id="dashboard">
        <header>
            <h1>AirSense Dashboard</h1>
            <div class="status">
                Connected to: <span id="connectedSSID"></span><br>
                Last update: <span id="time">--:--</span>
            </div>
        </header>

        <div class="section">
            <h3>Real-time Air Quality Index</h3>
            <div id="score" class="aqi">--</div>
            <p id="statusText">Waiting for sensor…</p>
        </div>

        <div class="section">
            <h3>Smoke Breach Count</h3>
            <div id="breach" class="breach">0</div>
            <p id="breachText">No smoke breaches detected so far.</p>
            <button onclick="showBreachHistory()">View Breach History</button>
        </div>
    </div>

    <div class="section" id="breachHistory" style="display:none;">
        <h3>Breach History</h3>
        <ul id="breachList" style="color:#ccc;"></ul>
        <button onclick="document.getElementById('breachHistory').style.display='none'">Close</button>
    </div>

    <script>
        let breachCount = 0;
        let Updated = false;
        let breachHistory = [];

        document.getElementById('userID').addEventListener('submit', async function(e) {
            e.preventDefault();
            const ssid = document.getElementById('ssid').value;
            const pass = document.getElementById('password').value;

            try {
                const response = await fetch('/login', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ ssid, pass })
                });

                const result = await response.json();
                if (result.success) {
                    document.getElementById('connectedSSID').textContent = ssid;
                    document.getElementById('signin').style.display = 'none';
                    document.getElementById('dashboard').style.display = 'block';
                } else {
                const errorMsg = document.getElementById('errorMessage');
                    errorMsg.textContent = "Wrong SSID or password — please try again.";
                    errorMsg.style.display = 'block';

                    document.getElementById('ssid').value = "";
                    document.getElementById('password').value = "";

                    document.getElementById('ssid').focus();
                }
            } catch (err) {
                console.error("Login request failed:", err);
            }
        });


        async function load() {
            try {
                const r = await fetch('/aqi');
                const d = await r.json();
                const s = d.score;

                const el = document.getElementById('score');
                const st = document.getElementById('statusText');
                el.textContent = s;

                if (s < 30) {
                    el.style.color = '#10b981';
                    st.textContent = 'Air quality is excellent — enjoy fresh air and keep smoke-free habits.';
                    Updated = false;
                } else if (s < 60) {
                    el.style.color = '#facc15';
                    st.textContent = 'Air quality is acceptable — avoid unnecessary smoking to prevent worsening conditions.';
                    Updated = false;
                } else {
                    el.style.color = '#ef4444';
                    st.textContent = 'Air quality is unhealthy — reduce smoking activities to help lower smoke levels and protect health.';
                    if (!Updated) {
                        breachCount++;
                        const now = new Date();
                        const timestamp = now.toLocaleDateString() + " " + now.toLocaleTimeString();
                        breachHistory.push(timestamp);
                        Updated = true;
                    }
                }

                document.getElementById('breach').textContent = breachCount;
                document.getElementById('breachText').textContent =
                    breachCount > 0 ? 'Smoke breaches detected!' : 'No smoke breaches detected so far.';

                document.getElementById('time').textContent = new Date().toLocaleTimeString();
            } catch (e) {
                document.getElementById('statusText').textContent = 'ESP32 offline';
            }
        }

        function showBreachHistory() {
            const list = document.getElementById('breachList');
            list.innerHTML = "";
            breachHistory.forEach(entry => {
                const li = document.createElement('li');
                li.textContent = entry;
                list.appendChild(li);
            });
            
            const historySection = document.getElementById('breachHistory');

            document.getElementById('breachHistory').style.display = 'block';
            historySection.scrollIntoView({ behavior: 'smooth', block: 'start' });

            function autoCloseOnScroll() {
                const rect = historySection.getBoundingClientRect();

                if (window.scrollY === 0 || rect.top > window.innerHeight || rect.bottom < 0) {
                    historySection.style.display = 'none';
                    window.removeEventListener('scroll', autoCloseOnScroll);
                }
            }

            window.addEventListener('scroll', autoCloseOnScroll);
        }

        setInterval(load, 1000);
        load();
    </script>

</body>
</html>
)rawliteral";

DeviceServer::DeviceServer(uint16_t port) : server(port) {}
bool DeviceServer::begin(const int Sensor_pin) {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ssid, password)) return false;
    
    IP = WiFi.softAPIP();
    Serial.print("IP address: ");
    Serial.println(IP);

    server.on("/status", HTTP_GET, [this]() {
        JsonDocument doc;
        doc["ip"] = WiFi.softAPIP().toString();
        doc["rssi"] = WiFi.RSSI();

        char buffer[64];
        serializeJson(doc, buffer);
        server.send(200, "application/json", buffer);
    });

    server.on("/login", HTTP_POST, [this]() {
        if (server.hasArg("plain")) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, server.arg("plain"));

            if (err) {
                server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }

            const char* inputSSID = doc["ssid"];
            const char* inputPass = doc["pass"];

            bool authenticated = false;
            for (size_t i = 0; i < allowedUserCount; i++) {
                if (inputSSID && inputPass &&
                    strcmp(inputSSID, allowedUsers[i].user) == 0 &&
                    strcmp(inputPass, allowedUsers[i].pass) == 0) {
                    authenticated = true;
                    break;
                }
            }

            server.send(
                200,
                "application/json",
                authenticated ? "{\"success\":true}" : "{\"success\":false}"
            );
        } 
        else {
            server.send(400, "application/json", "{\"error\":\"Bad Request\"}");
        }
    });


    server.on("/aqi", HTTP_GET, [this, Sensor_pin]() {
        int score = map(analogRead(Sensor_pin), 0, 4095, 0, 500);

        JsonDocument doc;
        doc["score"] = score;
        
        char buffer[64];
        serializeJson(doc, buffer);
        server.send(200, "application/json", buffer);
    });

    server.on("/", HTTP_GET, [this]() {
        server.send_P(200, "text/html", index_html);
    });

    server.begin();
    return true;
}

void DeviceServer::handle() {
    server.handleClient();
}

DeviceServer::~DeviceServer() {}