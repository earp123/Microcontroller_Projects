// Wire Secondary Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI secondary device
// Refer to the "Wire Main Reader" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

uint8_t ct_rsp[4];

void setup()
{
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onRequest(requestEvent); // register event
}

void loop()
{
  delay(100);
  for (int i = 0; i < 4; i++){
    ct_rsp[i] = i;
  }
}

// function that executes whenever data is requested by main
// this function is registered as an event, see setup()
void requestEvent()
{
  Wire.write(ct_rsp,4); // respond with message of 6 bytes
                       // as expected by main
}
