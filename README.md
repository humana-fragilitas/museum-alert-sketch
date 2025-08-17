# Museum Alert Sensor

This repository contains the electrical schematic and Arduino sketch required to build the "Museum Alert Sensor (MAS)" module, part of the ["Museum Alert"](https://github.com/humana-fragilitas/museum-alert) project.

## Electrical schematic
![alt text](./docs/images/electrical_schematic.svg "Museum Alert Sensor Electrical Schematic")

## Bill of materials

| Q.ty   | Component                                            |
|--------|------------------------------------------------------|
| 1      | Breadboard (5 columns on each side, dual power rails)|
| 1      | Arduino® Nano ESP32                                  |
| 1      | HC-SR04 ultrasonic sensor                            |
| 3      | Green led                                            |
| 1      | Red led                                              |
| 1      | 4-pin push button                                    |
| 4      | 220Ω resistor                                        |
| 4      | Pin Jumper Wire (Male-Female)                        |
| 6      | Pin Jumper Wire (Male-Male)                          |

## arduino® Nano ESP32 specifications summary

|                  |                                  |                                                                       |
|------------------|----------------------------------|-----------------------------------------------------------------------|
| **Board**        | Name                             | Arduino® Nano ESP32                                                   |
|                  | SKU                              | ABX00092                                                              |
| **Microcontroller** | –                             | u-blox® NORA-W106 (ESP32-S3)                                          |
| **USB connector**   | –                             | USB-C®                                                                |
| **Pins**         | Built-in LED Pin                 | 13                                                                    |
|                  | Built-in RGB LED pins            | 14–16                                                                 |
|                  | Digital I/O Pins                 | 14                                                                    |
|                  | Analog input pins                | 8                                                                     |
|                  | PWM pins                         | 5                                                                     |
|                  | External interrupts              | All digital pins                                                      |
| **Connectivity** | Wi-Fi®                           | u-blox® NORA-W106 (ESP32-S3)                                          |
|                  | Bluetooth®                       | u-blox® NORA-W106 (ESP32-S3)                                          |
| **Communication**| UART                             | 2x                                                                    |
|                  | I2C                              | 1x, A4 (SDA), A5 (SCL)                                                |
|                  | SPI                              | D11 (COPI), D12 (CIPO), D13 (SCK); any GPIO for Chip Select (CS)      |
| **Power**        | I/O Voltage                      | 3.3 V                                                                 |
|                  | Input voltage (nominal)          | 6–21 V                                                                |
|                  | Source Current per I/O Pin       | 40 mA                                                                 |
|                  | Sink Current per I/O Pin         | 28 mA                                                                 |
| **Clock speed**  | Processor                        | up to 240 MHz                                                         |
| **Memory**       | ROM                              | 384 kB                                                                |
|                  | SRAM                             | 512 kB                                                                |
|                  | External Flash                   | 128 Mbit (16 MB)                                                      |
|                  | RAM                              | 8 MB (NORA-W106-10B)                                                  |
| **Dimensions**   | Width                            | 18 mm                                                                 |
|                  | Length                           | 45 mm                                                                 |

Full technical specifications are available on [manufactorer's website](https://store.arduino.cc/products/nano-esp32).

## HC-SR04 ultrasonic sensor specifications summary

|                |                     |                           |
|----------------|---------------------|---------------------------|
| General        | Model               | HC-SR04                   |
|                | Sensor Type         | Ultrasonic Distance Sensor|
|                | Operating Principle | Echo ranging              |
| Electrical     | Operating Voltage   | 5 V                       |
|                | Operating Current   | 15 mA                     |
|                | Working Frequency   | 40 kHz                    |
| Performance    | Measuring Range     | 2 cm – 400 cm             |
|                | Accuracy            | ±3 mm                     |
|                | Effectual Angle     | < 15°                     |
|                | Resolution          | ~0.3 cm                   |
| Interface      | Trigger Input Pulse | ≥10 µs                    |
|                | Echo Output Pulse   | Proportional to distance  |
|                | Interface Type      | Digital (TTL)             |
| Timing         | Measuring Cycle     | ~60 ms                    |
|                | Response Time       | ~750 µs                   |
| Physical       | Dimensions          | 45 mm × 20 mm × 15 mm     |
|                | Weight              | ~10 g                     |

