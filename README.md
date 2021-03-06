# Multi-room Temperature and Humidity Monitor and Control (w/ ThingsBoard)
This repository is created as a part for fulfilling the requirement to the course SEP 769 Cyber Physical Systems. 

A demostration video can be found [here](https://youtu.be/9qk1WlgCuhU).

## Introduction
This project focuses on setting up a multi-room temperature and humidity monitoring and control system. 
An Arduino MKR1000 and a Raspberry Pi was used in this project to collect data from DHT11 sensor and controls the actuators. A TP-Link HS110 and a servo motor were used as the actuators in this project. A circulator fan was connected to the HS110 smart plug whileas the servo motor was used to control a wall switch for a exhaust fan. 

The ThingsBoard community edition was used to provide the MQTT Broker, data storage, RPC Calls and dashboard interface. Both MKR1000 and Raspberry Pi were congifured to talk to the MQTT Broker hosted by ThingsBoard to update the temperature and humidity readings. Remote procedure calls (RPC) was used to send the remote control signals to both edge devices. Two local automatic control loop were also setup on each device to perform automatic actions by default

## Sample Setup Image
![image](https://user-images.githubusercontent.com/92807082/172660474-9659bb38-6566-4377-bbda-a667e23df6ae.png)

## Material Used
- Raspberry Pi 4
- Arduino MKR1000 WIFI
- Breadboard (x2)
- Servo Motor (SG90)
- TP-Link Smart Plug HS110
- Air Circulator Fan
- 3D-printed Switch Mount 

## Library used
### Arduino
- [ThingsBoard SDK](https://github.com/thingsboard/thingsboard-arduino-sdk)
- [Adafruit DHT sensor](https://github.com/adafruit/DHT-sensor-library)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [pubsubClient](https://github.com/knolleary/pubsubclient)

### Python
- [Python Kasa](https://github.com/python-kasa/python-kasa)
- [Adafruit CircuitPython DHT](https://github.com/adafruit/Adafruit_CircuitPython_DHT)
- [Paho MQTT](https://github.com/eclipse/paho.mqtt.python)

## Code
### Arduino Code
The sketch used in this project can also be found on [here](https://create.arduino.cc/editor/shawnjxy/407df74e-2717-4218-bc97-e27b5a20a7ff/preview). 

## System Schematic
### System Block Diagram
![system_block](https://user-images.githubusercontent.com/92807082/172657020-3608587c-b586-46b1-976d-ac3e25bba319.png)

### Local Automatic Logic
![local_control_logic](https://user-images.githubusercontent.com/92807082/172954502-f8ae474c-7f46-45b6-a02a-756d40392679.png)

### Cloud Control Logic
![cloud_control_logic](https://user-images.githubusercontent.com/92807082/172954483-6ec13849-fb4e-4e6e-abb3-db98ba5b8e54.png)

