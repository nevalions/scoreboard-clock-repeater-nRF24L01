# Repeater Module

ESP8266-based repeater module for extending the scoreboard timer network range.

## Overview

The repeater module acts as a network extender that receives time data packets from the controller and retransmits them to extend the wireless range. It operates transparently - the existing controller and play clock modules work unchanged whether the repeater is present or not.

## Hardware

- **MCU**: ESP8266 (NodeMCU v2 recommended)
- **Radio**: nRF24L01+ module
- **Status LED**: Built-in LED (GPIO16)
- **Power**: 5V USB or external power supply

## Wiring

### ESP8266 to nRF24L01+ Connections

| ESP8266 Pin | nRF24L01+ Pin | Function |
|-------------|---------------|----------|
| D4 (GPIO2)  | CE            | Chip Enable |
| D8 (GPIO15) | CSN           | Chip Select |
| D7 (GPIO13) | MOSI          | Master Out |
| D6 (GPIO12) | MISO          | Master In |
| D5 (GPIO14) | SCK           | Clock |
| 3V3         | VCC           | Power (3.3V) |
| GND         | GND           | Ground |

### Status LED
- Built-in LED on GPIO16 shows network activity:
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
2. Upload firmware using PlatformIO:
   ```bash
   cd repeater
   pio run --target upload
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