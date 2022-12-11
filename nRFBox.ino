#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#include <RF24.h>
#include <nRF24L01.h>
#include "printf.h"

RF24 radio(7, 8);


const uint8_t num_channels = 64;
int           channels     = 0;
const byte    address[6]   = "00001";
const int     num_reps     = 50;
bool          jamming      = true;
int           ch           = 0;
int           values[num_channels];
int           valuesDisplay[32];

int selected =  0;
int entered  = -1;

#define CE  9

#define CHANNELS  64
int channel[CHANNELS];

int  line;
char grey[] = " .:-=+*aRW";

byte count;
byte sensorArray[128];
byte drawHeight;

char filled = 'F'; 
char drawDirection = 'R'; 
char slope = 'W'; 

#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09


void setup() {

  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
 
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);

  display.clearDisplay();
  
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 5);
  display.println("nRf-BOX");
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.println("by CiferTech");
  
  display.display();
  delay(3000);

  radio.begin();
  radio.startListening();
  radio.stopListening();

   for (count = 0; count <= 128; count++) 
  {
    sensorArray[count] = 0;
  }
 
  // Setup SPI
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
 
  // Activate Chip Enable
  pinMode(CE,OUTPUT);
  disable();
 
  powerUp();
 
  setRegister(_NRF24_EN_AA,0x0);

  setRegister(_NRF24_RF_SETUP,0x0F);
 
  line = 0;
  
}

void loop() {

 displaymenu();

  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_2MBPS);

}


void displaymenu(void) {

  int down =  digitalRead(2);
  int up =    digitalRead(3);
  int enter = digitalRead(4);
  int back  = digitalRead(6);
  

  if (up == LOW) {
    selected = selected + 1;
    delay(200);
  };
  if (down == LOW) {
    selected = selected - 1;
    delay(200);
  };
  if (enter == LOW) {
    entered = selected;
  };
    if (back == LOW) {
    entered = -1;
  };
 
  const char *options[4] = {
    " Rf Scanner ",
    " 2.4 jammer ",
    " Analyzer "
  };

  if (entered == -1) {
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("nRf-BOX");
    
    
    for (int i = 0; i < 4; i++) {
      if (i == selected) {
        //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.print(">");
        display.println(options[i]);
      } else if (i != selected) {
        display.setTextColor(SSD1306_WHITE);
        display.println(options[i]);
      }
    }
    
  } else if (entered == 0) {

   if (up == LOW) entered = -1;  
     scanChannels();
     outputChannels();


  } else if (entered == 1) {
    jammer();

   
  } else if (entered == 2) {
    renderGraph();    
  } 

  display.display();
}


byte getRegister(byte r)
{
 byte c;
 
 PORTB &=~_BV(2);
 c = SPI.transfer(r&0x1F);
 c = SPI.transfer(0);  
 PORTB |= _BV(2);

 return(c);
}

void setRegister(byte r, byte v)
{
 PORTB &=~_BV(2);
 SPI.transfer((r&0x1F)|0x20);
 SPI.transfer(v);
 PORTB |= _BV(2);
}
 
void powerUp(void)
{
 setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)|0x02);
 //delayMicroseconds(130);
}

void powerDown(void)
{
 setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)&~0x02);
}

void enable(void)
{
   PORTB |= _BV(1);
}

void disable(void)
{
   PORTB &=~_BV(1);
}

void setRX(void)
{
 setRegister(_NRF24_CONFIG,getRegister(_NRF24_CONFIG)|0x01);
 enable();
 // this is slightly shorter than
 // the recommended delay of 130 usec
 // - but it works for me and speeds things up a little...
 delayMicroseconds(100);
}

void scanChannels(void)
{
 disable();
 for( int j=0 ; j<200  ; j++)
 {
   for( int i=0 ; i<CHANNELS ; i++)
   {
     setRegister(_NRF24_RF_CH,(128*i)/CHANNELS);
     
     setRX();
     
     delayMicroseconds(40);
     
     disable();

     if( getRegister(_NRF24_RPD)>0 )   channel[i]++;
   }
 }
}

void outputChannels(void)
{
  display.clearDisplay(); 
  
  int norm = 0;
 
 for( int i=0 ; i<CHANNELS ; i++)
   if( channel[i]>norm ) norm = channel[i];
   
 for( int i=0 ; i<CHANNELS ; i++)
 {
   int pos;
   
   if( norm!=0 ) pos = (channel[i]*10)/norm;
   else          pos = 0;
   
   if( pos==0 && channel[i]>0 ) pos++;
   
   if( pos>9 ) pos = 9;
 
   channel[i] = 0;
 }

  display.setCursor(90, 10);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(norm);
  display.setCursor(90, 8);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("");


  display.drawLine(0, 0, 0, 32, WHITE);
  display.drawLine(80, 0, 80, 32, WHITE);

  for (count = 0; count < 40; count += 10)
  {
    display.drawLine(80, count, 75, count, WHITE);
    display.drawLine(0, count, 5, count, WHITE);
  }

  for (count = 10; count < 80; count += 10)
  {
    display.drawPixel(count, 0 , WHITE);
    display.drawPixel(count, 31 , WHITE);
  }

  
  drawHeight = map(norm, 0, 70, 0, 50);
  sensorArray[0] = drawHeight;

  for (count = 1; count <= 80; count++ )
  {
    if (filled == 'D' || filled == 'd')
    {
      if (drawDirection == 'L' || drawDirection == 'l')
      {
        display.drawPixel(count, 32 - sensorArray[count - 1], WHITE);
      }
      else //else, draw dots from right to left
      {
        display.drawPixel(80 - count, 32 - sensorArray[count - 1], WHITE);
      }
    }

    else
    {
      if (drawDirection == 'L' || drawDirection == 'l')
      {
        if (slope == 'W' || slope == 'w')
        {
          display.drawLine(count, 32, count, 32 - sensorArray[count - 1], WHITE);
        }
        else
        {
          display.drawLine(count, 1, count, 32 - sensorArray[count - 1], WHITE);

        }
      }

      else
      {
        if (slope == 'W' || slope == 'w')
        {
          display.drawLine(80 - count, 32, 80 - count, 32 - sensorArray[count - 1], WHITE);
        }
        else
        {
          display.drawLine(80 - count, 1, 80 - count, 32 - sensorArray[count - 1], WHITE);
        }
      }
    }
  }

  display.display(); 

  for (count = 80; count >= 2; count--) 
  {
    sensorArray[count - 1] = sensorArray[count - 2];
  }
}

  
void jammer() {

  pinMode(4, INPUT_PULLUP);

  int increase = digitalRead(4);

  display.clearDisplay(); 
  
  if (increase == LOW) {    
     ch++;  
    
   if (ch < 13) ch=ch;
    else ch = 1;
              
    delay(1);     
  }

     display.clearDisplay();
     display.setTextSize(2);
     display.setTextColor(SSD1306_WHITE);
     display.setCursor(30, 0);
     display.println(ch); 
     delay(50);
     
  
  display.display(); 


  const char text[] = "xxxxxxxxxxxxxxxx"; 
  
  for (int i = ((ch * 5) + 1); i < ((ch * 5) + 23); i++) {
    radio.setChannel(i);
    radio.write( & text, sizeof(text));
  }
}


void renderGraph() {
  
  pinMode(4, INPUT_PULLUP);
  int increase = digitalRead(4);

  
  if (increase == LOW) {    
     channels++;  
    
   if (channels < 13) channels=channels;
    else channels = 1;
              
    delay(1);     
  }
      
  memset(values, 0, sizeof(values));

  int rep_counter = num_reps;
  while (rep_counter--) {
    int i = num_channels;
    while (i--) {

      radio.setChannel(channels);

      radio.startListening();
      delayMicroseconds(50);
      radio.stopListening();

      if (radio.testCarrier())
        ++values[i];
    }
  }

  display.clearDisplay();
  display.drawLine(0, 50, 127, 50, WHITE);

  for (int i = 0; i < 64; i++) {
  display.fillRect((1 + (i * 2)), (60 - values[i]), 1, values[i], WHITE);
  
  display.setTextSize(1); 
  display.setTextColor(WHITE); 
  display.setCursor(0, 0); 
  display.print("channel: " + String(channels + 1));

  display.setCursor(80, 0); 
  display.print(60 - values[i]);
  }
  display.display();
}
