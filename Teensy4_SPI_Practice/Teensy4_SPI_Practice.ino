uint16_t  mem_buf;

uint16_t controller_response;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial1.addMemoryForRead(mem_buf, sizeof(mem_buf));

}

void loop() {
  // put your main code here, to run repeatedly:

  while (Serial1.available() > 0) {

    int first = Serial1.parseInt();

    int secnd = Serial1.parseInt();

    if (Serial1.read() == '\n') {
      controller_response = (first<<8) & secnd;
      Serial.println(controller_response);
    }
  
  }

  delay(100);
  

}
