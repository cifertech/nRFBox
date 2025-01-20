<div align="center">

  <img src="https://user-images.githubusercontent.com/62047147/195847997-97553030-3b79-4643-9f2c-1f04bba6b989.png" alt="logo" width="100" height="auto" />
  
  <h1> nRFBOX </h1>
  <p> All-in-One Gadget for BLE and 2.4GHz Networks </p>


<!-- Badges -->
<a href="https://github.com/cifertech/nrfbox" title="Go to GitHub repo"><img src="https://img.shields.io/static/v1?label=cifertech&message=nrfbox&color=purple&logo=github" alt="cifertech - nrfbox"></a>
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/cifertech/nrfbox/total)
<a href="https://github.com/cifertech/nrfbox"><img src="https://img.shields.io/github/stars/cifertech/nrfbox?style=social" alt="stars - nrfbox"></a>
<a href="https://github.com/cifertech/nrfbox"><img src="https://img.shields.io/github/forks/cifertech/nrfbox?style=social" alt="forks - nrfbox"></a>

   
<h4>
    <a href="https://twitter.com/techcifer">TWITTER</a>
  <span> · </span>
    <a href="https://www.instagram.com/cifertech/">INSTAGRAM</a>
  <span> · </span>
    <a href="https://www.youtube.com/@techcifer">YOUTUBE</a>
  <span> · </span>
    <a href="https://cifertech.net/">WEBSITE</a>
  </h4>
</div>
<br/>

## 📖 Explore the Full Documentation

Ready to dive deeper into nRFBOX's details? Discover the full story, in-depth tutorials, and all the exciting features in our comprehensive [documentation](https://cifertech.net/nrfbox-your-all-in-one-gadget-for-ble-and-2-4ghz-networks/). Click the link and explore further!
  
<div>&nbsp;</div>

<h2>🛠 Functionality Status and Reliability</h2>

<table>
  <thead>
    <tr>
      <th>Feature</th>
      <th>Status</th>
      <th>Reliability</th>
      <th>Notes</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><strong>Scanner</strong></td>
      <td>Stable</td>
      <td>High</td>
      <td>Reliably scans the 2.4 GHz band to detect active channels and nearby devices. Occasional misses in high-interference environments.</td>
    </tr>
    <tr>
      <td><strong>Analyzer</strong></td>
      <td>Stable</td>
      <td>High</td>
      <td>Provides useful insights into detected signals, but additional updates are needed for improved accuracy and detailed analysis.</td>
    </tr>
    <tr>
      <td><strong>Jammer</strong></td>
      <td>Stable</td>
      <td>High</td>
      <td>Basic jamming works but effectiveness varies by device type and signal strength. Testing on select channels is recommended.</td>
    </tr>
    <tr>
      <td><strong>BLE Jammer</strong></td>
      <td>Stable</td>
      <td>High</td>
      <td>Disrupts BLE devices inconsistently. Further improvements are needed to ensure stability and effectiveness across BLE variants.</td>
    </tr>
    <tr>
      <td><strong>BLE Spoofer</strong></td>
      <td>Stable</td>
      <td>Low</td>
      <td>Capable of simulating basic BLE signals but has limited compatibility. Best for controlled testing scenarios.</td>
    </tr>
    <tr>
      <td><strong>Sour Apple</strong></td>
      <td>Stable</td>
      <td>Low</td>
      <td>Specialized attack method with limited reliability; effective only under specific conditions. Further tuning is required.</td>
    </tr>
  </tbody>
</table>

- When using **multiple NRF24** modules, the power demands can exceed the capabilities of the onboard power supply or regulator. Running three NRF modules simultaneously may cause instability, leading to intermittent failures or causing the nRFBox to stop functioning altogether.
- **Range Limitations**: The jammer is most effective at short range. Beyond a certain distance, the signal weakens, making it harder to consistently disrupt communication.
- **Device Variability**: Different devices react to jamming signals in varying ways. Some may be more resistant.

<div>&nbsp;</div>

<!-- About the Project -->
## :star2: About the Project
nRFBOX is a wireless toolkit designed to explore, analyze, and interact with various wireless communication protocols. It combines the ESP32 Wroom32U, NRF24 modules, an OLED display, and other components to create a multifunctional device that can act as a scanner, analyzer, jammer, BLE jammer, BLE spoofer, and perform advanced tasks such as the "Sour Apple" attack.



<div align="center"> 
  <img src="https://github.com/user-attachments/assets/7012353c-2d09-4956-9152-937cf6a7ce18" alt="screenshot" width="Auto" height="Auto" />
</div>

<div>&nbsp;</div>

<!-- Features -->
### 🎯 Features

**Software:**
- **Scanner**: Scans the 2.4GHz frequency band to detect active channels and devices.
- **Analyzer**: Analyzes the detected signals and provides detailed information about the activity.
- **Jammer**: Jams wireless communication on selected channels to test network robustness.
- **BLE Jammer**: Specifically targets Bluetooth Low Energy (BLE) devices to disrupt their communication.
- **BLE Spoofer**: Spoofs BLE devices to simulate various BLE signals for testing and research.
- **Sour Apple**: A specialized attack for testing security measures against specific wireless vulnerabilities.
- **Proto Kill Mode**: Proto Kill has evolved into a powerful tool for disrupting various protocols.

- **WiFi Scanner**
- **BLE Scanner**

**Hardware:**
- ESP32 Wroom32U
- NRF24 GTmini Modules
- OLED 0.96" Display

<div>&nbsp;</div>

<!-- nRFBOX V2 -->
## :eyes: nRFBox Versions: Then and Now

<table>
  <tr>
    <td style="text-align: center;">
      <img src="https://github.com/user-attachments/assets/b121fe84-c789-409a-85f5-21f6d5854347" alt="nRFBOX V1" style="width: 600px; border: 1px solid #ccc; border-radius: 5px;">
      <p style="font-style: italic; font-size: 14px; margin-top: 5px;">nRFBox-v1 based on Arduino</p>
    </td>    
    <td style="text-align: center;">
      <img src="https://github.com/user-attachments/assets/a044ab5e-346c-415f-b1fb-f65aa04c520a" alt="nRFBOX V2" style="width: 600px; border: 1px solid #ccc; border-radius: 5px;">
      <p style="font-style: italic; font-size: 14px; margin-top: 5px;">nRFBox-v2 based on ESP32</p>
    </td>
  </tr>
</table>


<div>&nbsp;</div>

## 🔗 Uploading the Firmware (.bin) to ESP32

If you prefer not to upload the code manually or encounter errors during compilation, you can directly upload the precompiled `.bin` file to the ESP32. Follow these steps:

### Step 1: Download the Required Tools
1. **ESP32 Flash Download Tool**: Download the tool from Espressif's official site: [ESP32 Flash Download Tool](https://www.espressif.com/sites/default/files/tools/flash_download_tool_3.9.7_2.zip).
2. **USB Drivers**: Make sure the drivers for the ESP32 are installed on your system. You can download the drivers from [CP210x USB to UART Bridge VCP Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) if you're using the CP2102 chip.

### Step 2: Prepare the `.bin` File
1. Download the [precompiled `.bin`](https://github.com/cifertech/nRFBox/tree/main/precompiled%20%20bin) file from the repository. 

### Step 3: Upload the `.bin` File to ESP32
1. Connect your ESP32 to your computer using a USB cable.
2. Open the **ESP32 Flash Download Tool** and configure the following:
   - Select the appropriate **chip type** (ESP32) and click **OK**.
   - In the **Download Path Config** section, add the `.bin` file and set the start address to `0x10000`.
   - Select the correct **COM port** where your ESP32 is connected.
   - Choose the correct **Baud rate** (115200 is generally recommended).

3. Click on **Start** to begin uploading the `.bin` file to your ESP32.

### Step 4: Verify the Upload
Once the upload is completed, the tool will confirm a successful flash. You can now restart your ESP32 and run the firmware.

&nbsp;
### Arduino IDE 1.8 Settings:
If you want to upload the sketch code to the ESP32, use the following settings in the Tools menu:
- **Built using ESP32 Board Version 2.0.11**

<div style="display: flex; justify-content: center; align-items: center; gap: 10px;">
  <img src="https://github.com/user-attachments/assets/2e43dc05-9ef7-4743-85f2-a8fc38e7b785" alt="Photo 1" style="width: 34%; border: 1px solid #ccc; border-radius: 5px;">
  <img src="https://github.com/user-attachments/assets/9fc87b96-7442-44f8-9457-22d1b481eda1" alt="Photo 2" style="width: 53%; border: 1px solid #ccc; border-radius: 5px;">
</div>


<div>&nbsp;</div>

<!-- License -->
## :warning: License

Distributed under the MIT License. See LICENSE.txt for more information.

<div>&nbsp;</div>

<!-- Contact -->
## :handshake: Contact

▶ Support me on Patreon [patreon.com/cifertech](https://www.patreon.com/cifertech)

CiferTech - [@twitter](https://twitter.com/techcifer) - CiferTech@gmali.com

Project Link: [https://github.com/cifertech/nRFBOX](https://github.com/cifertech/nrfbox)

<div>&nbsp;</div>

<!-- Acknowledgments -->
## :gem: Acknowledgements 

**The libraries and projects listed below are used in this software:**
 - [Poor Man’s 2.4 GHz Scanner](https://forum.arduino.cc/t/poor-mans-2-4-ghz-scanner/54846)
 - [arduino_oled_menu](https://github.com/upiir/arduino_oled_menu)
 - [nRF24L01-WiFi-Jammer](https://github.com/hugorezende/nRF24L01-WiFi-Jammer)
 - [Universal-RC-system](https://github.com/alexbeliaev/Universal-RC-system/tree/master)
 - [AppleJuice](https://github.com/ECTO-1A/AppleJuice)
 - [ESP32-Sour-Apple](https://github.com/RapierXbox/ESP32-Sour-Apple)

**Community Contributors**: Thanks to everyone who helped improve nRFBox! Your support is much appreciated!

