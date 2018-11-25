#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <version.h>

int PIR_SENSOR_PIN = 0;
int PIXEL_PIN = 4;
int PIXEL_COUNT = 1;
int DISPLAY_SDA_PIN = 0;
int DISPLAY_SCL_PIN = 0;
#define OLED_RESET_PIN D5
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64

/* Object named display, of the class Adafruit_SSD1306 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
uint8_t s = 0, m = 0, h = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
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
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); /* Initialize display with address 0x3C */
  display.clearDisplay();  /* Clear display */
  display.setTextSize(1);	/* Select font size of text. Increases with size of argument. */
  display.setTextColor(WHITE);	/* Color of text*/
  pinMode(PIR_SENSOR_PIN, INPUT);

  strip.begin();
  strip.show();
}

void loop() {
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
