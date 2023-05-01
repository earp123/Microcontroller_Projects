// count ticks from external pin with  QTIMER4 chnl 2  pin 9 GPIO_B0_11
// test with PWM on pin 8   jumper to 9
#define data_pin (1<<3);
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;

uint32_t prev;

IntervalTimer it1;
volatile uint32_t comp_flag, dataReady;

void my_isr() {
  //if (TMR4_SCTRL2 & TMR_SCTRL_TCF){
  
  GPIO7_DR_TOGGLE = data_pin;
  
  //}
  
  //dataReady = TMRx->CH[2].CNTR;
  
}

void setup() {
  int cnt;

  Serial.begin(9600);
  while (!Serial);
  delay(1000);


  CCM_CCGR6 |= CCM_CCGR6_QTIMER4(CCM_CCGR_ON); //enable QTMR4

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 = 1;    // QT4 Timer2 on pin 9
  GPIO7_GDIR |= data_pin;

  TMR4_CTRL2 = 0; 
  TMR4_LOAD2 = 0;  // start val after compare
  TMR4_COMP12 = 11; // n-1 for a 9 bit poll signal
  TMR4_CMPLD12 = 11;
  TMR4_SCTRL2 = 0;

//  TMRx->CH[3].CTRL = 0; // stop
//  TMRx->CH[3].CNTR = 0;
//  TMRx->CH[3].LOAD = 0;  // start val after compare
//  TMRx->CH[3].COMP1 = 0;
//  TMRx->CH[3].CMPLD1 = 0;
//  TMRx->CH[3].CTRL = TMR_CTRL_CM(7) | TMR_CTRL_PCS(6);  //clock from clock 2

 TMR4_CTRL2 = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ; //configure the poll pin
            //rising edges^  | souce poll pin  | TODO poll pin needs to be the same as data pin
 attachInterruptVector(IRQ_QTIMER4, my_isr);
  NVIC_ENABLE_IRQ(IRQ_QTIMER4);
}

void loop() {
  
  
  
  //if (comp_flag){
    //Serial.println("Compare success");
    //Serial.println(dataReady);
  //}

  //delay(500);
  
}
