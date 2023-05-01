/* TFT Display GUI meant for use with this product 
*  https://www.amazon.com/gp/product/B073R7BH1B/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
*
*  HiLetgo ILI9341 2.8" SPI TFT LCD Display Touch Panel 240X320 with PCB 5V/3.3V STM32
*
*  By Sam Rall ~SWR~
*/
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>    // Core graphics library

#include <XPT2046_Touchscreen.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

/* Currently using Sparkfun ESP32-S2 Thing Plus
*  These pints are all on the same side of the board
*  It may require the RST pins on both the TFT and ESP32 board to be connected
*/
#define IRQ_PIN A2
#define CS_PIN  3
#define TFT_CS 34
#define TFT_DC 33
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046_Touchscreen ts(CS_PIN, IRQ_PIN);

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3


#define BOX_A0 BOXSIZE
#define BOX_A1 BOXSIZE*2
#define BOX_A2 BOXSIZE*3
#define BOX_A3 BOXSIZE*4
#define BOX_A4 BOXSIZE*5

#define MAX_OPTIONS 5

//easy index for the colors - must be in specific order
uint16_t color_idx[MAX_OPTIONS] = {ILI9341_RED, ILI9341_YELLOW, ILI9341_GREEN, 
                                                ILI9341_BLUE, ILI9341_CYAN};

int oldcolor, currentcolor;

//Main Menu Option Strings
char str_mm1[] = "Red Menu";
char str_mm2[] = "Yellow Menu";
char str_mm3[] = "Green Menu";
char str_mm4[] = "Blue Menu";
char str_mm5[] = "Purple Menu";

//Main Menu Options pointer
char * MMopts[]= {str_mm1, str_mm2, str_mm3, str_mm4, str_mm5};

void setup(void) {
 // while (!Serial);     // used for leonardo debugging
 
  Serial.begin(9600);

  tft.begin();
  tft.setRotation(2);

  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  ts.setRotation(0);
  Serial.println("Touchscreen started");
  
  loadMainMenu();

}


void loop()
{
  /* See if there's any  touch data for us
  if (ts.bufferEmpty()) {
    return;
  }*/
  
  // You can also wait for a touch
  if (! ts.touched()) {
    return;
  }
  

  // Retrieve a point  
  TS_Point p = ts.getPoint();
 
  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  //Select the option
  if (p.x < BOX_A0) {
     oldcolor = currentcolor;
     if (p.y < BOXSIZE){
       //Do nothing
     }
     else if (p.y < BOX_A1) { 
       currentcolor = color_idx[0];
       tft.drawRect(0, BOX_A0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       print_MainLine();

      } 
      else if (p.y < BOX_A2) {
       currentcolor = color_idx[1];
       tft.drawRect(0, BOX_A1, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       print_MainLine();
     } 
     else if (p.y < BOX_A3) {
       currentcolor = color_idx[2];
       tft.drawRect(0, BOX_A2, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       print_MainLine();
     } 
     else if (p.y < BOX_A4) {
       currentcolor = color_idx[3];
       tft.drawRect(0, BOX_A3, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       print_MainLine();
     } 
     else if (p.y < BOXSIZE*6) //Box "A5"
     {
       currentcolor = color_idx[4];
       tft.drawRect(0, BOX_A4, BOXSIZE, BOXSIZE, ILI9341_WHITE);
       print_MainLine();   
     } 

    // Deselect the old option
     if (oldcolor != currentcolor) {
       switch (oldcolor){
         case ILI9341_RED: tft.fillRect(0, BOXSIZE, BOXSIZE, BOXSIZE, color_idx[0]); break;
         case ILI9341_YELLOW: tft.fillRect(0, BOXSIZE*2, BOXSIZE, BOXSIZE, color_idx[1]); break;
         case ILI9341_GREEN: tft.fillRect(0, BOXSIZE*3, BOXSIZE, BOXSIZE, color_idx[2]); break;
         case ILI9341_BLUE: tft.fillRect(0, BOXSIZE*4, BOXSIZE, BOXSIZE, color_idx[3]); break;
         case ILI9341_CYAN: tft.fillRect(0, BOXSIZE*5, BOXSIZE, BOXSIZE, color_idx[4]); break;
         default: break;//do nothing
       }
        
        
     }
  }
  
}

void print_MainLine(){
  tft.setCursor(0, 0);
  tft.setTextColor(currentcolor);
  tft.println("Hello World!");
}

void loadMainMenu(){

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);

  for (int i = 1; i <= MAX_OPTIONS; i++){
    tft.fillRect(0, BOXSIZE*i, BOXSIZE, BOXSIZE, color_idx[i-1]);
    tft.setCursor(50, BOXSIZE*i+10);
    tft.setTextColor(color_idx[i-1]);  
    tft.print(MMopts[i-1]);

  }
}