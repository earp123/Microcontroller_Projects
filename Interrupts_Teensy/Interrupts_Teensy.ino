//Pins
const int data_pin = 13;
bool stop_flag = 0;

//Intervals used
const uint16_t comp_4 = 10000;//4uS
const uint16_t comp_3 = 7500;//3us
const uint16_t comp_1 = 2500; //1us

//Dummy controller data
uint8_t controller_response[4] = {0x01,0x0f,0xf0,0xff};

//init buffers
uint16_t OCR1B_vals[32];
uint16_t buf[sizeof(OCR1B_vals)];

//response bit index
//identifies which of the 32 bits is being evaluated
int res_bit = 0;

void setup(void)
{
  
  Serial.begin(115200);
  // Configure GPIO B0_03 (PIN 13) for output
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
    IOMUXC_GPR_GPR27 = 0xFFFFFFFF;
    GPIO7_GDIR |= (1 << 3);
  


  //Restart mode for the 
  GPT1_CR &= ~GPT_CR_FRR;

  //Free Run Mode for the set_pin timer
  GPT2_CR &= ~GPT_CR_FRR;
  
  
  //Select Clock Source
  GPT1_CR |= GPT_CR_CLKSRC(2);
  GPT2_CR |= GPT_CR_CLKSRC(2);

  //No prescaler - ok maybe a little one
  GPT1_PR = 0x004;
  GPT2_PR = 0x004;
  
  //clear Timers and configure Timer 1 compare value
  GPT1_CNT = 0;
  GPT2_CNT = 0;
  GPT1_OCR1 = comp_4;
  
  //Enable compare interrupt
  GPT1_IR |= GPT_IR_OF1IE;
  GPT2_IR |= GPT_IR_OF1IE;
  

  //Enable Timers
  GPT1_CR |= GPT_CR_EN;
  GPT2_CR |= GPT_CR_EN;

  //attach the intterupt callbacks
  attachInterruptVector(IRQ_GPT1,clear_data_pin);
  attachInterruptVector(IRQ_GPT2,set_data_pin);
  
  //enable global interrupts
  interrupts();
}

void loop(void)
{
  Serial.println("We are printing");
  
  if (stop_flag){

      
      
      //Stop the timers
      GPT1_CR &= ~GPT_CR_EN;
      GPT2_CR &= ~GPT_CR_EN;;

      //Clear the data pin
      // Set PIN 13 LOW
      GPIO7_DR_CLEAR = (1 << 3);

      //get new four new controller_response bytes TODO
      
      //load output buffer, clear stop flag
      for (int i = 0; i < 4; i++){
       encode_byte_to_out_comp(controller_response[i], OCR1B_vals, i);
      }
      memcpy(buf, OCR1B_vals, 32);
      stop_flag = 0;
      
      delay(10);//wait for poll signal TODO

      //set the data pin
      // Set PIN 13 HIGH
      GPIO7_DR_SET = (1 << 3);

      //start the timers
      GPT1_CR |= GPT_CR_EN;
      GPT2_CR |= GPT_CR_EN;
    
    }
}

void encode_byte_to_out_comp(uint8_t cntrllr_byte, uint16_t out_B[], int byte_idx)
{
  for (int i = 0; i < 8; i++)
  {
    if ((cntrllr_byte >> i) & 1U) out_B[byte_idx*8+i] = comp_1;
    else                          out_B[byte_idx*8+i] = comp_3;
  }
}

void set_data_pin()
{
  // Set PIN 13 LOW
  GPIO7_DR_SET = (1 << 3);
}

void clear_data_pin(void) 
{
  // Set PIN 13 LOW
  GPIO7_DR_CLEAR = (1 << 3);
  
  GPT2_OCR1 = 5000;//buf[res_bit];
  if (res_bit == 31)
  {
    res_bit = 0;
    stop_flag = 1;
  }
  else res_bit++;

  //essentially start the other timer
  GPT2_CNT = 0;
}
