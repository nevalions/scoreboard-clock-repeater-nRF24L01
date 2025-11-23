#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "radio_comm.h"

static const char *TAG = "repeater";



// Global variables
static RadioComm *radio;
static TimeData last_received_data;
static bool data_received = false;

// ESP-IDF timing functions
void delay_ms(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void delay_us(uint32_t us) {
    // Simple busy-wait delay for microsecond timing
    // This is approximate and may need calibration
    volatile uint32_t count = us * 10; // Rough approximation
    while (count--) {
        __asm__ volatile ("nop");
    }
}

uint32_t millis(void) {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

// GPIO functions for status LED
void gpio_write(uint8_t pin, bool level) {
    gpio_set_level(pin, level ? 1 : 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "Repeater starting...");
    
    // Get radio instance
    radio = get_radio_instance();
    
    // Initialize radio
    if (!radio_init(radio)) {
        ESP_LOGE(TAG, "Radio initialization failed!");
        while (1) {
            gpio_write(RADIO_STATUS_LED_PIN, 1);
            delay_ms(100);
            gpio_write(RADIO_STATUS_LED_PIN, 0);
            delay_ms(100);
        }
    }
    
    ESP_LOGI(TAG, "Radio initialized successfully");
    
    // Start listening
    radio_start_listening(radio);
    
    ESP_LOGI(TAG, "Repeater ready - listening for packets...");
    
    // Main loop
    while (1) {
        static uint32_t last_stats_time = 0;
        
        // Try to receive a packet
        TimeData received_data;
        if (radio_receive_packet(radio, &received_data)) {
            // Store the received data
            last_received_data = received_data;
            data_received = true;
            
            ESP_LOGI(TAG, "Received packet: seconds=%d, sequence=%d", 
                     received_data.seconds, received_data.sequence);
            
            // Immediately retransmit the packet
            if (radio_retransmit_packet(radio, &received_data)) {
                ESP_LOGI(TAG, "Packet retransmitted successfully");
            } else {
                ESP_LOGE(TAG, "Packet retransmission failed");
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
            
            ESP_LOGI(TAG, "=== Repeater Statistics ===");
            ESP_LOGI(TAG, "Packets received: %d", radio->packets_received);
            ESP_LOGI(TAG, "Packets retransmitted: %d", radio->packets_retransmitted);
            ESP_LOGI(TAG, "Link active: %s", radio->link_active ? "Yes" : "No");
            
            if (data_received) {
                ESP_LOGI(TAG, "Last data: seconds=%d, sequence=%d", 
                         last_received_data.seconds, last_received_data.sequence);
            }
            ESP_LOGI(TAG, "===========================");
        }
        
        // Small delay to prevent overwhelming the system
        delay_ms(10);
    }
}