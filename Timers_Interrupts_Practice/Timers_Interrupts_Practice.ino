

//Pins
const int data_pin = PB5;
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

void setup() {
  
  Serial.begin(115200);

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
}

void loop() {
    
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
