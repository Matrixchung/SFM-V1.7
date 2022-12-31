#include "sfm.hpp"

#define SFM_RX 19
#define SFM_TX 18
#define SFM_IRQ 17
#define SFM_VCC 15

SFM_Module SFM(SFM_VCC, SFM_IRQ, SFM_TX, SFM_RX);

bool lastTouchState = 0;

void sfmPinInt1() {
  SFM.pinInterrupt();
}

void setup() {
  SFM.setPinInterrupt(sfmPinInt1); // must perform this step in setup() to attach the inner interrupt.
  Serial.begin(115200); // not affiliated with module
}

uint8_t temp = 0; // used to get recognition return
uint16_t tempUid = 0; // used to get recognized uid

void loop() {
  if(SFM.isTouched() != lastTouchState) { // Make sure the action just performed once when touch state changes
    lastTouchState = SFM.isTouched();
    if(SFM.isTouched()) {
      SFM.setRingColor(SFM_RING_RED, SFM_RING_OFF); // Ring fade from red to black at default period (500ms), creating a breathe effect
      // Here we are going to start recognition, if unlocked, we will change the ring color to green and send a message
      temp = SFM.recognition_1vN(tempUid);
      if(tempUid != 0) {
        SFM.setRingColor(SFM_RING_GREEN);
        Serial.printf("Successfully matched with UID: %d", tempUid);
      }
    }
    else {
      SFM.setRingColor(SFM_RING_OFF);
    }
  }
  if(Serial.available()) { // sending any character to start register
    Serial.println("Start register..");
    SFM.setRingColor(SFM_RING_YELLOW, SFM_RING_OFF);
    Serial.println("Please put your finger");
    temp = SFM.register_3c3r_1st();
    if(temp==SFM_ACK_SUCCESS){
      Serial.println("Please releases your finger");
      delay(2000);
      SFM.setRingColor(SFM_RING_PURPLE, SFM_RING_OFF);
      Serial.println("Please put your finger");
      temp = SFM.register_3c3r_2nd();
      if(temp==SFM_ACK_SUCCESS){
        Serial.println("Please releases your finger");
        delay(2000);
        SFM.setRingColor(SFM_RING_BLUE, SFM_RING_OFF);
        Serial.println("Please put your finger");
        tempUid = 0;
        temp = SFM.register_3c3r_3rd(tempUid);
        if(temp==SFM_ACK_SUCCESS&&tempUid!=0){
          Serial.printf("Register successful with return UID: %d\n", tempUid);
          SFM.setRingColor(SFM_RING_GREEN);
        }
        else{
          Serial.println("Register failed, please re-submit the register command");
          SFM.setRingColor(SFM_RING_RED);
        }
      }
      else Serial.println("Error in register step #2");
    }
    else Serial.println("Error in register step #1");
  }
}