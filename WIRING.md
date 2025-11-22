# ESP32 Repeater Module Wiring

## ESP32 Development Board to nRF24L01+ Connection

### Pin Connections

| ESP32 Pin | Function | nRF24L01+ Pin | Wire Color |
|-----------|----------|---------------|------------|
| GPIO5     | CE       | CE            | Blue       |
| GPIO4     | CSN      | CSN           | Yellow     |
| GPIO23    | MOSI     | MOSI          | Green      |
| GPIO19    | MISO     | MISO          | Orange     |
| GPIO18    | SCK      | SCK           | White      |
| 3V3       | Power    | VCC           | Red        |
| GND       | Ground   | GND           | Black      |

### Status LED
- Built-in LED on GPIO2 shows network activity

## Wiring Diagram

```
ESP32 Board            nRF24L01+
-----------            ----------
GPIO5 ---------------- CE
GPIO4 ---------------- CSN  
GPIO23 --------------- MOSI
GPIO19 --------------- MISO
GPIO18 --------------- SCK
3V3 ------------------ VCC
GND ------------------ GND

Built-in LED (GPIO2) - Status Indicator
```

## Power Considerations

- Use ESP32 board's 3V3 pin for nRF24L01+ power (max 12mA draw)
- For better stability, consider external 3.3V supply with capacitor
- Add 10µF capacitor across VCC/GND close to nRF24L01+ module

## Antenna Placement

- Position nRF24L01+ antenna vertically for best range
- Keep away from metal objects and power lines
- Use PCB antenna version for better range than ceramic antenna

## Testing

1. Verify ESP32 board connections with multimeter
2. Check for 3.3V on VCC pin
3. Monitor status LED - should blink rapidly when forwarding packets
4. Use serial monitor at 115200 baud for debug output

## Troubleshooting

- **No LED activity**: Check ESP32 board power and CE/CSN connections
- **Erratic behavior**: Add 10µF capacitor across power pins
- **Poor range**: Check antenna orientation and interference sources