idf_component_register(
    SRCS "main.c" "radio_comm.c"
    INCLUDE_DIRS "../include" "../components/radio-common/include"
    REQUIRES driver esp_common esp_driver_gpio esp_driver_spi radio-common
)