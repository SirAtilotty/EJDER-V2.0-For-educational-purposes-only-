# EJDER-V2.0-For-educational-purposes-only-

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg) ![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-blue?logo=c%2B%2B&logoColor=white) ![IDE: Arduino](https://img.shields.io/badge/IDE-Arduino-00979D?logo=arduino&logoColor=white) ![Platform: ESP32-S2](https://img.shields.io/badge/Platform-ESP32--S2-df2a2a?logo=espressif&logoColor=white)

EJDER - ESP32-S2 WiFi Pentest Tool 🐉
EJDER is a portable WiFi security auditing tool developed specifically for the ESP32-S2 . It is designed to demonstrate 802.11 layer vulnerabilities and test network resilience through various frame injection techniques.

🚀 Features
Deauthentication Attack: Send spoofed deauth frames to disconnect clients from a target Access Point.

Beacon Flood: Populate the WiFi list of nearby devices with multiple fake SSIDs.

Probe Request Flood: Generate mass probe requests to test network monitoring tools and hide device signatures.

Web-Based Dashboard: Full control via a mobile-friendly web interface (192.168.4.1).

Standalone Operation: Power it with a 3.7V battery or power bank and deploy it anywhere.

🛠️ Hardware Requirements
ESP32-S2 or any ESP32-S2 based board.

Power Source: USB Power Bank or Li-Po Battery.

Tested On: ESP32-S2 Single-Core 240MHz.

⚙️ Installation
Open the Arduino IDE.

Select ESP32S2 Dev Module from the Boards menu.

Ensure USB CDC On Boot is set to Enabled in the Tools menu.

Flash the EJDERV2.0_eng.ino file to your board.

📖 How to Use
Connect to the WiFi network: "EJDER".

Password: "EJDERYA32".

Open your browser and go to 192.168.4.1.

Scan for nearby networks, select your target, and choose an attack mode.

⚠️ Technical Limitations
Single-Core Architecture: Since the ESP32-S2 is a single-core processor, high-intensity attacks may cause temporary web interface lag.

PMF Defense: Modern devices with 802.11w (Protected Management Frames) enabled will ignore Deauthentication frames. This is a protocol limitation, not a software bug.

⚖️ Disclaimer
This software is for educational and ethical testing purposes only. Unauthorized use against networks without explicit permission is illegal. The developer is not responsible for any damage or legal issues caused by this tool.

🤝 Contributing
Feel free to fork this project and submit pull requests.

Upcoming:

[ ] ESP32-S3 Dual-Core support (Coming Soon).

[ ] Captive Portal (Evil Twin) features.
