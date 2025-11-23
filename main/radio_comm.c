#include "../include/radio_comm.h"
#include <string.h>

// Global radio instance
static RadioComm g_radio;

// Function declarations - implementations are in main.c
extern void gpio_write(uint8_t pin, bool level);
extern void delay_ms(uint32_t ms);
extern void delay_us(uint32_t us);
extern uint32_t millis(void);

void update_status_led(bool active) {
    static uint32_t last_blink = 0;
    static bool blink_state = false;
    
    if (active) {
        // Fast blink when active
        if (millis() - last_blink > 200) {
            blink_state = !blink_state;
            gpio_write(RADIO_STATUS_LED_PIN, blink_state);
            last_blink = millis();
        }
    } else {
        // Slow blink when idle
        if (millis() - last_blink > 1000) {
            blink_state = !blink_state;
            gpio_write(RADIO_STATUS_LED_PIN, blink_state);
            last_blink = millis();
        }
    }
}

// Low-level nRF24L01+ functions - use radio-common implementations

// High-level radio functions
bool radio_init(RadioComm *radio) {
    radio->base.initialized = false;
    radio->base.ce_pin = RADIO_CE_PIN;
    radio->base.csn_pin = RADIO_CSN_PIN;
    radio->packets_received = 0;
    radio->packets_retransmitted = 0;
    radio->last_activity_time = 0;
    radio->link_active = false;
    
    // Initialize using radio-common (same as play_clock/controller)
    if (!radio_common_init(&radio->base, RADIO_CE_PIN, RADIO_CSN_PIN)) {
        return false;
    }
    
    // Configure radio settings
    if (!radio_common_configure(&radio->base)) {
        return false;
    }
    
    // Set network address
    uint8_t addr[] = RADIO_ADDRESS;
    if (!radio_common_set_addresses(&radio->base, addr, addr)) {
        return false;
    }
    
    radio->base.initialized = true;
    return true;
}

void radio_start_listening(RadioComm *radio) {
    gpio_write(radio->base.ce_pin, HIGH);
    delay_us(130); // Delay to enter RX mode
}

void radio_stop_listening(RadioComm *radio) {
    gpio_write(radio->base.ce_pin, LOW);
}

bool radio_is_data_available(RadioComm *radio) {
    uint8_t status = nrf24_get_status(&radio->base);
    return (status & NRF24_STATUS_RX_DR) != 0;
}

void radio_flush_rx(RadioComm *radio) {
    nrf24_flush_rx(&radio->base);
}

void radio_flush_tx(RadioComm *radio) {
    nrf24_flush_tx(&radio->base);
}

bool radio_receive_packet(RadioComm *radio, TimeData *data) {
    if (!radio->base.initialized) return false;
    
    if (!radio_is_data_available(radio)) return false;
    
    uint8_t payload[RADIO_PAYLOAD_SIZE];
    if (nrf24_read_payload(&radio->base, payload, sizeof(payload))) {
        // Parse time data (same format as controller)
        data->seconds = (payload[0] << 8) | payload[1];
        data->sequence = payload[2];
        
        radio->packets_received++;
        radio->last_activity_time = millis();
        radio->link_active = true;
        
        // Clear RX_DR flag
        nrf24_write_register(&radio->base, NRF24_REG_STATUS, NRF24_STATUS_RX_DR);
        
        return true;
    }
    
    return false;
}

bool radio_retransmit_packet(RadioComm *radio, const TimeData *data) {
    if (!radio->base.initialized) return false;
    
    // Stop listening
    radio_stop_listening(radio);
    
    // Switch to TX mode
    nrf24_write_register(&radio->base, NRF24_REG_CONFIG, 
                          NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP);
    
    // Prepare payload (same format as controller)
    uint8_t payload[RADIO_PAYLOAD_SIZE] = {0};
    payload[0] = (data->seconds >> 8) & 0xFF;
    payload[1] = data->seconds & 0xFF;
    payload[2] = data->sequence;
    
    // Transmit
    bool success = nrf24_write_payload(&radio->base, payload, sizeof(payload));
    
    // Pulse CE to start transmission
    gpio_write(radio->base.ce_pin, HIGH);
    delay_us(15); // Minimum pulse width
    gpio_write(radio->base.ce_pin, LOW);
    
    // Wait for transmission to complete
    delay_ms(2);
    
    // Check status
    uint8_t status = nrf24_get_status(&radio->base);
    success = success && ((status & NRF24_STATUS_TX_DS) != 0);
    
    // Clear status flags
    nrf24_write_register(&radio->base, NRF24_REG_STATUS, 0x70);
    
    // Switch back to RX mode
    nrf24_write_register(&radio->base, NRF24_REG_CONFIG, 
                          NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP | NRF24_CONFIG_PRIM_RX);
    
    // Resume listening
    radio_start_listening(radio);
    
    if (success) {
        radio->packets_retransmitted++;
    }
    
    return success;
}

// Getters for statistics
RadioComm* get_radio_instance(void) {
    return &g_radio;
}