/*  Joybus driver for the Teensy 4/4.1
*
*   by Sam Rall
*
*   Receives characters from Serial1 line and outputs controller info and button presses on data_pin.
*   Receives the poll command from the Ninentendo 64 on poll_pin.
*/

#define data_pin (1 << 3)
#include "core_pins.h"

uint8_t poll_pin = 8;
uint8_t res_bit_width = 32;
 

unsigned long command_buf[9];
unsigned long timeout_us = 35000;

const uint32_t DEFAULT_INPUT = 0x0;

const uint32_t bit_mask    = 0xFFFFFF;
const uint32_t press_a_btn = 1;
const uint32_t press_b_btn = (1U << 1);
const uint32_t press_z_btn = (1U << 2);
const uint32_t START = (1U << 3);
const uint32_t press_up    = (1U << 4);
const uint32_t press_down  = (1U << 5);
const uint32_t press_left  = (1U << 6);
const uint32_t press_right = (1U << 7);
const uint32_t lt_shldr    = (1U << 10);
const uint32_t rt_shldr    = (1U << 11);
const uint32_t c_up        = (1U << 12);
const uint32_t c_down      = (1U << 13);
const uint32_t c_left      = (1U << 14);
const uint32_t c_right     = (1U << 15);
const uint32_t x_axis_left  = (0b01111111 << 8);
const uint32_t x_axis_right = (0b11111111 << 8);
const uint32_t y_axis_left  = (0b01111111);
const uint32_t y_axis_right = (0b11111111);

const uint16_t CONTROLLER_INFO_RESPONSE = 0x0050;


volatile int stop_flag = 0;
volatile int poll = 0;

//150MHz intervals
const uint32_t comp_4 = 580;//4us
const uint32_t comp_3 = 400;//3us
const uint32_t comp_1 = 110; //1us

//init output buffer
uint32_t output_buf[33];

//buffer from Serial Monitor
uint32_t input_bytes;



//response bit
int res_bit = 0;

void config_pin(){
   // Configure GPIO B0_03 (PIN 13) for output
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
    IOMUXC_GPR_GPR27 = 0xFFFFFFFF;

    CORE_PIN33_CONFIG = INPUT_PULLDOWN;
    GPIO7_GDIR |= data_pin;
    
}

void config_timers(){

    CCM_CSCMR1 &= ~CCM_CSCMR1_PERCLK_CLK_SEL; // turn off 24mhz mode

    CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) ;  // enable GPT1 module
    GPT1_CR = 0;   // clear prev config
    GPT1_PR = 0;   // No prescaler
    GPT1_OCR1 = comp_4;  // compare for one whole period
    GPT1_SR = 0x3F; // clear all prior status
    GPT1_IR = GPT_IR_OF1IE; //enable compare interrupt
    GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(1); //start timer, set clock,

    //enable the ISRs
    attachInterruptVector(IRQ_GPT1, gpt1_isr);
    NVIC_ENABLE_IRQ(IRQ_GPT1);

    CCM_CCGR0 |= CCM_CCGR0_GPT2_BUS(CCM_CCGR_ON);  // enable GPT2 module
    GPT2_CR = 0;
    GPT2_PR = 0;   
    GPT2_OCR1 = 150;//dummy value that won't get used
    GPT2_SR = 0x3F; 
    GPT2_IR = GPT_IR_OF1IE;
    GPT2_CR = GPT_CR_EN | GPT_CR_CLKSRC(1) | GPT_CR_ENMOD; //restart after reenable
    
    attachInterruptVector(IRQ_GPT2, gpt2_isr);
    NVIC_ENABLE_IRQ(IRQ_GPT2);
}

void encode_info_response(uint32_t info_bytes, uint32_t out_I[])
{

  //encode two bytes with controller info response code(0x0050)
  for (int i = 0; i < 16; i++){
    if ((info_bytes >> i) & 1U){
      out_I[i] = comp_1;

    }
    else {
      out_I[i] = comp_3;

    }
  }

  //fill extra byte of data with 0s
  for (int i = 16; i < 24; i++){
    out_I[i] = comp_3;
  }

  //2us for the stop bit
  out_I[24] = 2*comp_1;
  
}

void encode_input_state(uint32_t cntrllr_bytes, uint32_t out_B[])
{

  for (int i = 0; i < 32; i++){
    
    if ((cntrllr_bytes >> i) & 1U)
      out_B[i] = comp_1;
    
    else 
      out_B[i] = comp_3;
  }

  //2us for the stop bit
  out_B[32] = 2*comp_1;


  
}

void setup()

{
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(poll_pin, INPUT);

  config_pin();
  config_timers();

  encode_input_state(DEFAULT_INPUT, output_buf);

}

void loop()

{ 
    
  //receive the command poll
  for(int i = 0; i < 9; i++){
    command_buf[i] = hf_pulseIn(poll_pin, LOW, timeout_us);
    if (!command_buf[i]) return;
  }

  //detect the command and send appropriate response
  if (command_buf[7] < 800) //controller input state polled 0x01 + 1
  {
    send_input_response();
  }
  else                      //info request polled 0x00 + 1
  {
    encode_info_response(CONTROLLER_INFO_RESPONSE, output_buf);
    send_controller_info_response();
  }
    
}

void send_controller_info_response()
{
  res_bit_width = 24;
  delayMicroseconds(2);
  
  //load the first interval, 3us or 1us compare val
  GPT2_OCR1 = output_buf[res_bit]; 
  
  //increase the bit counter
  res_bit++;

  //falling edge of the 4us period
  GPIO7_DR_CLEAR = data_pin;
  
  //start GP Timer 1 to begin Tx
  GPT1_CR |= GPT_CR_EN;
  
  //enables and clears the GPT2 set pin timer value
  GPT2_CR |= GPT_CR_EN;
       
  delayMicroseconds(200);//wait for the signal to transmit

}

void send_input_response()
{
  res_bit_width = 32;
  delayMicroseconds(2);
  
  
  //load the first interval, 3us or 1us compare val
  GPT2_OCR1 = output_buf[res_bit]; 
  
  //increase the bit counter
  res_bit++;

  //falling edge of the 4us period
  GPIO7_DR_CLEAR = data_pin;
  
  //start GP Timer 1 to begin Tx
  GPT1_CR |= GPT_CR_EN;
  
  //enables and clears the GPT2 set pin timer value
  GPT2_CR |= GPT_CR_EN;

 

  /*  ~SWR~ First attempt at deserializing the incoming controller input response into a uint32

  uint8_t byte_buf[4];
  uint32_t cntr_inpt_rsp = 0;

  if (Serial1.readBytes(byte_buf, 4) < 4)
  {
    Serial.println("Error reading controller input response");
    encode_input_state(DEFAULT_INPUT, output_buf);
  }
  else {
    cntr_inpt_rsp |= byte_buf[0] << 24;
    cntr_inpt_rsp |= byte_buf[1] << 16;
    cntr_inpt_rsp |= byte_buf[2] << 8;
    cntr_inpt_rsp |= byte_buf[3];

    encode_input_state(cntr_inpt_rsp, output_buf);
  }

  */

  /*  OR
  * This works decently. Can only receive a character and output
  * one button press at a time. Useful for connecting any ble uart contrller to
  * receive 'command' chars wirelessly and send via Serial1.
  */
  
  char i_char = Serial1.read();
  if      (i_char == 's') encode_input_state(START, output_buf);
  else if (i_char == 'a') encode_input_state(press_a_btn, output_buf);
  else if (i_char == 'b') encode_input_state(press_b_btn, output_buf);
  else if (i_char == 'z') encode_input_state(press_z_btn, output_buf);
  else if (i_char == 'u') encode_input_state(press_up, output_buf);
  else if (i_char == 'd') encode_input_state(press_down, output_buf);
  else if (i_char == 'l') encode_input_state(press_left, output_buf);
  else if (i_char == 'r') encode_input_state(press_right, output_buf);
  
  else                    encode_input_state(DEFAULT_INPUT, output_buf);
  
  
  delayMicroseconds(190);//wait for the signal to transmit

}

//~SWR~ modified the Teensy pulseIn fuctions to reutrn clock cycles instead of us
uint32_t hf_pulseIn(uint8_t pin, uint8_t state, uint32_t timeout)
{
  if (pin >= CORE_NUM_DIGITAL) return 0;
  if (state) return hf_pulseIn_high(pin, timeout);
  return hf_pulseIn_low(pin, timeout);
}

uint32_t hf_pulseIn_low(uint8_t pin, uint32_t timeout)
{
  const struct digital_pin_bitband_and_config_table_struct *p;
  p = digital_pin_to_info_PGM + pin;

  uint32_t usec_start, nsec_start, nsec_stop;

  // wait for any previous pulse to end
  usec_start = micros();
  while (!(*(p->reg + 2) & p->mask)) {
    if (micros() - usec_start > timeout) return 0;
  }
  // wait for the pulse to start
  usec_start = micros();
  while ((*(p->reg + 2) & p->mask)) {
    if (micros() - usec_start > timeout) return 0;
  }
  usec_start = micros();
  nsec_start = ARM_DWT_CYCCNT;
  // wait for the pulse to stop
  while (!(*(p->reg + 2) & p->mask)) {
    if (micros() - usec_start > timeout) return 0;
  }
  nsec_stop = ARM_DWT_CYCCNT;
  return nsec_stop - nsec_start;
}


uint32_t hf_pulseIn_high(uint8_t pin, uint32_t timeout)
{
  const struct digital_pin_bitband_and_config_table_struct *p;
  p = digital_pin_to_info_PGM + pin;

  uint32_t usec_start, nsec_start, nsec_stop;

  // wait for any previous pulse to end
  usec_start = micros();
  while ((*(p->reg + 2) & p->mask)) {
    if (micros() - usec_start > timeout) return 0;
  }
  // wait for the pulse to start
  usec_start = micros();
  while (!(*(p->reg + 2) & p->mask)) {
    if (micros() - usec_start > timeout) return 0;
  }
  usec_start = micros();
  nsec_start = ARM_DWT_CYCCNT;
  // wait for the pulse to stop
  while ((*(p->reg + 2) & p->mask)) {
    if (micros() - usec_start > timeout) return 0;
  }
  nsec_stop = ARM_DWT_CYCCNT;
  return nsec_stop - nsec_start;
}


/* Data Pin Set Timer
 * Sets the data pin to 1 at a dynamically changing interval 
 * loaded from Timer 2 Compare Register
 *
 */
void gpt2_isr() {
  
  // clear the flags
  GPT2_SR |= GPT_SR_OF3;  

  //set data pin HIGH
  GPIO7_DR_SET = data_pin;

  // wait for flags to clear
  while (GPT2_SR & GPT_SR_OF1); 
  
}

/* Data Pin Clear Timer
 * Fires periodaclly every 4us
 * Loads the next pin set timer (GPT2) value
 * Clears the data pin 
 */
void gpt1_isr() {
  
  // clear all status register bits
  GPT1_SR |= GPT_SR_OF3;

  //disable the pin set timer 
  GPT2_CR &= ~GPT_CR_EN;  
  
  //load the next interval in GPT2 , 3us or 1us compare val
  GPT2_OCR1 = output_buf[res_bit]; 
  
  //check for the end of the buffer
  if (res_bit > res_bit_width)
  {
    //reset the bit counter
    res_bit = 0;  
    
    //disables GPT1 and ends the transmission
    GPT1_CR &= ~(GPT_CR_EN);
  }
  else {
    
    //continue through the response bits
    res_bit++;
    
    //clear the data pin and start the set pin timer loaded with new value
    GPIO7_DR_CLEAR = data_pin;
    GPT2_CR |= GPT_CR_EN;     
  }
  
  // wait for flags to clear
  while (GPT1_SR & GPT_SR_OF1); 
  
}
  
