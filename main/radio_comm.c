#include "../include/radio_comm.h"
#include <string.h>

// Global radio instance
static RadioComm g_radio;

// Simple delay implementation
void delay_us(uint32_t us) {
    // Simple busy-wait delay for microsecond timing
    // This is approximate and may need calibration
    volatile uint32_t count = us * 10; // Rough approximation
    while (count--) {
        __asm__ volatile ("nop");
    }
}

// GPIO functions
void gpio_init(void) {
    // ESP8266 GPIO initialization would go here
    // For now, we'll assume this is handled by the ESP8266 Arduino framework
}

void gpio_write(uint8_t pin, bool level) {
    // ESP8266 GPIO write would go here
    // This will be implemented in the main Arduino file
}

bool gpio_read(uint8_t pin) {
    // ESP8266 GPIO read would go here
    // This will be implemented in the main Arduino file
    return false;
}

// SPI functions
void spi_init(void) {
    // ESP8266 SPI initialization would go here
    // This will be handled by the ESP8266 Arduino framework
}

uint8_t spi_transfer(uint8_t data) {
    // ESP8266 SPI transfer would go here
    // This will be implemented in the main Arduino file
    return 0;
}

// Utility functions
void delay_ms(uint32_t ms) {
    // This will be implemented in the main Arduino file
}

void update_status_led(bool active) {
    static uint32_t last_blink = 0;
    static bool blink_state = false;
    
    if (active) {
        // Fast blink when active
        if (millis() - last_blink > 200) {
            blink_state = !blink_state;
            gpio_write(STATUS_LED_PIN, blink_state);
            last_blink = millis();
        }
    } else {
        // Slow blink when idle
        if (millis() - last_blink > 1000) {
            blink_state = !blink_state;
            gpio_write(STATUS_LED_PIN, blink_state);
            last_blink = millis();
        }
    }
}

// Low-level nRF24L01+ functions
uint8_t nrf24_read_register(RadioComm *radio, uint8_t reg) {
    gpio_write(radio->csn_pin, LOW);
    spi_transfer(NRF24_CMD_R_REGISTER | (reg & 0x1F));
    uint8_t result = spi_transfer(NRF24_CMD_NOP);
    gpio_write(radio->csn_pin, HIGH);
    return result;
}

bool nrf24_write_register(RadioComm *radio, uint8_t reg, uint8_t value) {
    gpio_write(radio->csn_pin, LOW);
    uint8_t status = spi_transfer(NRF24_CMD_W_REGISTER | (reg & 0x1F));
    spi_transfer(value);
    gpio_write(radio->csn_pin, HIGH);
    return (status & 0x20) == 0; // Check if write succeeded
}

uint8_t nrf24_get_status(RadioComm *radio) {
    gpio_write(radio->csn_pin, LOW);
    uint8_t status = spi_transfer(NRF24_CMD_NOP);
    gpio_write(radio->csn_pin, HIGH);
    return status;
}

bool nrf24_read_payload(RadioComm *radio, uint8_t *data, uint8_t length) {
    gpio_write(radio->csn_pin, LOW);
    spi_transfer(NRF24_CMD_RX_PAYLOAD);
    for (uint8_t i = 0; i < length; i++) {
        data[i] = spi_transfer(NRF24_CMD_NOP);
    }
    gpio_write(radio->csn_pin, HIGH);
    return true;
}

bool nrf24_write_payload(RadioComm *radio, uint8_t *data, uint8_t length) {
    gpio_write(radio->csn_pin, LOW);
    spi_transfer(NRF24_CMD_TX_PAYLOAD);
    for (uint8_t i = 0; i < length; i++) {
        spi_transfer(data[i]);
    }
    gpio_write(radio->csn_pin, HIGH);
    return true;
}

void nrf24_power_up(RadioComm *radio) {
    uint8_t config = nrf24_read_register(radio, NRF24_REG_CONFIG);
    config |= NRF24_CONFIG_PWR_UP;
    nrf24_write_register(radio, NRF24_REG_CONFIG, config);
    delay_ms(2); // Power-up delay
}

void nrf24_power_down(RadioComm *radio) {
    uint8_t config = nrf24_read_register(radio, NRF24_REG_CONFIG);
    config &= ~NRF24_CONFIG_PWR_UP;
    nrf24_write_register(radio, NRF24_REG_CONFIG, config);
}

// High-level radio functions
bool radio_init(RadioComm *radio) {
    radio->initialized = false;
    radio->ce_pin = NRF24_CE_PIN;
    radio->csn_pin = NRF24_CSN_PIN;
    radio->packets_received = 0;
    radio->packets_retransmitted = 0;
    radio->last_activity_time = 0;
    radio->link_active = false;
    
    // Set network address
    uint8_t addr[] = NETWORK_ADDRESS;
    memcpy(radio->tx_address, addr, 5);
    memcpy(radio->rx_address, addr, 5);
    
    // Initialize hardware
    gpio_init();
    spi_init();
    
    // Power down first
    nrf24_power_down(radio);
    delay_ms(10);
    
    // Power up
    nrf24_power_up(radio);
    
    // Configure radio settings
    if (!nrf24_write_register(radio, NRF24_REG_RF_CH, NETWORK_CHANNEL)) return false;
    if (!nrf24_write_register(radio, NRF24_REG_RF_SETUP, 0x06)) return false; // 1Mbps, 0dBm
    if (!nrf24_write_register(radio, NRF24_REG_SETUP_AW, 0x03)) return false; // 5-byte address
    if (!nrf24_write_register(radio, NRF24_REG_SETUP_RETR, 0x00)) return false; // No auto-retry
    if (!nrf24_write_register(radio, NRF24_REG_EN_AA, 0x00)) return false; // Disable auto-ack
    if (!nrf24_write_register(radio, NRF24_REG_EN_RXADDR, 0x01)) return false; // Enable pipe 0
    if (!nrf24_write_register(radio, NRF24_REG_RX_PW_P0, NRF24_PAYLOAD_SIZE)) return false;
    
    // Set addresses
    gpio_write(radio->csn_pin, LOW);
    spi_transfer(NRF24_CMD_W_REGISTER | NRF24_REG_RX_ADDR_P0);
    for (int i = 0; i < 5; i++) {
        spi_transfer(radio->rx_address[i]);
    }
    gpio_write(radio->csn_pin, HIGH);
    
    gpio_write(radio->csn_pin, LOW);
    spi_transfer(NRF24_CMD_W_REGISTER | NRF24_REG_TX_ADDR);
    for (int i = 0; i < 5; i++) {
        spi_transfer(radio->tx_address[i]);
    }
    gpio_write(radio->csn_pin, HIGH);
    
    // Enable CRC and set to primary receiver mode
    if (!nrf24_write_register(radio, NRF24_REG_CONFIG, 
                              NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP | NRF24_CONFIG_PRIM_RX)) return false;
    
    // Clear status flags
    nrf24_write_register(radio, NRF24_REG_STATUS, 0x70);
    
    // Flush buffers
    radio_flush_rx(radio);
    radio_flush_tx(radio);
    
    radio->initialized = true;
    return true;
}

void radio_start_listening(RadioComm *radio) {
    gpio_write(radio->ce_pin, HIGH);
    delay_us(130); // Delay to enter RX mode
}

void radio_stop_listening(RadioComm *radio) {
    gpio_write(radio->ce_pin, LOW);
}

bool radio_is_data_available(RadioComm *radio) {
    uint8_t status = nrf24_get_status(radio);
    return (status & NRF24_STATUS_RX_DR) != 0;
}

void radio_flush_rx(RadioComm *radio) {
    gpio_write(radio->csn_pin, LOW);
    spi_transfer(NRF24_CMD_FLUSH_RX);
    gpio_write(radio->csn_pin, HIGH);
}

void radio_flush_tx(RadioComm *radio) {
    gpio_write(radio->csn_pin, LOW);
    spi_transfer(NRF24_CMD_FLUSH_TX);
    gpio_write(radio->csn_pin, HIGH);
}

bool radio_receive_packet(RadioComm *radio, TimeData *data) {
    if (!radio->initialized) return false;
    
    if (!radio_is_data_available(radio)) return false;
    
    uint8_t payload[NRF24_PAYLOAD_SIZE];
    if (nrf24_read_payload(radio, payload, sizeof(payload))) {
        // Parse time data (same format as controller)
        data->seconds = (payload[0] << 8) | payload[1];
        data->sequence = payload[2];
        
        radio->packets_received++;
        radio->last_activity_time = millis();
        radio->link_active = true;
        
        // Clear RX_DR flag
        nrf24_write_register(radio, NRF24_REG_STATUS, NRF24_STATUS_RX_DR);
        
        return true;
    }
    
    return false;
}

bool radio_retransmit_packet(RadioComm *radio, const TimeData *data) {
    if (!radio->initialized) return false;
    
    // Stop listening
    radio_stop_listening(radio);
    
    // Switch to TX mode
    nrf24_write_register(radio, NRF24_REG_CONFIG, 
                          NRF24_CONFIG_EN_CRC | NRF24_CONFIG_PWR_UP);
    
    // Prepare payload (same format as controller)
    uint8_t payload[NRF24_PAYLOAD_SIZE] = {0};
    payload[0] = (data->seconds >> 8) & 0xFF;
    payload[1] = data->seconds & 0xFF;
    payload[2] = data->sequence;
    
    // Transmit
    bool success = nrf24_write_payload(radio, payload, sizeof(payload));
    
    // Pulse CE to start transmission
    gpio_write(radio->ce_pin, HIGH);
    delay_us(15); // Minimum pulse width
    gpio_write(radio->ce_pin, LOW);
    
    // Wait for transmission to complete
    delay_ms(2);
    
    // Check status
    uint8_t status = nrf24_get_status(radio);
    success = success && ((status & NRF24_STATUS_TX_DS) != 0);
    
    // Clear status flags
    nrf24_write_register(radio, NRF24_REG_STATUS, 0x70);
    
    // Switch back to RX mode
    nrf24_write_register(radio, NRF24_REG_CONFIG, 
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