#include "SdFat.h"
#include "sdios.h"

static ArduinoOutStream cout(Serial);

const int8_t DISABLE_CS_PIN_1 = 33;
const int8_t DISABLE_CS_PIN_2 = 3;

const uint8_t SD_CS_PIN = A4;

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(16))

SdFs sd;
SdFile dataFile;

void setup() {
  Serial.begin(9600);
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }

  cout << F("\nDisabling SPI device on pin ");
  cout << int(DISABLE_CS_PIN_1) << endl;
  pinMode(DISABLE_CS_PIN_1, OUTPUT);
  digitalWrite(DISABLE_CS_PIN_1, HIGH);

  cout << F("\nDisabling SPI device on pin ");
  cout << int(DISABLE_CS_PIN_2) << endl;
  pinMode(DISABLE_CS_PIN_2, OUTPUT);
  digitalWrite(DISABLE_CS_PIN_2, HIGH);

  if (!sd.cardBegin(SD_CONFIG)) {
    cout << F(
            "\nSD initialization failed.\n"
            "Do not reformat the card!\n"
            "Is the card correctly inserted?\n"
            "Is there a wiring/soldering problem?\n");
    if (isSpi(SD_CONFIG)) {
      cout << F(
            "Is SD_CS_PIN set to the correct value?\n"
            "Does another SPI device need to be disabled?\n"
            );
    }
    while(1);
 
  }

  // test SD and write something
  else {
    Serial.println("SD OK");
    dataFile.open("Test.txt", FILE_WRITE);
    dataFile.print("This is a test");
    dataFile.close();
  }


}

void loop() {
  // put your main code here, to run repeatedly:

}
