import os, sys, inspect, thread, time
## Windows, Linux and Mac
#sys.path.append('C:/richard/projects/github/rogue/3rdparty/LeapSDK/lib/x64') #dll and so files
#sys.path.append('C:/richard/projects/github/rogue/3rdparty/LeapSDK/lib') #Leap.py
#sys.path.append('../3rdparty/LeapSDK/lib/x64') #dll and so files for win/linux/mac
#sys.path.append('../3rdparty/LeapSDK/lib') #Leap.py
sys.path.insert(0,'../3rdparty/LeapSDK/lib/x64')
sys.path.insert(0,'../3rdparty/LeapSDK/lib')
import Leap
from TXViewer import TXViewer

class SampleListener(Leap.Listener):
    aileron=0
    elevator=0
    throttle=0
    rudder=0
    lastAileron=0
    lastElevator=0
    lastThrottle=0
    lastRudder=0
    tx = TXViewer()

    #def __init__(self):
    #    super().__init__()
    
    def on_connect(self, controller):
        print("Connected")
        self.throttle=0 #set throttle to zero - I'm not having it fly away!
        self.lastThrottle=0
        
    def on_frame(self, controller):
        #print "Frame available"

        #NOTE: aileron, elevator, rudder are -1..+1, while throttle is 0..1
        self.aileron = 0.9*self.lastAileron #this is designed to back off the controls softly on loss of LeapMotion tracking
        self.elevator = 0.9*self.lastElevator
        self.rudder = 0.9*self.lastRudder
        self.throttle = 0.9*self.lastThrottle

        frame = controller.frame()
        #print ("Frame id: %d, timestamp: %d, hands: %d, fingers: %d" % (
        #    frame.id, frame.timestamp, len(frame.hands), len(frame.fingers)))
        hands = frame.hands
        if len(hands)==0:
            return #no hands to process
        #OK, so let's look at the first hand being tracked 
        hand = hands[0] #also hands.leftmost, rightmost frontmost
        if hand.confidence>0.9:
            if (len(frame.fingers) > 2): #if we're seeing less than two fingers, then assume the hand is a fist and don't read it
                #TODO: tidy this up - there's lots of stuff I'm not using - needs some real testing to see that it works!
                normal = hand.palm_normal #Leap.Vector
                direction = hand.direction #Leap.Vector
                handPitch = direction.pitch # * 180.0f / Mathf.PI;
                handRoll = normal.roll # * 180.0f / Mathf.PI; NOTE: 0.35=20 degrees, 1.22=70 degrees
                #handYaw = direction.Yaw # * 180.0f / Mathf.PI;
                palmPosition = hand.palm_position
                #palmVelocity = hand.palm_velocity #NOTE: this is a Leap.Vector not a Unity one
                #and on to the actual control mapping
                self.aileron = -handRoll
                self.elevator = handPitch
                #palm height seems to vary between 70 and 500 (mm from sensor?)
                self.throttle = (palmPosition.y-70)/500
                if self.throttle < 0:
                    self.throttle = 0
                elif self.throttle > 1:
                    self.throttle = 1
            #endif
        #endif

        #print("\033[1;1H Frame: a=%f e=%f t=%f" %(self.aileron,self.elevator,self.throttle) )
        self.tx.update(self.aileron,self.elevator,self.throttle,self.rudder)

        #remember last positions in case we lose tracking
        self.lastAileron = self.aileron
        self.lastElevator = self.elevator
        self.lastRudder = self.rudder
        self.lastThrottle = self.throttle

###

""" 
public const float confidenceThreshold = 0.9f;  
/// <summary>
    /// Poll function for current LeapMontion controller status.
    /// Return control inputs (aileron, elevator, rudder, throttle) based on latest hand position from the LeapMotion controller.
    /// This procedure contains all the logic to map the hand(s?) position to the control inputs.
    /// </summary>
    /// <param name="Aileron"></param>
    /// <param name="Elevator"></param>
    /// <param name="Rudder"></param>
    /// <param name="Throttle"></param>
    public static void GetControlInputs(out float Aileron, out float Elevator, out float Rudder, out float Throttle)
    {
        //NOTE: aileron, elevator, rudder are -1..+1, while throttle is 0..1
        Aileron = 0.9f*LastAileron; //this is designed to back off the controls softly on loss of LeapMotion tracking
        Elevator = 0.9f*LastElevator;
        Rudder = 0.9f*LastRudder;
        Throttle = 0.9f*LastThrottle;

        //use the values last seen by the leap listener event handler object
        //Debug.Log("confidence: " + leapListener.handConfidence);
        if (leapListener.handConfidence > confidenceThreshold)
        {
            if (leapListener.numFingers > 2) //if we're seeing less than two fingers, then assume the hand is a fist and don't read it
            {
                Aileron = -leapListener.handRoll;
                Elevator = leapListener.handPitch;
                //if (leapListener.palmVelocity.y>10)
                //{
                //    Throttle = 10;
                //}
                //else if (leapListener.palmVelocity.y<10)
                //{
                //    Throttle = -10;
                //}

                //palm height seems to vary between 70 and 500 (mm from sensor?)
                Throttle = (leapListener.palmPosition.y-70)/500;
                if (Throttle < 0) Throttle = 0;
                else if (Throttle > 1) Throttle = 1;
            }
        }
        //remember last positions in case we lose tracking
        LastAileron = Aileron;
        LastElevator = Elevator;
        LastRudder = Rudder;
        LastThrottle = Throttle;
    }
"""
###

def main():
    # Create a sample listener and controller
    listener = SampleListener()
    controller = Leap.Controller()
    
    # Have the sample listener receive events from the controller
    controller.add_listener(listener)
    
    # Keep this process running until Enter is pressed
    print("Press Enter to quit...")
    try:
        sys.stdin.readline()
    except KeyboardInterrupt:
        pass
    finally:
        # Remove the sample listener when done
        controller.remove_listener(listener)
        
if __name__ == "__main__":
    main()