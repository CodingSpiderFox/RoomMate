#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*     Arduini PIR Motion Sensor Tutorial
  *      
  *  by Dejan Nedelkovski, www.HowToMechatronics.com
  *  
  */
int PIR_SENSOR_PIN = 0;
int PIXEL_PIN = 4;
int PIXEL_COUNT = 1;
int DISPLAY_SDA_PIN = 0;
int DISPLAY_SCL_PINT = 0;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); /* Initialize display with address 0x3C */
  display.clearDisplay();  /* Clear display */
  display.setTextSize(1);	/* Select font size of text. Increases with size of argument. */
  display.setTextColor(WHITE);	/* Color of text*/
  display.println("1");
  pinMode(PIR_SENSOR_PIN, INPUT);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  int sensorValue = digitalRead(PIR_SENSOR_PIN);
  if (sensorValue == 1) {
    colorWipe(strip.Color(255, 0, 0), 50);
  }
  else {
    colorWipe(strip.Color(0,0,0),0);
  }
}