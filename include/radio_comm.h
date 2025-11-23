#pragma once

#include "driver/gpio.h"
#include <stdint.h>
#include <stdbool.h>
#include "../../radio-common/include/radio_common.h"
#include "../../radio-common/include/radio_config.h"





// Radio communication structure - extends RadioCommon with repeater-specific fields
typedef struct {
    RadioCommon base;  // radio-common base structure
    
    // Statistics
    uint32_t packets_received;
    uint32_t packets_retransmitted;
    uint32_t last_activity_time;
    bool link_active;
} RadioComm;

// Time data structure (matches controller format)
typedef struct {
    uint16_t seconds;
    uint8_t sequence;
} TimeData;

// Function declarations
bool radio_init(RadioComm *radio);
bool radio_receive_packet(RadioComm *radio, TimeData *data);
bool radio_retransmit_packet(RadioComm *radio, const TimeData *data);
void radio_start_listening(RadioComm *radio);
void radio_stop_listening(RadioComm *radio);
bool radio_is_data_available(RadioComm *radio);
void radio_flush_rx(RadioComm *radio);
void radio_flush_tx(RadioComm *radio);

// Low-level functions - use radio-common implementations

// GPIO functions
void gpio_init(void);
void gpio_write(uint8_t pin, bool level);
bool gpio_read(uint8_t pin);

// SPI functions
void spi_init(void);
uint8_t spi_transfer(uint8_t data);

// Arduino compatibility constants
#define HIGH true
#define LOW false

// Utility functions
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
uint32_t millis(void);
void update_status_led(bool active);

// Getters for statistics
RadioComm* get_radio_instance(void);