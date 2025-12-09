# sensor-cli

## Description
This project includes a CLI which controls and displays information for a humidity and temperature sensor. CLI will be implemented using UART.

## Hardware
- STM32F446RE
- DHT11

## Driver
- Following communication process as in: https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf
- Driver created for GPIO with Open Drain with 5k pull-up resistor
- Bitmap used to store DHT11 responses due to small size and efficiency
