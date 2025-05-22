# IoT Home Project

The IoT Home project is a smart home monitoring and automation system designed around the ESP32 microcontroller. This project integrates various sensors and actuators to monitor environmental conditions and control home appliances, enhancing safety and convenience. It utilizes the ERa platform for remote monitoring and control through a mobile app or dashboard.

## Features
- **Environmental Monitoring**: 
  - Measures temperature and humidity in three rooms using DHT11 sensors.
  - Detects flames in three rooms with flame sensors to warn of fire hazards.
  - Identifies gas leaks in three rooms using MQ2 sensors.
- **Automation**: 
  - Controls a servo motor to automatically open/close a door based on infrared sensor detection.
  - Adjusts lighting based on ambient light levels using a photoresistor, with manual control via the ERa app.
  - Triggers a buzzer for alerts in case of fire or gas detection.
- **Display**: Shows real-time temperature and humidity data on a 16x2 LCD screen.
- **Connectivity**: Connects to the ERa platform via Wi-Fi for remote monitoring and control.

## Platform
- **Microcontroller**: ESP32 (ESP-WROOM-32 module).
- **Development Environment**: Arduino IDE with ESP32 support.
- **Cloud Platform**: ERa for remote access and management.
- **Communication Protocol**: Wi-Fi.

## Hardware Components
- **Sensors**: 
  - DHT11 (x3) for temperature and humidity monitoring.
  - MQ2 gas sensors (x3) for gas leak detection.
  - Flame sensors (x3) for fire detection.
  - Infrared sensor for door automation.
  - Photoresistor for ambient light sensing.
- **Actuators**: 
  - Servo motor for door control.
  - Buzzer for audible alerts.
  - Relay for light control.
- **Power Supply**: 12V input with an LM2596 voltage regulator providing 5V and 3.3V.
- **Display**: 16x2 LCD with I2C interface.
- **Other**: Resistors, capacitors, and connectors for circuit stability.

## Software
- **Libraries**: 
  - `Wire.h` for I2C communication.
  - `LiquidCrystal_I2C.h` for LCD operation.
  - `ESP32Servo.h` for servo motor control.
  - `DHT.h` for DHT11 sensor data.
  - `freertos/task.h` for task management.
  - ERa-specific libraries for smart features and time synchronization.
- **Code**: 
  - Initializes sensors, actuators, and the LCD.
  - Reads sensor data and updates global variables.
  - Transmits data to the ERa platform using virtual pins.
  - Manages buzzer alerts, servo operation, relay-based light control, and LCD updates.
  - Enables remote light control via the ERa app.

## Setup Instructions
1. **Hardware Assembly**: 
   - Assemble the circuit according to the provided schematic and PCB layout.
   - Connect the ESP32, sensors, actuators, and power supply as specified.
   - Ensure proper grounding and power distribution.
2. **Software Configuration**: 
   - Install the Arduino IDE and add ESP32 board support.
   - Install the required libraries via the Library Manager.
   - Update the `ssid` and `pass` in the code with your Wi-Fi credentials.
   - Set the `ERA_AUTH_TOKEN` to your ERa authentication token.
   - Upload the `code.ino` file to the ESP32.
3. **ERa Integration**: 
   - Register the device on the ERa platform using the provided token.
   - Use the ERa app or dashboard to monitor sensor data and control the light.

## Usage
- The LCD displays real-time temperature and humidity readings for the three rooms.
- The buzzer sounds an alert if fire or gas is detected.
- The door opens automatically upon detecting a person via the infrared sensor and closes after 2.5 seconds.
- The light turns on/off based on ambient light levels or manual control through the ERa app.

## Illustrations
- **<span style="font-size: 1.6em;">Schematic</span>**  
  <img src="image\Schematic.png" alt="Alt text" width="60%">
- **<span style="font-size: 1.6em;">3D View</span>**  
  <img src="image\3D_View.png" alt="Alt text" width="40%">  
- **<span style="font-size: 1.6em;">PCB_Layout</span>**  
  <img src="image\PCB_Layout.png" alt="Alt text" width="40%">
- **<span style="font-size: 1.6em;">User_Interface - Screen</span>**  
  <img src="image\Era_Interface.png" alt="Alt text" width="40%">
## Video Demo
<video width="560" height="315" controls>
  <source src="video_demo\demo_vid.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

## Future Improvements
- Add real-time notifications through the ERa platform for alerts.
- Incorporate additional sensors, such as motion detectors.
- Develop a web interface to complement the ERa app.

This project lays the groundwork for a robust smart home system, combining environmental monitoring, automation, and remote access to improve home safety and efficiency.