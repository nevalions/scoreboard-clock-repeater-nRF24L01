# Repeater Module

ESP32-based repeater module for extending the scoreboard timer network range.

## Overview

The repeater module acts as a network extender that receives time data packets from the controller and retransmits them to extend the wireless range. It operates transparently - the existing controller and play clock modules work unchanged whether the repeater is present or not.

## Hardware

- **MCU**: ESP32 (ESP32-DevKitC recommended)
- **Radio**: nRF24L01+ module
- **Status LED**: Built-in LED (GPIO16)
- **Power**: 5V USB or external power supply

## Wiring

### ESP32 to nRF24L01+ Connections

| ESP32 Pin | nRF24L01+ Pin | Function |
|-----------|---------------|----------|
| GPIO5     | CE            | Chip Enable |
| GPIO4     | CSN           | Chip Select |
| GPIO23    | MOSI          | Master Out |
| GPIO19    | MISO          | Master In |
| GPIO18    | SCK           | Clock |
| 3V3       | VCC           | Power (3.3V) |
| GND       | GND           | Ground |

### Status LED
- LED on GPIO2 shows network activity:
  - Fast blink (200ms): Active packet forwarding
  - Slow blink (1000ms): Idle/no activity

## Network Configuration

The repeater uses the same network settings as the main system:
- **Channel**: 76 (2.476 GHz)
- **Data Rate**: 1 Mbps
- **Address**: 0xE7E7E7E7E7
- **Power**: 0 dBm
- **CRC**: 16-bit enabled
- **Auto-ACK**: Disabled

## Operation

1. **Initialization**: Configures nRF24L01+ with network settings
2. **Listening**: Continuously listens for time data packets
3. **Forwarding**: Immediately retransmits received packets
4. **Statistics**: Tracks packet counts and link status

## Packet Format

The repeater forwards the same 3-byte time data format used by the controller:
```
Byte 0: seconds_high (MSB)
Byte 1: seconds_low (LSB)  
Byte 2: sequence number
```

## Installation

1. Connect hardware as shown in wiring diagram
2. Upload firmware using ESP-IDF:
    ```bash
    cd repeater
    idf.py flash monitor
    ```
3. Monitor serial output at 115200 baud for debugging

## Placement Tips

- Place repeater centrally between controller and play clock
- Elevate for better line-of-sight (2.5-4m recommended)
- Avoid metal obstacles and interference sources
- Ensure stable power supply

## Statistics Output

Every 30 seconds, the repeater outputs statistics:
- Packets received count
- Packets retransmitted count  
- Link status
- Last received data

## Integration

The repeater is completely transparent to the system:
- **Controller**: Unchanged, continues broadcasting every 250ms
- **Play Clock**: Unchanged, receives time data from whichever source is strongest
- **System**: Works with or without repeater present

Multiple repeaters can be deployed for larger areas - they will automatically forward packets creating a mesh-like extension.