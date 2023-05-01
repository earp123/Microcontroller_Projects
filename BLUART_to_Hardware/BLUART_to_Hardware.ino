#include <bluefruit.h>

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

//Pins
const int data_pin = A0;
bool stop_flag = 0;



const uint8_t zero_8  = 0;

uint8_t controller_response[4] = {0x01,0x0f,0xf0,0xff};


//prescaled-256 values
const uint8_t comp_4 = 250;//4ms
const uint8_t comp_3 = 188;//3ms
const uint8_t comp_1 = 63; //1ms

//init buffers
uint8_t OCR2B_vals[32];
uint8_t buf[sizeof(OCR2B_vals)];

//response bit
int res_bit = 0;

void setup(void) {
  
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println(F("Adafruit Bluefruit52 Controller App Example"));
  Serial.println(F("-------------------------------------------"));

  //Set LED pin to be output
  DDRB |= (1 << data_pin);

  //Reset Timers Control Reg A
  TCCR2A = 0x00;
  TCCR2B = 0x00;
  
  // Set CTC mode
  TCCR2A |= (1 << WGM21);
  
  //Set Timer2 to prescaler of 256
  TCCR2B |= (1 << CS22);
  TCCR2B |= (1 << CS21);
  //Reset Timers and set compare values
  TCNT2 = zero_8;
  OCR2A = comp_4;
  
  //Enable Timers compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  TIMSK2 |= (1 << OCIE2B);
  
  //enable global interrupts
  sei();

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

void loop() {

  // Wait for new data to arrive
  uint8_t len = readPacket(&bleuart, 500);
  if (len == 0) return;

  // Got a packet!
  printHex(packetbuffer, len);
    
    if (stop_flag){
      //Halt the pulses
      TIMSK2 &= ~(1 << OCIE2A);
      TIMSK2 &= ~(1 << OCIE2B);

      //Clear the data pin
      PORTB  &= ~(1 << data_pin);

      //get new four new controller_response bytes
      
      //load output buffer
      for (int i = 0; i < 4; i++){
        encode_byte_to_out_comp(controller_response[i], OCR2B_vals, i);
      }
      memcpy(buf, OCR2B_vals, 32);
      stop_flag = 0;
      
      delay(10);//wait for poll signal
      
      PORTB |= (1 << data_pin);
      TIMSK2 |= (1 << OCIE2A);
      TIMSK2 |= (1 << OCIE2B);
    
    }
}

void encode_byte_to_out_comp(uint8_t cntrllr_byte, uint8_t out_B[], int byte_idx)
{
  //Serial.print(cntrllr_byte, HEX);
  //Serial.print(", ");

  for (int i = 0; i < 8; i++){
    if ((cntrllr_byte >> i) & 1U){
      out_B[byte_idx*8+i] = comp_1;
      //Serial.print(*out_B[byte_idx*8+i], DEC);
      //Serial.print(", ");
    }
    else {
      out_B[byte_idx*8+i] = comp_3;
      //Serial.print(*out_B[byte_idx*8+i], DEC);
      //Serial.print(", ");
    }
  }
  
  
}
    
ISR(TIMER2_COMPB_vect) 
{
  PORTB |= (1 << data_pin);
}

ISR(TIMER2_COMPA_vect) 
{
  PORTB &= ~(1 << data_pin);
  OCR2B = buf[res_bit];
  if (res_bit == 31)
  {
    res_bit = 0;
    stop_flag = 1;
  }
  else res_bit++;
}
