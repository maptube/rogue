//ROGUE Drone racing project
//PPM Encoder used to drive a Taranis via the trainer jack input

#include "PPMEncoder.h"

#define pin_PPM_OUT 2

void setup() {
  // put your setup code here, to run once:
  ppmEncoder.init(pin_PPM_OUT);

}

uint16_t value = 1000; //PPMEncoder::CHANNEL_MIN_uS;

void loop() {
  // put your main code here, to run repeatedly:
  
  ppmEncoder.setTXChannel(0,value);
  //ppmEncoder.setTXChannel(1,value);
  //ppmEncoder.setTXChannel(2,value);
  value+=100;
  if (value>PPMEncoder::CHANNEL_MAX_uS)
    value = PPMEncoder::CHANNEL_MIN_uS;
  delay(1000);
}
