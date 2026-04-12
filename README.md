# Breadboard Weather Station - LoRa + GPS + DHT11
This is the companion repository for my breadboard weather station project. It includes the Arduino code, wiring schematics, and documentation for the transmitter and receiver.

I've recently posted a tutorial about this project on YouTube. You can watch it [here](https://www.youtube.com/).

![The project over a table.](https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-1.jpg)

## 🚀OVERVIEW
This project started as a way to test wireless communication for a weather station concept. The goal was simple: build a functional prototype using two Arduino boards, LoRa modules, a DHT11 sensor, and a GPS receiver.

One Arduino acts as the **transmitter** (collects weather data + GPS coordinates). The other acts as the **receiver** (displays the data on an LCD screen).

Communication happens via LoRa (Long Range), a low-power wireless protocol common in IoT applications. If you're new to LoRa, I recommend you watch [this quick introduction].(https://www.youtube.com/watch?v=w7p6iTCCfqA&t=5s).

<div style="display: flex; flex-wrap: wrap;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-2.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-3.jpg" width="400" height="250" style="margin: 10px;">
</div>

## 🧩WHAT THIS PROJECT DOES
+ Measures **temperature** and **humidity** (DHT11 sensor)
+ Gets **GPS coordinates** (NEO-6M module)
+ Sends data wirelessly from transmitter to receiver via **LoRa**
+ Displays data on a **16x2 LCD screen**
+ Allows toggling between **Celsius** and **Fahrenheit** with a button

## 🧰LIST OF MATERIALS

| Item            | Quantity |
|--------------------------|-------------|
| Arduino UNO                                   | 1 |
| Arduino Nano                                  | 1 |
| RYLR993 Lite                                  | 2 |
| DHT11 sensor                                  | 1 |
| NEO-6M GPS                                    | 1 |
| I2C LCD                                       | 1 |
| NEO-6M GPS                                    | 1 |
| I/O expansion shield                          | 1 |
| Push button                                   | 1 |
| 5kΩ resistor                                  | 2 |
| 10kΩ resistor                                 | 2 |
| Battery                                       | 2 |
| Breadboard                                    | 2 |
| Jumper wires                                  | Several |

## ⚙️WIRING
Follow the schematics below to assemble all components.

### Transmitter
![Project's diagram - transmitter](https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-4.jpg)

### Receiver
![Project's diagram - receiver](https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-5.jpg)

In the end, you'll have something like this:

<div style="display: flex; flex-wrap: wrap;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-6.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-7.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-8.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-9.jpg" width="400" height="250" style="margin: 10px;">
</div>

## 💡TESTING AND RESULTS
### Indoor Test (GPS failure)

First test was inside my house. The weather station worked fine - temperature and humidity readings came through. But the GPS module showed no signal.

I rebooted. Moved to different rooms. Still nothing.

### Why did the GPS fail indoors?

The NEO-6M is a basic GPS receiver. It needs a clear view of the sky to lock onto satellite signals. Roofs, walls, and even thick windows block the weak signals from space. Unlike your phone (which uses assisted GPS + cell towers), this module has no help, just direct line-of-sight to satellites.

Lesson learned: Don't expect cheap GPS modules to work indoors. Save yourself the troubleshooting and go outside from the start.

### Outdoor Test (success)

I took the transmitter to a nearby square, powered it on, and waited. About 5 minutes later - the GPS locked onto satellites.

Back at the receiver, I could finally see:

+ Temperature
+ Humidity
+ GPS coordinates

I checked the coordinates on Google Maps. They matched perfectly. If I ever lose the transmitter outdoors, I'll be able to find it.

The button toggled between Celsius and Fahrenheit without issues.

<div style="display: flex; flex-wrap: wrap;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-10.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-11.jpg" width="400" height="250" style="margin: 10px;">
    <img src="https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-12.jpg" width="400" height="250" style="margin: 10px;">
</div>

## NEXT STEPS

+ Perform a city-based range test (obstacles, buildings, traffic)
+ Design an enclosure for outdoor use
+ Improve GPS lock time (consider a better module or add a backup battery)

## ❤️SPONSOR
I want to give a huge thank you to [Reyax](https://reyax.com/), who kindly provided the [RYLR993 Lite modules](https://reyax.com/products/RYLR993_Lite) for this tutorial.

You can find their products on [Amazon](https://bit.ly/4kfPzGm), [DigiKey](https://www.digikey.com/en/products/detail/reyax/RYLR993-Lite/22237834), and other (global distributors)[https://reyax.com//distributors.php].

![Reyax's website.](https://github.com/lucasfernandoprojects/breadboard-weather-station/blob/main/photos/photo-13.jpg)
