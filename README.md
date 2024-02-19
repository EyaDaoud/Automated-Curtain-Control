<div style="background-color: #ffb6c1; padding: 20px;">

  # ESP32 Automated-Curtain-Control
This project involves an ESP32-based automated curtain control system that uses sensors and actuators to adjust the curtain position based on environmental conditions and user preferences.

## Features

- Automatic mode: The curtain opens or closes based on environmental conditions.
- Manual mode: Users can manually control the curtain position.
- MQTT Communication: The system communicates with an MQTT broker for remote control and status updates.

## Hardware Components

- ESP32 microcontroller
- Servo motor
- LDR (Light Dependent Resistor)
- PIR (Passive Infrared) sensor
- RGB LED for status indication
- Real-Time Clock (RTC) module

## Dependencies

- [ESP32Servo](https://github.com/jkb-git/ESP32Servo)
- [RTClib](https://github.com/adafruit/RTClib)
- [WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
- [PubSubClient](https://github.com/knolleary/pubsubclient)

## Setup

1. Wiring on Wokwi:
   - Connect the components on the Wokwi platform as shown in the provided circuit diagram.

2. Install Libraries:
   - For Wokwi simulations, you don't need to manually install libraries. Wokwi automatically includes the necessary libraries.

3. Configure Simulation Settings:
   - Open the simulation in Wokwi and configure any simulation-specific settings.
     
4. Run the code of the interface:
   - Open a new python file write the code and run it. 

5. Run the Simulation:
   - Start the simulation in Wokwi to observe the behavior of the ESP32 curtain control system.
   - Use the GUI to see the changes in the simulation in wokwi 
## MQTT Topics

- etat: Curtain status updates
- manuell: Manual mode control
- auto: Automatic mode control
- lum: Luminosity sensor readings
- time_ouv: Opening time setting
- time_ferm: Closing time setting
- mode : automatic and manual mode
- possible 

## Usage

- Monitor MQTT messages for control and status updates.
- Adjust settings such as opening and closing times through MQTT topics.
- Observe the system's behavior based on environmental conditions.

## Wokwi Simulation

- The project is configured for simulation on the Wokwi platform.
- Visit [[Wokwi](https://wokwi.com/)](https://wokwi.com/projects/390099968992321537) and open the project to run the simulation.

## Prerequisites

- Python: Make sure you have Python installed. You can download it from [python.org](https://www.python.org/downloads/).

## Dependencies

- tkinter: Tkinter is the standard GUI toolkit for Python.
- paho-mqtt: A Python client library for MQTT.

Install the dependencies using:

```bash
pip install tk
pip install paho-mqtt
  


</div>
