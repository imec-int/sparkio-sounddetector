/**
 ******************************************************************************
 * @file    application.cpp
 * @authors  Sam Decrock
 *
 * Detect sound over a sliding window
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "application.h"

/* Function prototypes -------------------------------------------------------*/
void updateState(int state);
void postState(int state);
void readIncommingHttpData();


/* Variables -----------------------------------------------------------------*/
int ledPin = D0;
int ledPin2 = D1;
int soundPin = A0;
int currentSoundValue;
int buttonPin = D2;

bool buttonState = 0;
bool buttonPressed = false;
int threshold = 2200; //in mV
int capacity = 0;
int thresholdCapacity = 0;
int currentSoundState = 0;

// HTTP:
TCPClient httpclient;
IPAddress httpServer(10,100,11,7); // Sam
// IPAddress httpServer(10,100,11,216); // Dorthe
int httpPort = 8090;
uint8_t *responseBuffer;

const char *actionSound   = "PUT /message/putit HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: 75\r\n\r\n{\n\t\"MicStateChangedInRunningMeeting\": {\n\t\t\"SeatNr\": 0,\n\t\t\"State\": \"On\"\n\t}\n}";
const char *actionNoSound = "PUT /message/putit HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: 76\r\n\r\n{\n\t\"MicStateChangedInRunningMeeting\": {\n\t\t\"SeatNr\": 0,\n\t\t\"State\": \"Off\"\n\t}\n}";
const char *actionStartup = "PUT /message/putit HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: 37\r\n\r\n{\"sounddetector\":{\"state\":\"booting\"}}";

bool debug = false;

/* This function is called once at start up ----------------------------------*/
void setup()
{
	pinMode(ledPin, OUTPUT);
	pinMode(soundPin, INPUT);
	pinMode(buttonPin, INPUT);

	Serial.begin(9600);

	responseBuffer = new uint8_t[1024];

	updateState(-1);

	digitalWrite(ledPin2, 1);
}

/* This function loops forever (every 5ms) ------------------------------------*/
void loop()
{
	// SIMULATE SOUND USING BUTTON:
	buttonState = digitalRead(buttonPin);

	if(buttonState == 1 && buttonPressed == false)
	{
		buttonPressed = true;

		updateState(1);
	}

	if(buttonPressed == true && buttonState == 0)
	{
		buttonPressed = false;

		updateState(0);
	}


	// LISTEN FOR SOUND:
	if(!buttonPressed) // dont interfere with the testbutton
	{
		currentSoundValue = analogRead(soundPin);

		if(currentSoundValue > threshold)
		{
			thresholdCapacity++;

			if(thresholdCapacity >= 3)
			{
				capacity = 200; //2 seconds
				thresholdCapacity = 0;

				updateState(1);
			}
		}
		else
		{
			capacity--;
			if(capacity <= 0 ){
				// happens after capacity*(delay+5 ms):
				capacity = 0; //cap

				updateState(0);
			}

			thresholdCapacity--;
			if(thresholdCapacity <= 0)
			{
				thresholdCapacity = 0;
			}
		}
	}


	readIncommingHttpData();

	delay(5); // wait an extra 5ms, that's 10ms between loops

	// indicates machine has bootet:
	digitalWrite(ledPin2, 0);
}

void updateState(int state)
{
	digitalWrite(ledPin, state); // turn LED on/off

	// only send state if different from previous:
	if(currentSoundState != state)
	{
		currentSoundState = state;

		postState(state);
	}
}


void postState(int state) {
	if(debug) Serial.println("Sending state over HTTP POST");


	if( !httpclient.connected() )
	{
		if(debug) Serial.println("HTTP was not connected. Connecting...");
		httpclient.connect(httpServer, httpPort);
	}

	if( !httpclient.connected() )
	{
		if(debug) Serial.println("HTTP still not connected. Trying a second time...");
		httpclient.connect(httpServer, httpPort);
	}

	if( !httpclient.connected() )
	{
		if(debug) Serial.println("HTTP could not connect.");
		return;
	}

	if(debug) Serial.println("HTTP connected. Sending state.");


	if(state == 1)
	{
		httpclient.print(actionSound);
	}

	if(state == 0)
	{
		httpclient.print(actionNoSound);
	}

	if(state == -1)
	{
		httpclient.print(actionStartup);
	}

	// flush, so the buffer is clear to read response:
	httpclient.flush();

	if(debug) Serial.println("> HTTP request sent:");

	// httpclient.stop(); //break connection (not necessary, server will break connection depending on 'Connection' header)
}

void readIncommingHttpData()
{
	// READ RESPONSE
	if( httpclient.available() )
	{
		// 1. fill the buffer with zeros
		memset(responseBuffer, 0x00, 1024);

		// 2. read the first 1023 bytes
		httpclient.read(responseBuffer, 1023);

		// 3. read the rest of the socket buffer so that it's empty
		while(httpclient.available())
		{
			httpclient.read();
		}

		if(responseBuffer[0] != 0x00)
		{
			if(debug) Serial.println("> incomming HTTP response:");
			if(debug) Serial.println();
			if(debug) Serial.println((char*)responseBuffer);
			if(debug) Serial.println();
		}
	}
}
