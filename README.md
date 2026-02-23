
<!-- PROJECT LOGO -->
<br />
<p align="center">
    <img src="public/favicon.ico" alt="Logo" width="80" height="80">

  <h3 align="center">SMART BIN</h3>

  <p align="center">
    A Project built from Lazyness
    <br />
    <a href="https://github.com/kattapis64/SmartBin/blob/master/SmartBin.ino"><strong>Explore the Main Source Code »</strong></a>
    
  </p>
</p>

## About This Project
This Project is built as Grade 9's final computer project. This is a Trash bin that can walk built from the lazyness of us.

You can use your phone to connect to the bin and control it as far as the internet goes.

## Equipments
Materials Used:
1. ESP32 S3 CAM N16R8 (1 board) and OV2640 camera (1 unit)
2. SG90 continuous servo (2 servos)
3. 5V voltage reducer circuit (1 board)
4. Jumper wires (15 pieces)
5. Breadboard (1 board)
6. Wheels (free-spinning and motorized) (3 wheels)
7. Cardboard box
8. Glue gun, tape, scissors, cutter
9. Two 3.4V Lithium Ion batteries
## Wiring
1. The selected equipment includes ESP32, S3 CAM, N16R8, and OV2640 because a camera is required; a continuous SG90 servo motor is needed for 360-degree rotation and ease of control. And a voltage reducer circuit to 5V to power the servos and MCU.
2. Connect the components onto the breadboard.
    1. Solder the spade leads to all components.
    2. Plug all components onto the breadboard.
    3. Connect the negative and positive terminals on both sides of the breadboard for ease of use.
    4. Start with the 5V Buck Converter using jumper wires.
        1. Connect VIN+ to the positive terminal.
        2. Connect VIN- to the negative terminal.
    5. Then connect the ESP32 using jumper wires.
        1. Connect GND to the negative terminal.
        2. Connect 14 to the PWM pin of the first servo.
        3. Connect 21 to the PWM pin of the second servo.
        4. Connect 5V to the V+ pin of the Buck Converter.
    6. Connect to the servos using jumper wires.
        1. Connect the V+ pin. 2.5.1 Connect the V+ pins of both servos to the negative terminal of the Buck Converter.
        2. Connect the GND pins of both servos to the negative terminal of the rail.
    7. Finally, at the positive and negative terminals, connect one jumper wire to each, leaving it hanging for connection to the battery.

## Installation
1. Install Arduino IDE on your respective operating system
2. Install the following Libraries
    - ESP HTTP Server
    - ESP Camera
    - ESP Servo
3. Configure the Settings in the Tools tab accordingly

    | Setting                                  | Value                                 |
    | ---------------------------------------- | ------------------------------------- |
    | **Board**                                | ESP32S3 Dev Module                    |
    | **Port**                                 | `/dev/ttyACM0`                        |
    | **USB CDC On Boot**                      | Enabled                               |
    | **CPU Frequency**                        | 240 MHz (WiFi)                        |
    | **Core Debug Level**                     | None                                  |
    | **USB DFU On Boot**                      | Disabled                              |
    | **Erase All Flash Before Sketch Upload** | Disabled                              |
    | **Events Run On**                        | Core 1                                |
    | **Flash Mode**                           | QIO 80 MHz                            |
    | **Flash Size**                           | 16 MB (128 Mb)                        |
    | **JTAG Adapter**                         | Disabled                              |
    | **Arduino Runs On**                      | Core 1                                |
    | **USB Firmware MSC On Boot**             | Disabled                              |
    | **Partition Scheme**                     | 16 MB Flash (3 MB APP / 9.9 MB FATFS) |
    | **PSRAM**                                | OPI PSRAM                             |
    | **Upload Mode**                          | UART0 / Hardware CDC                  |
    | **Upload Speed**                         | 921600                                |
    | **USB Mode**                             | Hardware CDC and JTAG                 |
    | **Zigbee Mode**                          | Disabled                              |

3. Clone this Repository or copy from the source code provied into Arduino IDE
4. Upload the Code and Enjoy!

## Usage
1. Start up the SMARTBIN by connecting to the battery.
2. On your smartphone you should see a wifi named "SMARTBIN SETUP AP". Connect to it.
3. Go to 192.168.4.1 and enter your home wifi or hotspot credentials (Cannot use your own hotspot as there are some ip problems we have not yet fix).
4. The page will show an ip. Click on it and then switch your phone's wifi to the one your provided.
5. Refresh that page and you should see a controls UI and a camera feed.

FYI : The controls are wierd. Once you click something. It will do that order until it recieves a new order or a stop order.

## Our Team
1. Keen Rungamornrat
2. Thanyarat Ruenteerasunti
3. Rawiarpar Manusviyangoon
4. Surapitch Arunsawatwong
5. Kittiphatsa Sripairojn
