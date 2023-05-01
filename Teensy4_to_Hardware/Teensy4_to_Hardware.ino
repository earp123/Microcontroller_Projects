#define data_pin (1 << 3)

//TODO final stitch of poll-response singals

const uint32_t controller_response = 0x0;

const uint32_t bit_mask    = 0xFFFFFF;
const uint32_t press_a_btn = 1;
const uint32_t press_b_btn = (1U << 1);
const uint32_t press_z_btn = (1U << 2);
const uint32_t press_start = (1U << 3);
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

volatile int stop_flag = 0;
volatile int poll = 0;

//150MHz intervals
const uint32_t comp_4 = 600;//4us
const uint32_t comp_3 = 407;//3us
const uint32_t comp_1 = 113; //1us

//init buffers
uint32_t comp_vals[33];
uint32_t buf[sizeof(comp_vals)];

//response bit
int res_bit = 0;

void config_pin(){
   // Configure GPIO B0_03 (PIN 13) for output
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
    IOMUXC_GPR_GPR27 = 0xFFFFFFFF;

    CORE_PIN33_CONFIG = INPUT_PULLUP;
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

void poll_counter_init(){

  CCM_CCGR6 |= CCM_CCGR6_QTIMER4(CCM_CCGR_ON);

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 = 1;    // QT4 Timer2 on pin 9

  TMR4_CTRL2 = 0; 
  TMR4_LOAD2 = 0;  // start val after compare
  TMR4_COMP12 = 8; // n-1 for a 9 bit poll signal
  TMR4_CMPLD12 = 8;
  
  TMR4_CTRL2 = TMR_CTRL_CM(0) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ; //configure the poll pin
      //leave off at start ^  | sour ce poll pin  | TODO poll pin needs to be the same as data pin
  attachInterruptVector(IRQ_QTIMER4, my_isr);
  NVIC_ENABLE_IRQ(IRQ_QTIMER4);
  
}

//Must be initialized before enabling. 
void enable_poll_counter(){
  TMR4_CNTR2 = 0;
  TMR4_LOAD2 = 0;  // start val after compare
  
  TMR4_CTRL2 |= TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ;
  TMR4_CSCTRL2 |= TMR_CSCTRL_TCF1EN;  // enable interrupt
}

void disable_poll_counter(){
  TMR4_CTRL2 = 0;
  TMR4_CSCTRL2 = 0;
  GPIO7_DR_SET = data_pin;
}

int master_count = 0;

void setup() {

    Serial.begin(115200);
    
    config_pin();
    
    poll_counter_init();
    config_timers();
    enable_poll_counter();
    
    
}



void loop() {

  if (poll){
    
      disable_poll_counter();
    
      //Clear the data pin
      //GPIO7_DR_SET = data_pin;

      delayMicroseconds(30);

      
      
      GPT1_CR |= GPT_CR_EN;
       //GPT2_CR |= GPT_CR_EN;
      delayMicroseconds(130);//wait for the signal to go

//        if(Serial.available()){
//
//          char key_command = Serial.read();
//    
//          switch(key_command){
//    
//            case 'a':{
//              encode_byte_to_out_comp(press_a_btn, comp_vals);
//            }break;
//            case 'b':{
//              encode_byte_to_out_comp(press_b_btn, comp_vals);
//            }break;
//            case 'z':{
//              encode_byte_to_out_comp(press_z_btn, comp_vals);
//            }break;
//            case 's':{
//              encode_byte_to_out_comp(press_start, comp_vals);
//            }break;
//            case 'u':{
//              encode_byte_to_out_comp(press_up, comp_vals);
//            }break;
//            case 'd':{
//              encode_byte_to_out_comp(press_down, comp_vals);
//            }break;
//            case 'l':{
//              encode_byte_to_out_comp(press_left, comp_vals);
//            }break;
//            case 'r':{
//              encode_byte_to_out_comp(press_right, comp_vals);
//            }break;
//
//            More keystokes can be added here TODO
//            
//            default:{
//              encode_byte_to_out_comp(controller_response, comp_vals);
//          }
//          }
//        }  
//        else{
            encode_byte_to_out_comp(controller_response, comp_vals);
//        } 
//          //Serial.println((char)key_command);

      

      memcpy(buf, comp_vals, sizeof(buf));
      
      poll = 0;
      enable_poll_counter();//enable for new poll
      
   }
   

}

void encode_byte_to_out_comp(uint32_t cntrllr_bytes, uint32_t out_B[])
{
  //Serial.println(cntrllr_bytes, HEX);
  //Serial.println(", ");

  for (int i = 0; i < 32; i++){
    if ((cntrllr_bytes >> i) & 1U){
      out_B[i] = comp_1;
      //Serial.print(out_B[i], DEC);
      //Serial.print(", ");
    }
    else {
      out_B[i] = comp_3;
      //Serial.print(out_B[i], DEC);
      //Serial.print(", ");
    }
  }
  out_B[32] = comp_1;
  
}

void gpt2_isr() {
  
  
  GPT2_SR |= GPT_SR_OF3;  // clear all set bits
  GPIO7_DR_SET = data_pin;
  while (GPT2_SR & GPT_SR_OF1); // wait for clear
  
}

void gpt1_isr() {
  
  
  GPT1_SR |= GPT_SR_OF3;  // clear all status register bits
  GPT2_CR &= ~GPT_CR_EN;  //diable the set pin timer
  
  GPT2_OCR1 = buf[res_bit]; //load the next interval, 3us or 1us compare val
  if (res_bit > 32)//check for the end of the buffer
  {
    res_bit = 0;  //reset the idx
    GPT1_CR &= ~(GPT_CR_EN);//halts the pulses
  }
  else {
    res_bit++;
    GPIO7_DR_CLEAR = data_pin;//clear the pin so the HW can read the next interval
    GPT2_CR |= GPT_CR_EN;     //enables and clears the GPT2 set pin timer
  }
  
  while (GPT1_SR & GPT_SR_OF1); // wait for clear
  
}

void my_isr() {  // compare
  TMR4_CSCTRL2 &= ~(TMR_CSCTRL_TCF1);
  poll = 1;
  while(TMR4_CSCTRL2 & TMR_CSCTRL_TCF1);
}
