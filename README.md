# Micromouse

MicroMouse is a small autonomous robot designed to navigate and solve a maze completely on its own.
![alt text](Assets/About-the-project.jpg)

## About The Project

Our project is about a wall-following micromouse robot built on ESP32-S3, using IR proximity sensors for wall-centering and junction detection, a VL53L0X ToF sensor for front-wall stopping, quadrature encoders for wheel feedback, and a WiFi-based web UI for live tuning of PID gains, thresholds, and duty cycle limits.

- <p>
  <a href="https://youtu.be/bc_Xk7g2EeM" target="_blank">
    <img src="https://img.youtube.com/vi/bc_Xk7g2EeM/maxresdefault.jpg" width="48%" alt="PCB Moving Demo" />
  </a>
- <a href="https://youtube.com/shorts/5PlMwJj3upM" target="_blank">
    <img src="https://img.youtube.com/vi/5PlMwJj3upM/maxresdefault.jpg" width="48%" alt="PCB U-Turn Demo" />
  </a>
</p>

## Project Overview

### Hardware Used
- 1x ESP32-S3
- 1x DRV8833 Motor Driver
- 1x AMS1117 Buck Converter
- 2x DC encoder Motors
- 4x IR sensors
- 1x VL53L0X (Time of flight sensor)

### Pinout Mapping

| Component | Function / Pin | ESP32-S3 GPIO |
| :--- | :--- | :--- |
| **ToF Sensor** | I2C SDA / SCL | GPIO 41 / GPIO 42 |
| **IR Right (`IR_PIN_0`)** | ADC Input | ADC1 CH2 GPIO 3 |
| **IR Left (`IR_PIN_1`)** | ADC Input | ADC1 CH8 GPIO 9|
| **IR Left Diag (`IR_PIN_2`)** | ADC Input | ADC1 CH9 GPIO 10|
| **IR Right Diag (`IR_PIN_3`)**| ADC Input | ADC1 CH7 GPIO 8|
| **Encoder Motors** | Channel A | GPIO 5 / GPIO 4 |
| **Encoder Motors** | Channel B | GPIO 6 / GPIO 7 |

# Repository Structure

Each subdirectory has its own README with details specific to that piece:

| Directory | Contents |
| :--- | :--- |
| [`LeftWallFollow`](LeftWallFollow/README.md) | Full left-hand-follow navigation, PD wall-centering, ToF stopping, WiFi live tuning |
| [`Test-Codes`](Test-Codes/README.md) | Reference codes for development stages along the project |
| [`PCB`](PCB/README.md) | KiCad schematic and board layout |


## CAD and PCB Models

### PCB Model

![alt text](Assets/Schematic-and-PCB.jpg)

### CAD Models

![alt text](Assets/CAD-Models.jpg)

## Installation and Setup

Prerequisites:
1. ESP-IDF v5.5 
2. Git
3. Python 3.10 or higher

**Setup:**
1. **Clone the Repository**
   ```bash
   git clone https://github.com/typewriter13/Micromouse
   cd Micromouse
   cd LeftWallFollow
2. **Configure and Build**
   ```bash
   idf.py set-target esp32s3
   idf.py build
3. **Flashing the Project**
   ```bash
   idf.py flash monitor

## Future Work

- Implementing Left hand Follow algorithm tuned according to the Maze which we have designed.
- Work and study many more algorithms which are more efficient than LHF.
- Integrate the encoder readings with our path calculations of the bot for mapping the maze and reading the distance travelled.
