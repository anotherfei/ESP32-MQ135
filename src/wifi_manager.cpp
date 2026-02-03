#include <Arduino.h>
#include <wifi_manager.h>

const int SENSOR_POWER = 27;

wifi_manager::wifi_manager(uint16_t port) : server(port) {}
bool wifi_manager::begin() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    if (!WiFi.softAP(ssid, password)) return false;

    IP = WiFi.softAPIP();
    Serial.print("IP address: ");
    Serial.println(IP);

    server.begin();
    return true;
}

void wifi_manager::manageClient(const int Sensor_pin) {
    WiFiClient client = server.available();
    if (!client) return;

    Serial.println("Client Connected");

    String requestLine = "";
    while (client.connected()) {
        if (!client.available()) {
            delay(5);
            continue;
        }
        requestLine = client.readStringUntil('\r');
        client.read(); // consume \n
        Serial.println(requestLine);
        break;
    }

    // ================= ROUTING =================
    if (requestLine.startsWith("GET /aqi")) {
        // ---- JSON API ----
        int score = map(analogRead(Sensor_pin), 0, 4095, 0, 500); // AQI-like score mapping

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        client.print("{\"score\":");
        client.print(score);
        client.println("}");
    }
    else if (requestLine.startsWith("GET /")) {
        // ---- MAIN HTML PAGE ----
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println();

        client.println(F(R"rawliteral(
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
                <form id="wifiForm">
                    <label for="ssid">Network Name (SSID)</label>
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


                const user_id = "esp32"
                const user_password = "000000"

                document.getElementById('wifiForm').addEventListener('submit', function(e) {
                    e.preventDefault();
                    const ssid = document.getElementById('ssid').value;
                    const pass = document.getElementById('password').value;

                    if (ssid === user_id && pass === user_password) {
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
                    document.getElementById('breachHistory').style.display = 'block';
                }


                setInterval(load, 2000);
                load();
            </script>

        </body>
        </html>
        )rawliteral"));
    }
    else {
        // ---- 404 error ----
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("Not Found");
    }

    client.stop();
    Serial.println("Client Disconnected");
}

wifi_manager::~wifi_manager() {}