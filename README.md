<div align="center">

  <img src="https://user-images.githubusercontent.com/62047147/195847997-97553030-3b79-4643-9f2c-1f04bba6b989.png" alt="logo" width="100" height="auto" />
  <h1>nRFBOX</h1>
  
  <p>
    2.4Ghz ULTIMATE Tool
  </p>
  
  
<!-- Badges -->

<a href="https://github.com/cifertech/nrfbox" title="Go to GitHub repo"><img src="https://img.shields.io/static/v1?label=cifertech&message=nrfbox&color=purple&logo=github" alt="cifertech - nrfbox"></a>
<a href="https://github.com/cifertech/nrfbox"><img src="https://img.shields.io/github/stars/cifertech/nrfbox?style=social" alt="stars - nrfbox"></a>
<a href="https://github.com/cifertech/nrfbox"><img src="https://img.shields.io/github/forks/cifertech/nrfbox?style=social" alt="forks - nrfbox"></a>
   
<h4>
    <a href="https://twitter.com/cifertech1">TWITTER</a>
  <span> · </span>
    <a href="https://www.instagram.com/cifertech/">INSTAGRAM</a>
  <span> · </span>
    <a href="https://www.youtube.com/c/cifertech">YOUTUBE</a>
  <span> · </span>
    <a href="https://cifertech.net/">WEBSITE</a>
  </h4>
</div>

<br />



nRF-BOX Release Note - Version 2.1.1

Release Date: [8/15/2023]

## New Features

- 128-channel analyzer
- Show status with NeoPixel ws2812

## Enhancements

- 2.4 Jammer improved with better control for channel selection and starting the attack
- 2.4 jammer is more effective (this gives you the option to lower the traffic in a specific channel not entirely cut the channel off)
- More suitable graph for channel analyzer

## Bug Fixes

- Menu Selection bug fixed
- RF scanner bug fixed
- 2.4 jammer bug fixed

===========================================================================




  

<!-- About the Project -->
## :star2: About the Project
In this project, I build a 2.4GHz scanner/jammer/Channel Analyzer using the nRF24L01.


<!-- Pictures -->
### :camera: Pictures

<div align="center"> 
  <img src="https://user-images.githubusercontent.com/62047147/206877956-d8b08ef5-fdc4-4f3d-a5c2-49f01483b8cb.jpg" alt="screenshot" />
</div>


<!-- Features -->
### :dart: Features

- Scan 2.4Ghz band
- 2.4Ghz jammer
- Channel Analyzer

<!-- Getting Started -->
## 	:toolbox: Getting Started

We will use Arduino Pro Mini as a processor. Also, an OLED display to show the Menu and desired options. With the nRF24 module, we can execute the features. 

- Arduino Pro Mini
- nRF24
- Oled 0.96 SSD1306

<!-- Schematic -->
### :electric_plug: Schematic
Make the connections according to the table and schematic below.

* Arduino and nRF24.

| Arduino| nRF24|  
| ----   | -----|
| 9  | CE   |
| 13 | SCK  |
| 12 | MISO |
| 10 | CSN  |
| 11 | MOSI |
| 3V3 | Vcc |
| GND | GND |


* Arduino and OLED display.

| Arduino| Oled 0.96|
| ----   | -----|
| A5  | SCK |
| A4 | SDA  |
| Vin | VDD |
| GND | GND |

 
* Complete Schematic

<img src="https://user-images.githubusercontent.com/62047147/206878457-5e729716-5ee7-4f6b-97f5-b45559d7cc2a.png" alt="screenshot" width="800" height="auto" />


<!-- Installation -->
### :gear: Installation

Before uploading the code you need to install the required library in Arduino IDE. Follow these steps:

- Follow this path Sketch> Include Library> Manage Libraries
- Search for Adafruit SSD1306
- Install the library

-Then search for the “GFX” and install it also.

-Also you need "NRF24" library.
   
<!-- Usage -->
## :eyes: Usage

After uploading the code, the Menu will show up and you able to choose the option you want.

<img src="https://user-images.githubusercontent.com/62047147/206902220-c793003a-2a08-4eb2-8154-6182c203cf49.jpg" alt="screenshot" width="300" height="auto" />


<!-- Contributing -->
## :wave: Contributing

<a href="https://github.com/cifertech/nrfbox/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=cifertech/nrfbox" />
</a>


<!-- License -->
## :warning: License

Distributed under the MIT License. See LICENSE.txt for more information.


<!-- Contact -->
## :handshake: Contact

CiferTech - [@twitter](https://twitter.com/cifertech1) - CiferTech@gmali.com

Project Link: [https://github.com/cifertech/nRFBox](https://github.com/cifertech/nRFBox)

<!-- Acknowledgments -->
## :gem: Acknowledgements 

 - [Poor Man’s 2.4 GHz Scanner](https://forum.arduino.cc/t/poor-mans-2-4-ghz-scanner/54846)
 - [nRF24L01-WiFi-Jammer](https://github.com/hugorezende/nRF24L01-WiFi-Jammer)
 - [Universal-RC-system](https://github.com/alexbeliaev/Universal-RC-system/tree/master)

