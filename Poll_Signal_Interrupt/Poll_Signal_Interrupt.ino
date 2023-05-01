#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)


volatile int poll = 0;

void my_isr() {  // compare
  TMR4_CSCTRL2 &= ~(TMR_CSCTRL_TCF1);
  poll = 1;
  while(TMR4_CSCTRL2 & TMR_CSCTRL_TCF1);
}

void setup_poll_counter(){

  CCM_CCGR6 |= CCM_CCGR6_QTIMER4(CCM_CCGR_ON);

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 = 1;    // QT4 Timer2 on pin 9

  TMR4_CTRL2 = 0; 
  TMR4_LOAD2 = 0;  // start val after compare
  TMR4_COMP12 = 8;
  TMR4_CMPLD12 = 8;
  TMR4_CSCTRL2 = TMR_CSCTRL_TCF1EN;  // enable interrupt
  TMR4_CTRL2 = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ;

  attachInterruptVector(IRQ_QTIMER4, my_isr);
  NVIC_ENABLE_IRQ(IRQ_QTIMER4);
  
}

void setup() {
  
  Serial.begin(9600);
  while (!Serial);
  delay(1000);


  setup_poll_counter();
  
}

void loop() {
  
  if (poll){
    TMR4_CSCTRL2 &= ~TMR_CSCTRL_TCF1EN;
    Serial.println("Polled");
    poll = 0;
    TMR4_CSCTRL2 |= TMR_CSCTRL_TCF1EN;
  }
  else
  {
    Serial.println(".");
  }

  delay(100);
  

}
