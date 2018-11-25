#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <version.h>
#include <SoftwareSerial.h>
#include <pcf8574_esp.h>
#include <math.h>
#include "DHT.h"

#define DHT_SENSOR_TYPE DHT22   // DHT22
const int DHTPin = 15; //D8
// Initialize DHT sensor.
DHT dht(DHTPin, DHT_SENSOR_TYPE);

SoftwareSerial swSer(D0, D3, false); //TODO is this still needed?

int counter[16];       // how many times we have seen new value
int reading[16];           // the current value read from the input pin
int current_state[16];    // the debounced input value
// the following variable is a long because the time, measured in milliseconds,
// will quickly become a bigger number than can be stored in an int.
long time_btn[16];         // the last time the output pin was sampled
int debounce_count = 10; // number of millis/samples to consider before declaring a debounced input
int pressCount[16];


TwoWire testWire;
TwoWire wire2;

int PIR_SENSOR_PIN = 2; //D4
int PIXEL_PIN = 0; //D3
int PIXEL_COUNT = 1;
int DISPLAY_SDA_PIN = 4; //D2
int DISPLAY_SCL_PIN = 5; //D1
int LDR_PIN = 0; //A0
#define PIN_PCF8574_INT 14 //D5
#define PIN_PCF8574_SDA 12 //D6
#define PIN_PCF8574_SCL 13 //D7

#define OLED_RESET_PIN D5 //just a dummy value. the OLED is not connected to this pin ... TODO: check wheter other devices can still use D5 .. also try to set a pin number here which is not present on the nodemcu module so there will be no interferance

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64

#define BRIGHT    300     //max led intensity (1-500)
#define INHALE    1250    //Inhalation time in milliseconds.
#define PULSE     INHALE*1000/BRIGHT
#define REST      1000    //Rest Between Inhalations.

#define ledPin D8


int i=0;
int breathe_delay = 15;   // delay between loops
unsigned long breathe_time = millis();


/* Object named display, of the class Adafruit_SSD1306 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
uint8_t s = 0, m = 0, h = 0;


Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Initialize a PCF8574 at I2C-address 0x20, using GPIO5, GPIO4 and testWire for the I2C-bus
PCF857x pcf8574(0x20, &testWire);
PCF857x pcf8574_2(0x27, &testWire);

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait); 
  }
}

void drawStr(uint8_t x, uint8_t y, char* str){
  display.setCursor(x, y);  /* Set x,y coordinates */
  display.println(str);
}

void writeOLED(){
  char time[30];
  display.clearDisplay();  /* Clear display */
  sprintf(time, "%02d:%02d:%02d", h, m, s);
  drawStr(50, 10, "Timer");
  drawStr(40, 30, time);
  
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  Serial.println("humidity "+ String(h));

  display.println("Humid: " + String(h));
  display.println("Temp: " + String(t));

  int sensorValue = analogRead(A0);   // read the input on analog pin 0

	float voltage = sensorValue * (5.0 / 1023.0);   // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)

	display.println("Voltage: " + String(voltage));

  display.println(VERSION);
  display.display();
}

void updateWatch() {
  s = s + 1;
  if (s == 60) {
    s = 0;
    m = m + 1;
  }
  if (m == 60) {
    m = 0;
    h = h + 1;
  }
  if (h == 13)
    h = 1;

  writeOLED();
}

void setup()   {
  
  for(int i = 0; i < sizeof(current_state) / sizeof(current_state[0]); i++) {
    current_state[i] = HIGH;
    counter[i] = 0;
    time_btn[i] = 0;
    reading[i] = 0;
    pressCount[i] = 0;
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); /* Initialize display with address 0x3C */
  display.clearDisplay();  /* Clear display */
  display.setTextSize(1);	/* Select font size of text. Increases with size of argument. */
  display.setTextColor(WHITE);	/* Color of text*/
  pinMode(PIR_SENSOR_PIN, INPUT);

  dht.begin();

  strip.begin();
  strip.show();

  Serial.begin(9600);
  Serial.print("Starting");
  wire2.pins(PIN_PCF8574_SDA, PIN_PCF8574_SCL);//SDA - D1, SCL - D2
  wire2.begin();
  testWire.begin();

  swSer.begin(9600);  

  pinMode(PIN_PCF8574_INT, INPUT);
  pcf8574.begin( 0xFF); 
  pcf8574.resetInterruptPin();
}

bool CheckKey(byte key, byte num){ //0, 1, 2, 3
  return key & (1 << num);
}

void printSerial(String str) {
  Serial.print(str);
  swSer.print(str + '\0');
  swSer.flush();
}

void printlnSerial(String str) {
  printSerial(str + "\r\n");
}

void handleInterrupt () {
   if (digitalRead(PIN_PCF8574_INT) == LOW) {
    for (int i = 0; i < sizeof(current_state) / sizeof(current_state[0]); i++) {
      if (millis() != time_btn[i])
      {
        if (i < 8) {
          reading[i] = pcf8574.read(i);
        }
        else {
          reading[i] = pcf8574_2.read(i-8);
        }
    
        if (reading[i] == current_state[i] && counter[i] > 0)
        {
          counter[i]--;
        }
        if (reading[i] != current_state[i])
        {
           counter[i]++; 
        }
        // If the Input has shown the same value for long enough let's switch it
        if (counter[i] >= debounce_count)
        {
          counter[i] = 0;
          current_state[i] = reading[i];
          
          if (pressCount[i]!=1) {
            ++pressCount[i];            
            printlnSerial("BTN_" + String(i));
          }
          else {
            --pressCount[i];
          }
        }
        time_btn[i] = millis();
      }
    } 
  }
}

// NON-Blocking LED breathing
void nonBlockingBreath()
{
  if( (breathe_time + breathe_delay) < millis() ){
    breathe_time = millis();
    float val = (exp(sin(i/2000.0*PI*10)) - 0.36787944)*108.0; 
    // this is the math function recreating the effect
    //analogWrite(ledPin, val);  // PWM
    i=i+1;
    handleInterrupt();
  }
  handleInterrupt();
}

void loop() {
  Serial.print("Entering loop");
  handleInterrupt();
  updateWatch();/* Every second increment clock and display */
  delay(1000);
  int sensorValue = digitalRead(PIR_SENSOR_PIN);
  if (sensorValue == 1) {
    colorWipe(strip.Color(255, 0, 0), 50);
  }
  else {
    colorWipe(strip.Color(0,0,0),0);
  }
}
