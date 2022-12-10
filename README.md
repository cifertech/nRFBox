<div align="center">

  <img src="https://user-images.githubusercontent.com/62047147/195847997-97553030-3b79-4643-9f2c-1f04bba6b989.png" alt="logo" width="100" height="auto" />
  <h1>nRFBOX</h1>
  
  <p>
    2.4Ghz Tool
  </p>
  
  
<!-- Badges -->

<a href="https://github.com/cifertech/2.4-GHz-band-Scanner" title="Go to GitHub repo"><img src="https://img.shields.io/static/v1?label=cifertech&message=2.4-GHz-band-Scanner&color=white&logo=github" alt="cifertech - 2.4-GHz-band-Scanner"></a>
<a href="https://github.com/cifertech/2.4-GHz-band-Scanner"><img src="https://img.shields.io/github/stars/cifertech/2.4-GHz-band-Scanner?style=social" alt="stars - 2.4-GHz-band-Scanner"></a>
<a href="https://github.com/cifertech/2.4-GHz-band-Scanner"><img src="https://img.shields.io/github/forks/cifertech/2.4-GHz-band-Scanner?style=social" alt="forks - 2.4-GHz-band-Scanner"></a>
   
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

<!-- Table of Contents -->
# :notebook_with_decorative_cover: Table of Contents

- [About the Project](#star2-about-the-project)
  * [Pictures](#camera-Pictures)
  * [Features](#dart-features)
- [Getting Started](#toolbox-getting-started)
  * [Schematic](#electric_plug-Schematic)
  * [Installation](#gear-installation)
- [Usage](#eyes-usage)
- [Contributing](#wave-contributing)
- [License](#warning-license)
- [Contact](#handshake-contact)

  

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
   
<!-- Usage -->
## :eyes: Usage

After uploading the code, the network scanner starts working automatically and Show the Scanned Data in Graph.


<!-- Contributing -->
## :wave: Contributing

<a href="https://github.com/cifertech/2.4-GHz-band-Scanner/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=cifertech/2.4-GHz-band-Scanner" />
</a>


<!-- License -->
## :warning: License

Distributed under the MIT License. See LICENSE.txt for more information.


<!-- Contact -->
## :handshake: Contact

CiferTech - [@twitter](https://twitter.com/cifertech1) - CiferTech@gmali.com

Project Link: [https://github.com/cifertech/nRFBox](https://github.com/cifertech/nRFBox)


