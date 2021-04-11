#include "PPMEncoder.h"

PPMEncoder ppmEncoder;

//hardware initialisation
void PPMEncoder::init(uint8_t ppmPinNumber) {
  //set up digital output pin for PPM output - we're using PULSE_LOW=true to invert the signal if required
  ppmOutPin = ppmPinNumber;
  pinMode(ppmOutPin, OUTPUT);
  digitalWrite(ppmOutPin,LOW^inverted);

  ppmOutState=true;
  currentChannel = 0;

  //set all channels centre position
  for (int i=0; i<NUM_CHANNELS; i++) txChannels[i]=(CHANNEL_MIN_uS+CHANNEL_MAX_uS)/2;

  //HARDWARE SETUP//

  cli(); //disable interrupts while we set things up

  //Use timer 1 counter, as it is 16 bit.
  //WGM=waveform generator mode, COM=compare output mode, CS=clock scaling
  //We want Timer 1A counting and generating interrupts when the count reaches OCR1A (output compare register 1A) value
  //A pre-scaler is used with Timer 1 to divide the clock by 8, so 16MHz becomes 2MHz. This puts the frame
  //timings in the 16 bit counter range and means that it's counting in 0.5 uSec intervals. Multiply uS timings
  //by two when setting timer delays in the software.
  //The following data is from the ATMEL datasheets.
  
  //TCCR1A - timer control register 1A
  //7      6      5      4      3 2 1     0
  //COM1A1 COM1A0 COM1B1 COM1B0 - - WGM11 WGM10
  //bits 7,6 compare output mode channel a
  //bits 5,4 compare output mode channel b
  TCCR1A = 0;

  //TCCR1B - timer control register 1B
  //7     6     5 4     3     2    1    0
  //ICNC1 ICES1 - WGM13 WGM12 CS12 CS11 CS10
  //CS=clock scaling 001 = no scaling, 010 = clk/8 scaling (of 16MHz -> 2MHz)
  TCCR1B = (1 << WGM12) | (1 << CS11);

  //OCR1A - output compare register 1A
  OCR1A = 255; //we dont' want it to trigger just yet

  //TIMSK1 - Timer 1 interrupt 
  //7 6 5     4 3 2      1      0
  //- - ICIE1 - - OCIE1B OCIE1A TOIE1
  //OCIE1B interrupt enable B match compare
  //OCIE1A interrupt enable A match compare
  TIMSK1 = (1<<OCIE1A); //mask for generating interrupts on timer 1 match with A unit

  sei(); //interrupts back on and we're good to go
}

//channel number to set (0..7) where 0 is the TX channel 1
//value is the timing in uSec, so 500..3000?
void PPMEncoder::setTXChannel(uint8_t channelNum, uint16_t value) {
  //sanity check on value
  if (value<CHANNEL_MIN_uS) value = CHANNEL_MIN_uS;
  else if (value>CHANNEL_MAX_uS) value = CHANNEL_MAX_uS;
  txChannels[channelNum]=value;
}

void PPMEncoder::timer1CompAInterrupt() {
  TCNT1=0; //reset counter, although it should do automatically

  if (ppmOutState) {
    //we've just timed out on a high pulse, so send it low for the channel count
    digitalWrite(ppmOutPin, LOW^inverted); //xor flips the high/low if inverted=true
    if (currentChannel>=NUM_CHANNELS) {
      currentChannel=0;
      accFrameUS += PPM_SYNC_PULSE_uS;
      OCR1A = (PPM_FRAME_LENGTH_uS-accFrameUS) * 2; //calculate remaining time to get constant frame rate
      accFrameUS=0; //reset frame
    }
    else {
      uint16_t chan = txChannels[currentChannel];
      OCR1A = (chan - PPM_SYNC_PULSE_uS) * 2;
      accFrameUS += chan;
      ++currentChannel;
    } 
  }
  else {
    //we're currently low, so send it high for a short pulse - this is signalling a sync pulse
    digitalWrite(ppmOutPin, HIGH^inverted); //xor flips the high/low if inverted=true
    OCR1A = PPM_SYNC_PULSE_uS * 2;
  }
  ppmOutState = !ppmOutState; //flip state for next time
}

ISR(TIMER1_COMPA_vect){
  //timer1 compare A interrupt service routine
  ppmEncoder.timer1CompAInterrupt();
}
