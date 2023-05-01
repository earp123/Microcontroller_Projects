#include <Adafruit_NeoPixel.h>

#define LED_PIN    A2
#define LED_COUNT  16
#define MAX_BRIGHTNESS 128

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;
uint16_t rainbow_hue = 128;

int cycle_state = 0;



void setup() {
  Serial.begin(115200);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(MAX_BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)  

}

void loop(void)
{
  // Wait for new data to arrive
  
  while (!Serial.available())
  {
    switch (cycle_state){
      case 0: break;
      case 1: colorPulse(10);
              break;
      case 2: custom_rainbow(rainbow_hue, 5);
              rainbow_hue+=64;
              break;
      case 3: break;
      case 4: break;
    }

  }

  char i_char = Serial.read();
  


  switch (i_char){
    case 'r':
      strip.setBrightness(MAX_BRIGHTNESS);
      colorWipe(strip.Color(255, 0, 0), 200);
      cycle_state = 0;
                   
      break;
    case 'g':
      strip.setBrightness(MAX_BRIGHTNESS);
      colorWipe(strip.Color(0, 255, 0), 200);
      cycle_state = 0;
      break;
    case 'b':
      strip.setBrightness(MAX_BRIGHTNESS);
      colorWipe(strip.Color(0, 0, 255), 200);
      cycle_state = 0; 
      break;
    case 'x':
      strip.clear();
      strip.show();
      cycle_state = 0;
      break;
    case '1':
      cycle_state = 1;             
      break;
    case '2':
      cycle_state = 2;
      break;
    case '3':
      cycle_state = 3; 
      break;
    case '4':
      cycle_state = 4; 
      break;

    default: break;

  }

  Serial.print("Cycle state: "); Serial.println(cycle_state);
      
  

}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}



void colorPulse(int wait)
{
  uint8_t brightness = strip.getBrightness();

  for (uint8_t b = brightness; b < MAX_BRIGHTNESS; b++){
    strip.setBrightness(b);
    strip.show();
    delay(wait);
  }
  for (uint8_t k = MAX_BRIGHTNESS; k > 0; k--){
    strip.setBrightness(k);
    strip.show();
    delay(wait);
  }
}

void custom_rainbow(uint16_t first_hue, int wait) {

  for (int i=0; i < LED_COUNT; i++) {
    uint16_t hue = first_hue + (i * 65536) / (3 * LED_COUNT);
    uint32_t color = strip.ColorHSV(hue, 196, MAX_BRIGHTNESS);
    color = strip.gamma32(color);
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
  
}
