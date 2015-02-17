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
void updateState(int pin, int state);
void postState(int pin, int state);
void readIncommingHttpData();


/* Variables -----------------------------------------------------------------*/
const int ledPin = D0;
const int ledPin2 = D1;
const int soundPins[] = {A0, A1, A2, A3, A4, A5, A6, A7};

const int pinCount = 8;
int soundValues[pinCount] = {0};

int threshold = 2200; //in mV
int capacity[pinCount] = {0};
int thresholdCapacity[pinCount] = {0};
int currentSoundState[pinCount] = {0};

// HTTP:
TCPClient httpclient;
IPAddress httpServer(10,100,11,7); // Sam
// IPAddress httpServer(10,100,11,216); // Dorthe
int httpPort = 8090;

uint8_t responseBuffer[1024];

char actionSound[pinCount][230];
char actionNoSound[pinCount][231];

bool debug = false;

/* This function is called once at start up ----------------------------------*/
void setup()
{
	pinMode(ledPin, OUTPUT);
	pinMode(ledPin2, OUTPUT);

	for (int pin = 0; pin < pinCount; ++pin)
	{
		// register every pin as input:
		pinMode(soundPins[pin], INPUT);

		// create http post strings:
		sprintf(actionSound[pin], "PUT /message/putit HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: 75\r\n\r\n{\n\t\"MicStateChangedInRunningMeeting\": {\n\t\t\"SeatNr\": %d,\n\t\t\"State\": \"On\"\n\t}\n}", pin);
		sprintf(actionNoSound[pin], "PUT /message/putit HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/json\r\nContent-Length: 76\r\n\r\n{\n\t\"MicStateChangedInRunningMeeting\": {\n\t\t\"SeatNr\": %d,\n\t\t\"State\": \"Off\"\n\t}\n}", pin);
	}


	if(debug) Serial.begin(9600);

	// indicates machine is booting:
	digitalWrite(ledPin2, 1);
}

/* This function loops forever (every 5ms) ------------------------------------*/
void loop()
{
	for (int pin = 0; pin < pinCount; ++pin)
	{
		soundValues[pin] = analogRead(soundPins[pin]);

		if(soundValues[pin] > threshold)
		{
			thresholdCapacity[pin]++;

			if(thresholdCapacity[pin] >= 3)
			{
				capacity[pin] = 200; //2 seconds
				thresholdCapacity[pin] = 0;

				updateState(pin, 1);
			}
		}
		else
		{
			capacity[pin]--;
			if(capacity[pin] <= 0 ){
				// happens after capacity*(delay+5 ms):
				capacity[pin] = 0; //cap

				updateState(pin, 0);
			}

			thresholdCapacity[pin]--;
			if(thresholdCapacity[pin] <= 0)
			{
				thresholdCapacity[pin] = 0;
			}
		}
	}

	readIncommingHttpData(); // so that the socket doesn't block everything

	delay(5); // wait an extra 5ms, that's 10ms between loops

	// indicates machine has booted:
	digitalWrite(ledPin2, 0);
}

void updateState(int pin, int state)
{
	// only send state if different from previous:
	if(currentSoundState[pin] != state)
	{
		currentSoundState[pin] = state;

		digitalWrite(ledPin, state); // turn LED on/off

		postState(pin, state);
	}
}


void postState(int pin, int state) {
	if(debug) Serial.println("> Sending state over HTTP POST");
	if(debug) Serial.print("> pin: ");
	if(debug) Serial.print(pin);
	if(debug) Serial.print(" | state: ");
	if(debug) Serial.println(state);


	if( !httpclient.connected() )
	{
		if(debug) Serial.println("> HTTP was not connected. Connecting...");
		httpclient.connect(httpServer, httpPort);
	}

	if( !httpclient.connected() )
	{
		if(debug) Serial.println("> HTTP still not connected. Trying a second time...");
		httpclient.connect(httpServer, httpPort);
	}

	if( !httpclient.connected() )
	{
		if(debug) Serial.println("> HTTP could not connect.");
		return;
	}

	if(debug) Serial.println("> HTTP connected. Sending state.");


	if(state == 1)
	{
		httpclient.print(actionSound[pin]);
	}

	if(state == 0)
	{
		httpclient.print(actionNoSound[pin]);
	}


	// flush, so the buffer is clear to read response:
	httpclient.flush();

	if(debug) Serial.println("> HTTP request sent.");

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
