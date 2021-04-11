#ifndef __PPMEncoder_h__
#define __PPMEncoder_h__

#include "Arduino.h"

//pulse length 500us
//frame 22500 us
//channels 8, but 10 max

/**
 * This is the regular PPM signal, where PULSE_LOW=false, so the sync pulses go high.
 * The logic of how the code works follows this form of the signal with the inverted option
 * just xoring the final high or low pin output at the very last stage.
 * PULSE_LOW=false
 * 
 * SYNC=PPM_SYNC_PULSE_uS
 * ->| |<-   
 *     |<-  CH1  ->|<- CH2 ->|<-     CH3        ->|<- CH4,5,6,7,8     ->|
 *    _           _         _                    _                     _                    _ 
 *   | |         | |       | |                  | |                   | |                  | |
 *   | |   CH1   | |  CH2  | |       CH3        | |    x8 CH    CH8   | |  BETWEEN FRAMES  | | CH1
 * __| |_________| |_______| |__________________| |____.........______| |____________....._| |_____...
 * 
 *   |<-----------------          22500uSEC           ----------------->|
 *   
 *   
 *  This is the inverted signal, with PULSE_LOW=true, which is use for the Taranis trainer port signal
 *  PULSE_LOW=true
 * 
 *  SYNC=PPM_SYNC_PULSE_uS
 * ->| |<-   
 *     |<-  CH1  ->|<- CH2 ->|<-     CH3        ->|<- CH4,5,6,7,8     ->|
 * __   _________   _______   __________________   ____.........______   ____________....._   _____...
 *   | |         | |       | |                  | |                   | |                  | |
 *   | |   CH1   | |  CH2  | |       CH3        | |    x8 CH    CH8   | |  BETWEEN FRAMES  | | CH1
 *   |_|         |_|       |_|                  |_|                   |_|                  |_|
 * 
 *   |<-----------------          22500uSEC           ----------------->|
 * 
 * NOTE: diagram is using 8 channels, which is normal for this type of radio, but 10 are possible.
 * NOTE: channels shown as [1..8] here to match radio, but setTXChannel uses [0..7]
 * PULSE_LOW inverts the signal - see diagrams above. Taranis uses PULSE_LOW=true
 * SYNC=PPM_HIGH_PULSE_uS, all low pulses are this length
 * CH[1..8]=HIGH pulse for between CHANNEL_MIN_uS and CHANNEL_MAX_uS (1ms to 2ms typically)
 * Channel timings include the 300uS high pulse, so CHANNEL_MAX_uS includes the 500uS high pulse
 * This gives a total frame length of 10 channels x 2ms + 500us???? NOT 22500! 25000!
 */

class PPMEncoder {
  private:
    uint8_t ppmOutPin; //pin used to output PPM signal on
    bool ppmOutState; //current state of output pin
    uint16_t accFrameUS; //accumulated frame timing uS
    uint16_t txChannels[10]; //define 10 channels, but probably only ever using 8
    uint8_t currentChannel=0; //current channel being output on PPM 
  protected: 
  public:
    static const uint16_t PPM_SYNC_PULSE_uS = 300; //500uSec - not more than CHANNEL_MIN
    static const uint16_t PPM_FRAME_LENGTH_uS = 22500; //22500uSec - allegedly 20ms=2ms*10ch frames plus the start sync pulse
    static const uint8_t NUM_CHANNELS = 8;
    static const uint16_t CHANNEL_MIN_uS = 1000; //minimum channel pulse (uS)
    static const uint16_t CHANNEL_MAX_uS = 2000; //maximum channel pulse (uS)
    static const bool inverted = true; //INVERT SIGNAL - true->sync goes low (see top readme), else sync pulses high
    //methods
    void init(uint8_t ppmPinNumber); //call this first to initialise
    void setTXChannel(uint8_t channelNum, uint16_t value); //call this with value between 500uS and 1500uS (channel min/max)
    void timer1CompAInterrupt();
};

extern PPMEncoder ppmEncoder;

#endif
