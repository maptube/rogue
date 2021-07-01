//#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#include "public.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "vjoyinterface.h"
#include "Math.h"

#include "Leap.h"
const float RAD_TO_DEG = 180.0 / 3.141592654;

// Default device ID (Used when ID not specified)
#define DEV_ID		1

// Prototypes
// ????
JOYSTICK_POSITION_V2 iReport; // The structure that holds the full position data
Leap::Controller leapController; //this is the Leap Motion controller
int64_t lastFrameID = 0; //used by leap hand polling

float aileron = 0, elevator = 0, rudder = 0;

void pollLeapHand() {
	//this is a polling routine for getting data from the Leap Motion
	//POST: sets aileron and elevator from the Leap Hand
	if (leapController.isConnected())
	{
		Leap::Frame frame = leapController.frame(); //The latest frame
		//Leap::Frame previous = leapController.frame(1); //The previous frame

		if (frame.id() == lastFrameID) return; //no new frames, so exit
		
		Leap::HandList hands = frame.hands();
		//Leap::PointableList pointables = frame.pointables();
		//Leap::FingerList fingers = frame.fingers();
		//Leap::ToolList tools = frame.tools();

		aileron = 0;
		elevator = 0;
		rudder = 0;
		//todo: track the hand id so it's always the same hand and add some silly range checking wraparound
		for (auto itHand = hands.begin(); itHand!=hands.end(); ++itHand )
		{
			Leap::Hand hand = *itHand;
			//if (hand.confidence() > 0.75f)
			//{
				const Leap::Vector palm = hand.palmNormal();
				const Leap::Vector direction = hand.direction();
				aileron = -palm.roll() * RAD_TO_DEG; //rads!
				//elevator = 3.14/2.0 + palm.pitch();
				elevator = direction.pitch() * RAD_TO_DEG;
				rudder = direction.yaw() * RAD_TO_DEG;
				//debug
				//printf("raw a=%f \t e=%f\n", aileron, elevator);
				//rotation
				//greater than 1.5?
				//mapping
				aileron = aileron / 45.0;
				elevator = elevator / 45.0;
				//hard limits
				if (aileron < -1.0) aileron = -1.0;
				else if (aileron > 1.0) aileron = 1.0;
				if (elevator < -1.0) elevator = -1.0;
				else if (elevator > 1.0) elevator = 1.0;
				//debug
				//printf("a=%f \t e=%f\n", aileron, elevator);
			//}
		}

		//update frame id tracker
		lastFrameID = frame.id();
	}

}

int main() {
	UINT DevID = DEV_ID; //id of the vjoy device 1,2,3 etc from the vjoy info panel
	int stat = 0; //device status
	USHORT X = 0;
	USHORT Y = 0;
	USHORT Z = 0;
	LONG   Btns = 0;

	BYTE id = 1;
	UINT iInterface = 1;
	PVOID pPositionMessage;

	//leap setup
	//we need to set up background frames, but this can be denied on the control panel
	//we could override though: https://developer-archive.leapmotion.com/documentation/cpp/devguide/Leap_Policies.html
	leapController.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

    // Get the driver attributes (Vendor ID, Product ID, Version Number)
    if (!vJoyEnabled())
    {
        wprintf(L"Failed Getting vJoy attributes.\n");
        return -2;
    }
    else
    {
        wprintf(
            L"Vendor: %s\nProduct :%s\nVersion Number:%s\n",
            static_cast<TCHAR*> (GetvJoyManufacturerString()),
            static_cast<TCHAR*>(GetvJoyProductString()),
            static_cast<TCHAR*>(GetvJoySerialNumberString())
        );
    };

	// Get the status of the vJoy device before trying to acquire it
	VjdStat status = GetVJDStatus(DevID);

	switch (status)
	{
	case VJD_STAT_OWN:
		printf("vJoy device %d is already owned by this feeder\n", DevID);
		break;
	case VJD_STAT_FREE:
		printf("vJoy device %d is free\n", DevID);
		break;
	case VJD_STAT_BUSY:
		printf("vJoy device %d is already owned by another feeder\nCannot continue\n", DevID);
		return -3;
	case VJD_STAT_MISS:
		printf("vJoy device %d is not installed or disabled\nCannot continue\n", DevID);
		return -4;
	default:
		printf("vJoy device %d general error\nCannot continue\n", DevID);
		return -1;
	};

	// Acquire the vJoy device
	if (!AcquireVJD(DevID))
	{
		printf("Failed to acquire vJoy device number %d.\n", DevID);
		int dummy = getchar();
		stat = -1;
		RelinquishVJD(DevID);
		return 0;
	}
	else
		printf("Acquired device number %d - OK\n", DevID);

	// Start endless loop
	// The loop injects position data to the vJoy device
	// If it fails it let's the user try again  
	while (1)
	{
		pollLeapHand(); //poll the Leap device to set aileron, elevator

		//the following is to send data to the joystick via vJoy

		// Set destination vJoy device
		id = (BYTE)DevID;
		iReport.bDevice = id;

		// Set position data of 3 first axes
		if (Z > 35000) Z = 0;
		Z += 200;
		//iReport.wAxisZ = Z;
		//iReport.wAxisX = 32000 - Z;
		//iReport.wAxisY = Z / 2 + 7000;
		//
		//leap joystick
		iReport.wAxisZ = 0;
		iReport.wAxisX = (unsigned int)(16384.0f + aileron * 16384.0f);
		iReport.wAxisY = (unsigned int)(16384.0f - elevator * 16384.0f); //NOTE reverse

		// Set position data of first 8 buttons
		Btns = 1 << (Z / 4000);
		iReport.lButtons = Btns;

		// Send position data to vJoy device
		pPositionMessage = (PVOID)(&iReport);
		if (!UpdateVJD(DevID, pPositionMessage))
		{
			printf("Feeding vJoy device number %d failed - try to enable device then press enter\n", DevID);
			getchar();
			AcquireVJD(DevID);
		}
		Sleep(2);
	}

	//UNREACHABLE???
	//finish up by releasing the device
	RelinquishVJD(DevID);
	return 0;
};