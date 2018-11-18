#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/*     Arduini PIR Motion Sensor Tutorial
  *      
  *  by Dejan Nedelkovski, www.HowToMechatronics.com
  *  
  */
int pirSensor = 0;
int PIXEL_PIN = 4;
int PIXEL_COUNT = 1;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void setup() {
  pinMode(pirSensor, INPUT);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  int sensorValue = digitalRead(pirSensor);
  if (sensorValue == 1) {
    colorWipe(strip.Color(255, 0, 0), 50);
  }
  else {
    colorWipe(strip.Color(0,0,0),0);
  }
}