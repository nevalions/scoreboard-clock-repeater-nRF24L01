# ESP32 Board Repeater Module - Agent Guidelines

## Build Commands
- `idf.py build` - Build the project for ESP32 board
- `idf.py flash monitor` - Build, flash to ESP32 board and open serial monitor
- `idf.py monitor` - Open serial monitor at 115200 baud
- `idf.py clean` - Clean build artifacts
- `idf.py menuconfig` - Configure project settings

**Note**: This is an ESP-IDF project for ESP32 development board, not PlatformIO. Requires ESP-IDF environment setup.

## Radio Common Library
- **radio-common**: Located in parent folder `../radio-common/`
- **Shared**: Uses shared radio-common library across all modules

## Code Style Guidelines
- **Language**: C with ESP-IDF framework for ESP32 board
- **Formatting**: 4-space indentation, no tabs
- **Naming**: 
  - Functions: snake_case (e.g., `radio_init`)
  - Variables: snake_case (e.g., `packets_received`)
  - Constants: UPPER_SNAKE_CASE (e.g., `NRF24_CE_PIN`)
  - Structs: PascalCase (e.g., `RadioComm`)
- **Headers**: Use `#pragma once` for include guards
- **Includes**: System headers first, then local headers with relative paths
- **Types**: Use standard integer types (`uint8_t`, `uint32_t`, etc.)
- **Error Handling**: Return bool for success/failure, check return values, use ESP_LOG for logging
- **Comments**: Minimal, only for complex hardware interactions
- **Git Commits**: Never use opencode user in git commit messages

## Architecture
- **Hardware Abstraction**: ESP-IDF drivers for ESP32 board in main.c, radio logic in radio_comm.c/h
- **Radio Layer**: nRF24L01+ communication using radio-common library from parent folder
- **Data Structures**: TimeData struct for packet format (3 bytes: seconds_high, seconds_low, sequence)
- **GPIO**: ESP32 board pins defined as constants (CE=5, CSN=4, LED=2)
- **SPI**: 1MHz clock, MSB first, Mode 0 using ESP-IDF SPI driver
- **FreeRTOS**: Main loop in app_main task, use vTaskDelay for timing
- **Components**: Modular ESP-IDF component architecture with radio-common as separate component

## Compatibility
- **Protocol**: Compatible with existing ESP32 board controller and playclock modules
- **Radio Settings**: Same channel (76), address (0xE7E7E7E7E7), data rate (1Mbps)
- **Packet Format**: 3-byte TimeData structure (seconds_high, seconds_low, sequence)
- **Auto-ACK**: Disabled for transparent packet forwarding

## Testing
No automated test framework - verify via serial monitor and hardware testing.

