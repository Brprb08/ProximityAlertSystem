# ProximityAlertSystem
A small scale smart access control and proximity alert system using Arduino, featuring ultrasonic distance sensing, IR remote authentication, and real-time notifications on an LCD screen. This project was used to introduce me to embedded systems as well as working a bit with mixing hardware and software. I also wanted to utilize FreeRTOS to gain experience, even though it's not entirenly necessary for a project like this.

## Features
- Ultrasonic sensor for detecting proximity and triggering alerts.
- IR remote control for user authentication.
- Real-time data display on an LCD screen.
- Buzzer and LED alerts based on proximity and authentication status.

## Components Used
- Arduino Uno R3
- Ultrasonic Sensor (HC-SR04)
- IR Receiver Module
- LCD Screen (16x2)
- Buzzer
- LEDs
- 5V Relay Module (if applicable)
- Stepper Motor (if applicable)
- Servo Motor (if applicable)

## Getting Started

### Prerequisites
- Arduino IDE installed
- Git installed for version control

### Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/Brprb08/ProximityAlertSystem.git
Open the .ino file in the Arduino IDE.
Install necessary libraries in the Arduino IDE:
- LiquidCrystal
- FreeRTOS
- IRremote
- Ultrasonic

## Wiring Diagram
![Wiring Diagram](images/wiringDiagram.png)

## How It Works
- The ultrasonic sensor detects when someone approaches the door.
- A buzzer alerts if an unauthorized person is detected.
- The user can authenticate using an IR remote control.
- Successful authentication stops the buzzer and allows access.

## Usage
- Power On: Connect the Arduino to a power source via USB or battery.
- Authentication: Use the IR remote to enter the access code displayed on the LCD.
- Alerts: The system will notify you of unauthorized access attempts.
- Configuration

## Troubleshooting
### Problem: The buzzer keeps beeping non-stop.
- Solution: Check the distance threshold setting and ensure the IR code is correctly entered.

### Problem: No response from the LCD.
- Solution: Verify the wiring and ensure the correct pins are used.
