int set_gap_toggle = 0;

void setup() {
  
  // Configure GPIO B0_03 (PIN 13) for output
    IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
    IOMUXC_GPR_GPR27 = 0xFFFFFFFF;
    GPIO7_GDIR |= (1 << 3);

    CCM_CSCMR1 &= ~CCM_CSCMR1_PERCLK_CLK_SEL; // turn off 24mhz mode

    CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) ;  // enable GPT1 module
    GPT1_CR = 0;
    GPT1_PR = 0;   // prescale+1
    GPT1_OCR1 = 600;  // compare
    GPT1_SR = 0x3F; // clear all prior status
    GPT1_IR = GPT_IR_OF1IE;
    GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(1);
    attachInterruptVector(IRQ_GPT1, gpt1_isr);
    NVIC_ENABLE_IRQ(IRQ_GPT1);

    CCM_CCGR0 |= CCM_CCGR0_GPT2_BUS(CCM_CCGR_ON);  // enable GPT2 module
    GPT2_CR = 0;
    GPT2_PR = 0;   // prescale+1
    GPT2_OCR1 = 150;
    GPT2_SR = 0x3F; // clear all prior status
    GPT2_IR = GPT_IR_OF1IE;
    GPT2_CR = GPT_CR_EN | GPT_CR_CLKSRC(1);
    attachInterruptVector(IRQ_GPT2, gpt2_isr);
    NVIC_ENABLE_IRQ(IRQ_GPT2);

    

}

void loop() {
  
  delay(1000);

}

void gpt2_isr() {
  
  
  GPT2_SR |= GPT_SR_OF3;  // clear all set bits
  GPIO7_DR_SET = (1 << 3);
  while (GPT2_SR & GPT_SR_OF1); // wait for clear
  
}

void gpt1_isr() {
  
  
  GPT1_SR |= GPT_SR_OF3;  // clear all set bits
  GPIO7_DR_CLEAR = (1 << 3);
  
  if (set_gap_toggle) GPT2_OCR1 = 121;
  else                GPT2_OCR1 = 422;
  set_gap_toggle = !set_gap_toggle;
  
  
  while (GPT1_SR & GPT_SR_OF1); // wait for clear
  
}
