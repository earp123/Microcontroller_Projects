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

// OTA DFU service
BLEDfu bledfu;

// Uart over BLE service
BLEUart bleuart;

// Function prototypes for packetparser.cpp
uint8_t readPacket (BLEUart *ble_uart, uint16_t timeout);
float   parsefloat (uint8_t *buffer);
void    printHex   (const uint8_t * data, const uint32_t numBytes);
uint16_t output_buf = 0;
uint16_t test_byte = 0xf010;
uint8_t test_buf[2] = {123,1};

// Packet buffer
extern uint8_t packetbuffer[];

void setup(void)
{
  Serial.begin(115200);
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial1.begin(115200);

  

  Serial.println(F("Adafruit Bluefruit52 Controller App Example"));
  Serial.println(F("-------------------------------------------"));

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("Bluefruit52");

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



/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
 
  Serial1.println(test_byte);
  Serial.print("0b"); Serial.println(output_buf, BIN);
  
  // Wait for new data to arrive
  uint8_t len = readPacket(&bleuart, 1);
  if (len == 0){
    delay(250);
    return;
  }
  

  // Got a packet!
  // printHex(packetbuffer, len);



  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
//    Serial.print ("Button "); Serial.print(buttnum);
//    if (pressed) {
//      Serial.println(" pressed");
//    } else {
//      Serial.println(" released");
//    }

    switch(buttnum)
    {
      case 1:
      {
        //if (pressed)output_buf |= (1U<<15);//Serial.println("0x11");
        //else output_buf &= ~(1U<<15);//Serial.println("0x01");

        output_buf = pressed ? (output_buf | (1U<<15)) : (output_buf & ~(1U<<15));
        break;
      }

      case 2:
      {
        output_buf = pressed ? (output_buf | (1U<<14)) : (output_buf & ~(1U<<14));
        break;
      }

      case 3:
      {
         output_buf = pressed ? (output_buf | (1U<<13)) : (output_buf & ~(1U<<13));
         break;
      }

      case 4:
      {
        output_buf = pressed ? (output_buf | (1U<<12)) : (output_buf & ~(1U<<12));
        break;
      }

      case 5:
      {
        output_buf = pressed ? (output_buf | (1U<<11)) : (output_buf & ~(1U<<11));
        break;
      }

      case 6:
      {
        output_buf = pressed ? (output_buf | (1U<<10)) : (output_buf & ~(1U<<10));
        break;
      }

      case 7:
      {
        output_buf = pressed ? (output_buf | (1U<<9)) : (output_buf & ~(1U<<9));
        break;
      }

      case 8:
      {
        output_buf = pressed ? (output_buf | (1U<<8)) : (output_buf & ~(1U<<8));
        break;
      }

      default: break;
    }
  }

  
}
