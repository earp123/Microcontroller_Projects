/*  Nintendo 64 Controller Input capture on Teensy 4/4.1
*
*   by Sam Rall
*
*   Sends poll signal to controller and receives the controller response (buttons pressed)
*   on the same pin poll_pin. Converts the response by comparing the intervals between pulses.
*   Outputs the controller response as a byte array of 4 bytes.
*/


//int inPin = 38;
int poll_pin = 31;

unsigned long duration[32];
unsigned long cont_rsp = 0;


//TODO This needs to be faster! Need to address registers directly. :(
void generate_poll(){

  pinMode(poll_pin, OUTPUT);
  digitalWriteFast(poll_pin, HIGH);

  for(int j = 0; j<7; j++){
    digitalWriteFast(poll_pin, LOW);
    delayNanoseconds(2950);
    digitalWriteFast(poll_pin, HIGH);
    delayNanoseconds(1000);
    
  }

  
  digitalWriteFast(poll_pin, LOW);
  delayNanoseconds(1000);
  digitalWriteFast(poll_pin, HIGH);
  delayNanoseconds(3000);
  digitalWriteFast(poll_pin, LOW);
  delayNanoseconds(1000);
  digitalWriteFast(poll_pin, HIGH);

  pinMode(poll_pin, INPUT_PULLUP);
  
}


void setup()


{
  Serial.begin(115200);
  
  pinMode(poll_pin, INPUT_PULLUP);

  Serial1.begin(1000000);
  
  
}



void loop()

{
  
  generate_poll();
  
  for(int i = 0; i <= 31; i++){
    duration[i] = hf_pulseIn(poll_pin, HIGH, 1000);

    if(duration[i] > 100)    cont_rsp |= (1<<(31-i));
    else if(duration[i] > 0) cont_rsp &= ~(1<<(31-i));
  }

  Serial1.write((byte*) &cont_rsp, 4);
  //Serial.write((byte*) &cont_rsp, 4);
  
}

  
