#include <Arduino.h>
#include <SPI.h>
#include "../include/radio_comm.h"

// Arduino-specific implementations
void delay_ms(uint32_t ms) {
    delay(ms);
}

void delay_us(uint32_t us) {
    delayMicroseconds(us);
}

uint32_t millis(void) {
    return millis();
}

void gpio_write(uint8_t pin, bool level) {
    digitalWrite(pin, level ? HIGH : LOW);
}

bool gpio_read(uint8_t pin) {
    return digitalRead(pin) == HIGH;
}

void gpio_init(void) {
    pinMode(NRF24_CE_PIN, OUTPUT);
    pinMode(NRF24_CSN_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    
    // Initialize to safe states
    digitalWrite(NRF24_CE_PIN, LOW);
    digitalWrite(NRF24_CSN_PIN, HIGH);
    digitalWrite(STATUS_LED_PIN, LOW);
}

void spi_init(void) {
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 4MHz
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
}

uint8_t spi_transfer(uint8_t data) {
    return SPI.transfer(data);
}

// Global variables
static RadioComm *radio;
static TimeData last_received_data;
static bool data_received = false;

void setup() {
    Serial.begin(115200);
    Serial.println("Repeater starting...");
    
    // Get radio instance
    radio = get_radio_instance();
    
    // Initialize radio
    if (!radio_init(radio)) {
        Serial.println("Radio initialization failed!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(100);
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(100);
        }
    }
    
    Serial.println("Radio initialized successfully");
    
    // Start listening
    radio_start_listening(radio);
    
    Serial.println("Repeater ready - listening for packets...");
}

void loop() {
    static uint32_t last_stats_time = 0;
    static uint32_t last_activity_check = 0;
    
    // Try to receive a packet
    TimeData received_data;
    if (radio_receive_packet(radio, &received_data)) {
        // Store the received data
        last_received_data = received_data;
        data_received = true;
        
        Serial.print("Received packet: seconds=");
        Serial.print(received_data.seconds);
        Serial.print(", sequence=");
        Serial.println(received_data.sequence);
        
        // Immediately retransmit the packet
        if (radio_retransmit_packet(radio, &received_data)) {
            Serial.println("Packet retransmitted successfully");
        } else {
            Serial.println("Packet retransmission failed");
        }
    }
    
    // Check for link activity timeout
    if (millis() - radio->last_activity_time > 5000) { // 5 seconds timeout
        radio->link_active = false;
    }
    
    // Update status LED based on activity
    update_status_led(radio->link_active);
    
    // Print statistics every 30 seconds
    if (millis() - last_stats_time > 30000) {
        last_stats_time = millis();
        
        Serial.println("=== Repeater Statistics ===");
        Serial.print("Packets received: ");
        Serial.println(radio->packets_received);
        Serial.print("Packets retransmitted: ");
        Serial.println(radio->packets_retransmitted);
        Serial.print("Link active: ");
        Serial.println(radio->link_active ? "Yes" : "No");
        
        if (data_received) {
            Serial.print("Last data: seconds=");
            Serial.print(last_received_data.seconds);
            Serial.print(", sequence=");
            Serial.println(last_received_data.sequence);
        }
        Serial.println("===========================");
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}