// Wire Secondary Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI secondary device
// Refer to the "Wire Main Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup()
{
  Wire.begin(2);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb
  Serial.println("Example Start:");// start serial for output
  
}

void loop()
{
  Wire.beginTransmission(2);
  Wire.write(1);
  delay(100);
  Wire.endTransmission();
  delay(100);
}

// function that executes whenever data is received from main
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  //while(Wire.available()) // loop through all but the last
  //{
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  //}
  Serial.println();         // print the integer
}
