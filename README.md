# SmartMeter ESP32

A smart meter reader for the ESP32 that collects meter data and provides a web interface for configuration and monitoring. The device reads SML (Smart Meter Language) data and can push readings to a remote API.

## Overview

This project implements a firmware for the **ESP32-C3 Super Mini** microcontroller that:

- **Reads Smart Meter Data**: Captures SML-formatted meter readings via serial connection
- **Web Interface**: Provides a responsive HTML-based dashboard for real-time monitoring
- **WiFi Configuration**: Easy WiFi setup through a captive portal (Access Point mode)
- **Push API**: Automatically uploads meter readings to a remote API endpoint
- **System Monitoring**: Includes watchdog timer, heartbeat monitoring, and system health checks

<img width="1583" height="1682" alt="grafik" src="https://github.com/user-attachments/assets/e6501357-64cb-4d47-9dd2-917704bcca36" />

## Getting Started

### Hardware Requirements

- **Board**: ESP32-C3 Super Mini (or compatible ESP32-C3 board)
- **Smart Meter**: Smart meter with SML protocol
- **Optical Reader**: Optical reading head to interface with the smart meter
- **Power Supply**: 5V USB or battery

### Development Tools

This project is built with:
- **[Visual Studio Code](https://code.visualstudio.com/)** - Code editor
- **[PlatformIO](https://platformio.org/)** - Embedded systems build system and IDE
- **[Arduino Framework](https://www.arduino.cc/)** - Hardware abstraction layer

### Installation

1. **Install VS Code and PlatformIO**:
   - Download [Visual Studio Code](https://code.visualstudio.com/)
   - Install the PlatformIO extension from the marketplace

2. **Clone/Open this repository** in VS Code

3. **Connect your ESP32-C3**:
   - Plug the board into your computer via USB-C

4. **Build and Upload**:
   ```bash
   # Build the firmware
   platformio run

   # Upload to device
   platformio run --target upload
   ```

## Configuration

### Initial Setup - Access Point Mode

1. **Power on the device** - it will start in Access Point mode if not configured
2. **Connect to WiFi**: Look for SSID `ESP32-SmartMeter`
3. **Open Web Interface**: Navigate to `http://192.168.10.1` in your browser
4. **Enter WiFi Credentials**:
   - Select your WiFi network from the list
   - Enter your WiFi password
   - Click "Save"
5. **Device Connects**: The ESP32 will connect to your network

Once connected, you can access the interface at the device's local IP address.

### Push API Configuration

To enable automatic data uploads:

1. **Access Settings Page**: Connect to the web interface and go to Settings
2. **API Configuration**:
   - **API Endpoint**: Enter the full URL (e.g., `https://api.example.com/push`)
   - **API Key**: Optional authentication token if required by your API
3. **Save Settings**: Click "Save" to apply
4. **Data Upload**: The device will now automatically push meter readings at configured intervals

The device sends JSON-formatted meter data to the specified endpoint.

## Web Interface

The web interface provides:

- **Overview Tab**: Real-time meter readings and system status (`/overview`)
- **Settings Tab**: WiFi, API, and system configuration (`/settings`)
- **Status Indicators**: Connection status, uptime, and signal strength

## Factory reset:

To reset all stored settings, press and hold the **BOOT** button for at least 3 seconds.
The ESP32 will automatically reboot and return to Access Point mode for reconfiguration.

## Dependencies

The project uses the following libraries:

| Library | Version | Purpose |
|---------|---------|---------|
| **WiFiManager** | 2.0.17+ | WiFi configuration and captive portal |
| **SML Parser** | 0.29+ | Smart Meter Language protocol parsing |
| **ArduinoJson** | 7.4.3+ | JSON serialization/deserialization |

### Build Dependencies

- **ESP32 Arduino Framework** - Core Arduino functionality for ESP32
- **PlatformIO** - Build system and dependency management

## System Features

- **Watchdog Timer**: Automatic recovery from hangs (5-second timeout)
- **Heartbeat LED**: Visual feedback (pin 6)
- **Reset Button**: Long press for factory reset (pin BOOT)
- **System Health**: Continuous monitoring of uptime and resource usage
- **Data Collection**: Configurable meter reading intervals (default 1 minute)

## Disclaimer

⚠️ **Note on HTML Files**

The HTML interface files (`overview.html`, `settings.html`) were created with the assistance of **GitHub Copilot**. While they have been tested and verified to work correctly, they are generated code and should be reviewed for your specific security and styling requirements if necessary.

## Project Structure

```
src/
├── main.cpp                 # Main firmware entry point
├── datacollector.cpp        # Meter data collection and storage
├── smlreader.cpp            # SML protocol parsing
├── iec62056.cpp             # Unused (IEC62056 protocol for older meters)
├── networking.cpp           # WiFi connectivity and WiFiManager
├── reset.cpp                # Reset and factory reset handling
├── system.cpp               # System health and uptime tracking
├── components/              # Core system components
│   ├── watchdog.cpp         # Watchdog timer
│   ├── heartbeat.cpp        # Status LED heartbeat
│   ├── timer.cpp            # General timers
│   └── datetime.cpp         # Unused (simple DateTime implementation)
├── config/                  # Configuration management
│   ├── config_network.cpp   # Network settings
│   ├── config_pushapi.cpp   # Push API configuration
│   └── config_system.cpp    # System settings
└── web/                     # Web interface
    ├── httpserver.cpp       # Web server
    ├── httpclient.cpp       # API client
    └── server/html/         # HTML interface files
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Support

For issues, questions, or contributions, please open an issue on the repository.
