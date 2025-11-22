# Repeater Module Wiring

## ESP8266 NodeMCU to nRF24L01+ Connection

### Pin Connections

| NodeMCU Pin | GPIO Number | nRF24L01+ Pin | Wire Color |
|-------------|-------------|---------------|------------|
| D4          | GPIO2       | CE            | Blue       |
| D8          | GPIO15      | CSN           | Yellow     |
| D7          | GPIO13      | MOSI          | Green      |
| D6          | GPIO12      | MISO          | Orange     |
| D5          | GPIO14      | SCK           | White      |
| 3V3         | -           | VCC           | Red        |
| GND         | -           | GND           | Black      |

### Status LED
- Built-in LED on GPIO16 (D0) shows network activity

## Wiring Diagram

```
NodeMCU                nRF24L01+
-------                ----------
D4 (GPIO2) ----------- CE
D8 (GPIO15) ---------- CSN  
D7 (GPIO13) ---------- MOSI
D6 (GPIO12) ---------- MISO
D5 (GPIO14) ---------- SCK
3V3 ------------------ VCC
GND ------------------ GND

Built-in LED (GPIO16) - Status Indicator
```

## Power Considerations

- Use NodeMCU's 3V3 pin for nRF24L01+ power (max 12mA draw)
- For better stability, consider external 3.3V supply with capacitor
- Add 10µF capacitor across VCC/GND close to nRF24L01+ module

## Antenna Placement

- Position nRF24L01+ antenna vertically for best range
- Keep away from metal objects and power lines
- Use PCB antenna version for better range than ceramic antenna

## Testing

1. Verify connections with multimeter
2. Check for 3.3V on VCC pin
3. Monitor status LED - should blink rapidly when forwarding packets
4. Use serial monitor at 115200 baud for debug output

## Troubleshooting

- **No LED activity**: Check power and CE/CSN connections
- **Erratic behavior**: Add 10µF capacitor across power pins
- **Poor range**: Check antenna orientation and interference sources