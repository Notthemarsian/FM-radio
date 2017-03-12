#include <SparkFunSi4703.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display 
#define OLED_address 0x3C //OBS!! banggood OLED has different I2C address than Adafruit OLED
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Input buttons on radio
#define Station1 5
#define Station2 6
#define Station3 7
#define Station4 8
#define volDown 9             
#define volUp 10
#define channelDown 11
#define channelUp 12

// Si4703 radio chip
#define resetPin 2
#define SDIO A4
#define SCLK A5
Si4703_Breakout radio(resetPin, SDIO, SCLK);
#define MONO true               // No point in increasing noise by receiving Stereo so set MONO in Si4703
#define SI4703Address 0x10      // Si4703 I2C Address.

// Variables definition
float volts;
int channel;
int volume;
char rdsBuffer[10];

void setup()
{
  // Set internal pull up resistors on inputs
  pinMode(Station1, INPUT_PULLUP);
  pinMode(Station2, INPUT_PULLUP); 
  pinMode(Station3, INPUT_PULLUP); 
  pinMode(Station4, INPUT_PULLUP); 
  pinMode(volDown, INPUT_PULLUP);        
  pinMode(volUp, INPUT_PULLUP);
  pinMode(channelUp, INPUT_PULLUP);
  pinMode(channelDown, INPUT_PULLUP);  
  // Initialise radio
  radio.powerOn();
  
  if (MONO) {
    // This code segment was copied from: http://www.vwlowen.co.uk/arduino/pocket-radio-3v3/pocket-radio-3v3.htm  Many thanks!!
    // radio.powerOn initializes the Si4703 with register bits set to Enable IC, Disable Mute and Disable softmute
    // by writing 0x4001 to POWERCFG register (01000000 00000001).  As we now already know the contents of the register,
    // there's no need to read it again before we force the Si4703 into MONO mode by including the MONO bit (bit 13)
    // and writing 0x6001 (01100000 00000001) to the register instead.
    Wire.beginTransmission(SI4703Address);                 // Force Si4703 tuner into MONO mode.
    Wire.write(0x60);                                      // Si4703 always receives data to POWERCFG 
    Wire.write(0x01);                                      // register (0x02) first.
    Wire.endTransmission(true);
  }
  
  channel = 882; // NRK P1
  radio.setChannel(channel);
  volume = 4;
  radio.setVolume(volume);
  // Initialise OLED display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_address);  // initialize with the I2C addr 0x3C (different from Adafruit)
  UpdateDisplay();
}

void loop()
{
  // Volume down
  if (digitalRead(volDown) == LOW) 
   {
     if (volume > 0) volume--;
     radio.setVolume(volume);
     UpdateDisplay();
     delay(100);
   }  
  // Volume up 
  if (digitalRead(volUp) == LOW) 
   {
     if (volume < 15) volume++;
     radio.setVolume(volume);
     UpdateDisplay();
     delay(100);
   } 
  // Channel down
  if (digitalRead(channelDown) == LOW) 
   {
     channel = radio.seekDown();
     UpdateDisplay();
     delay(100);
   }  
  // Channel up
  if (digitalRead(channelUp) == LOW) 
   {
     channel = radio.seekUp();
     UpdateDisplay();
     delay(100);
   }  
  // Favorite stations 1-4
  if (digitalRead(Station1) == LOW) 
   {
     channel = 882; // NRK P1
     radio.setChannel(channel);
     UpdateDisplay();
     delay(300);
   }  
  if (digitalRead(Station2) == LOW) 
   {
     channel = 891; // NRK P2
     radio.setChannel(channel);
     UpdateDisplay();
     delay(300);
   }  
  if (digitalRead(Station3) == LOW) 
   {
     channel = 1004; // NKR P3
     radio.setChannel(channel);
     UpdateDisplay();
     delay(300);
   }  
  if (digitalRead(Station4) == LOW) 
   {
     channel = 1052; // Radio Norge
     radio.setChannel(channel);
     UpdateDisplay();
     delay(300);
   }  
 }



void UpdateDisplay()
{
  // Clear the buffer
  display.clearDisplay();
  // White on black
  display.setTextColor(WHITE);
  // Volume
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("Vol:"); display.print(volume);
  // Battery
  display.setTextSize(1);
  display.setCursor(80,0);
  volts = readVcc();
  display.print(volts/1000,1);
  display.print("Volt");
  // Frequency
  display.setCursor(0,21);
  display.setTextSize(2);
  display.print((float)channel/10, 1); 
  display.print(" FM");
  // Channel information
  display.setCursor(0,42);
  display.setTextSize(2);
  if (channel == 882) display.println("NRK P1");
  if (channel == 1004) display.println("NRK P3");
  if (channel == 1052) display.println("Norge");
  // Show new buffer content on display
  display.display();
}

/*
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1230000L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
*/



// Get Battery Voltage subroutine courtesy John Owen (http://www.vwlowen.co.uk/arduino/pocket-radio-3v3/pocket-radio-3v3.htm) see also Gammon Forum section power
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1250000L / result; // Back-calculate AVcc in mV; 1125300 = 1.1*1023*1000
  return result;
}

