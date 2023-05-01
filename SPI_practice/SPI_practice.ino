#include <SPI.h>

byte  c0 = 0xfe;
char c1 = 0;
void setup (void) {

 SPI.begin ();
 SPI.setClockDivider(SPI_CLOCK_DIV16);
 
}

void loop (void) {

 delayMicroseconds(1);
 SPI.transfer (c1);
 
 //SPI.transfer (c1);

 
}
