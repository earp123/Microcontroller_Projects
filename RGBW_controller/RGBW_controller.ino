/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <bluefruit.h>
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN     13

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  60

#define RAINBOW_CHASE_SPEED 100
#define RAINBOW_FADE_SPEED  40
#define PULSE_WHITE_SPEED   25


// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

   static uint8_t red=0;
   static uint8_t green=0;
   static uint8_t blue=0;
   static uint8_t white = 0;
   static uint8_t brightness = 128;
   

// OTA DFU service
BLEDfu bledfu;

// Uart over BLE service
BLEUart bleuart;

// Function prototypes for packetparser.cpp
uint8_t readPacket (BLEUart *ble_uart, uint16_t timeout);
float   parsefloat (uint8_t *buffer);
void    printHex   (const uint8_t * data, const uint32_t numBytes);

// Packet buffer
extern uint8_t packetbuffer[];

void setup(void)
{
  Serial.begin(115200);
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(brightness);

  Serial.println(F("Adafruit Bluefruit52 Controller App Example"));
  Serial.println(F("-------------------------------------------"));

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("4611 LIT AF");

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and start the BLE Uart service
  bleuart.begin();

  // Set up and start advertising
  startAdv();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();


}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include the BLE UART (AKA 'NUS') 128-bit UUID
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
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

void fadeToColor(uint32_t color, int wait){
  uint32_t setColor = strip.Color(red, green, blue, white);
  uint32_t newColor = color;
  while (setColor != newColor){
    if (setColor < newColor) setColor++;
    else if (setColor > newColor) setColor--;
  
    strip.fill(setColor);
    strip.show();
    delay(wait);
  }
  
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//Add a floor value instead of zero to have better looking fades
int pulseWhite(uint8_t wait, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t len;
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(r, g, b, strip.gamma8(j)));
    strip.show();
    delay(wait);
    len = readPacket(&bleuart, 1);
    if (len) return 0;
  }

  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    strip.fill(strip.Color(r, g, b, strip.gamma8(j)));
    strip.show();
    delay(wait);
    len = readPacket(&bleuart, 1);
    if (len) return 0;
  }

  return 1;
}

int rainbowFade(int wait, int rainbowLoops) {
  int fadeVal=0, fadeMax=100;
  uint8_t len;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for(uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops*65536;
    firstPixelHue += 256) {

    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255,
        255 * fadeVal / fadeMax)));
    }

    strip.show();
    delay(wait);
    uint8_t len = readPacket(&bleuart, 1);
    if (len) return 0;

    if(firstPixelHue < 65536) {                              // First loop,
      if(fadeVal < fadeMax) fadeVal++;                       // fade in
    } else {
      fadeVal = fadeMax; // Interim loop, make sure fade is at max
    }
  }

// Took out the white loops

  delay(500); // Pause 1/2 second
  return 1;
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  // Wait for new data to arrive
  uint8_t len = readPacket(&bleuart, 1);
  if (len == 0) return;

  // Got a packet!
  // printHex(packetbuffer, len);

  // Color
  if (packetbuffer[1] == 'C') {
    red   = (strip.getPixelColor(10) & 0x00FF0000);
    green = (strip.getPixelColor(10) & 0x0000FF00);
    blue  = (strip.getPixelColor(10) & 0x000000FF);
    int newRed = packetbuffer[2];
    int newGreen = packetbuffer[3];
    int newBlue = packetbuffer[4];
    Serial.print ("RGB #");
    if (red < 0x10) Serial.print("0");
    Serial.print(red, HEX);
    if (green < 0x10) Serial.print("0");
    Serial.print(green, HEX);
    if (blue < 0x10) Serial.print("0");
    Serial.println(blue, HEX);
    
    fadeToColor(strip.Color(newRed, newGreen, newBlue, white), 10);
    
  }

  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';

    Serial.print("Button "); Serial.print(buttnum);
    Serial.print("Pressed "); Serial.println(pressed);
    red   = (strip.getPixelColor(10) & 0x00FF0000);
    green = (strip.getPixelColor(10) & 0x0000FF00);
    blue  = (strip.getPixelColor(10) & 0x000000FF);
    brightness = strip.getBrightness();
    uint32_t lastColor;


      switch(buttnum){

        case 1: break;//Animation 1
             
        case 2: if (!pressed) while (rainbowFade(RAINBOW_FADE_SPEED, 100)); 
                break;//Animation 2

        case 3: break;//Animation 3
        
        case 4: brightness = strip.getBrightness();
                while (brightness > 1){
                  brightness--;
                  strip.setBrightness(brightness);
                  strip.show();
                  delay(20);
                }
                strip.clear();
                strip.show();

                break;//Fade off

        case 5: while (pressed)
                {
                  if (white > 0)
                  {
                    white--;
                    strip.fill(strip.Color(red, green, blue, white));
                    strip.show();
                  }
                  delay(25);
                  Serial.println(white);
                  //else blink something;
                  len = readPacket(&bleuart, 1);
                  pressed = packetbuffer[3] - '0';
                }
                break;
               
        case 6: while (pressed)
                {
                  if (white < 255)
                  {
                    white++;
                    strip.fill(strip.Color(red, green, blue, white));
                    strip.show();
                    delay(25);
                  }
                  Serial.println(white);
                  //else blink something;
                  len = readPacket(&bleuart, 1);
                  pressed = packetbuffer[3] - '0';
                }
                break;

        case 7: while (pressed)
                {
                  if (brightness > 2)
                  {
                    brightness-= 1;
                    strip.setBrightness(brightness);
                    strip.show();
                    delay(25);
                  }
                  //else blink something;
                  len = readPacket(&bleuart, 1);
                  pressed = packetbuffer[3] - '0';
                }
                break;

        case 8: while (pressed)
                {
                  if (brightness < 255)
                  {
                    brightness+= 1;
                    strip.setBrightness(brightness);
                    strip.show();
                    delay(25);
                  }
                  len = readPacket(&bleuart, 1);
                  pressed = packetbuffer[3] - '0';
                }
                break;

        

        default: red = 255; green = 0; blue = 0; break;
      }
      
      
    }
    
  }
