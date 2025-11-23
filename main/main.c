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

// SPI device handle
static spi_device_handle_t spi_handle;

// Global variables
static RadioComm *radio;
static TimeData last_received_data;
static bool data_received = false;

// ESP-IDF GPIO functions
void gpio_init(void) {
    gpio_config_t io_conf = {};
    
    // Configure CE pin as output
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << RADIO_CE_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    // Configure CSN pin as output
    io_conf.pin_bit_mask = (1ULL << RADIO_CSN_PIN);
    gpio_config(&io_conf);
    
    // Configure Status LED pin as output
    io_conf.pin_bit_mask = (1ULL << RADIO_STATUS_LED_PIN);
    gpio_config(&io_conf);
    
    // Initialize to safe states
    gpio_set_level(RADIO_CE_PIN, 0);
    gpio_set_level(RADIO_CSN_PIN, 1);
    gpio_set_level(RADIO_STATUS_LED_PIN, 0);
}

void gpio_write(uint8_t pin, bool level) {
    gpio_set_level(pin, level ? 1 : 0);
}

bool gpio_read(uint8_t pin) {
    return gpio_get_level(pin) == 1;
}

// ESP-IDF SPI functions
void spi_init(void) {
    spi_bus_config_t buscfg = {
        .mosi_io_num = RADIO_MOSI_PIN,
        .miso_io_num = RADIO_MISO_PIN,
        .sclk_io_num = RADIO_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, // 1MHz
        .mode = 0,  // SPI mode 0
        .spics_io_num = -1,  // CSN controlled manually
        .queue_size = 7,
    };
    
    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));
}

uint8_t spi_transfer(uint8_t data) {
    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &data,
        .rx_buffer = &data,
    };
    
    ESP_ERROR_CHECK(spi_device_transmit(spi_handle, &trans));
    return data;
}

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