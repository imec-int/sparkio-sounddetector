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

#define  _BUFFERSIZE 200

/* Function prototypes -------------------------------------------------------*/
void updateState(int state);
void sendStateOverTcp(int state);
void postState(int state);


/* Variables -----------------------------------------------------------------*/
bool useTCP = false;
bool useHTTP = true;

int ledPin = D0;
int ledPin2 = D1;
int soundPin = A0;
int currentSoundValue;
int buttonPin = D2;

int buttonState = 0;
bool buttonPressed = false;
int threshold = 2200; //in mV
int capacity = 0;
int thresholdCapacity = 0;
int currentSoundState = 0;

// TCP:
TCPClient tcpclient;
IPAddress tcpServer(10,100,11,7);
int tcpPort = 8000;

// HTTP:
TCPClient httpclient;
IPAddress httpServer(10,100,11,7);
int httpPort = 3000;
uint8_t *responseBuffer;

const char *actionSound   = "POST /rest/soundstate HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 12\r\n\r\naction=sound\r\n";   //171
const char *actionNoSound = "POST /rest/soundstate HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 14\r\n\r\naction=nosound\r\n"; //173
const char *actionStartup = "POST /rest/soundstate HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 14\r\n\r\naction=startup\r\n"; //173

bool debug = false;

/* This function is called once at start up ----------------------------------*/
void setup()
{
	pinMode(ledPin, OUTPUT);
	pinMode(soundPin, INPUT);
	pinMode(buttonPin, INPUT);

	Serial.begin(9600);

	responseBuffer = new uint8_t[2048];

	updateState(-1);

	digitalWrite(ledPin2, 1);
}

/* This function loops forever (every 5ms) ------------------------------------*/
void loop()
{
	// SIMULATE SOUND USING BUTTON:
	buttonState = digitalRead(buttonPin);

	if(buttonState == 1 && buttonPressed == false){
		buttonPressed = true;

		updateState(1);
	}

	if(buttonPressed == true && buttonState == 0){
		buttonPressed = false;

		updateState(0);
	}


	// LISTEN FOR SOUND:
	if(!buttonPressed){ // dont interfere with the testbutton

		currentSoundValue = analogRead(soundPin);

		if(currentSoundValue > threshold){
			thresholdCapacity++;

			if(thresholdCapacity >= 3){
				capacity = 200; //2 seconds
				thresholdCapacity = 0;

				updateState(1);
			}
		}else{
			capacity--;
			if(capacity <= 0 ){
				// happens after capacity*(delay+5 ms):
				capacity = 0; //cap

				updateState(0);
			}

			thresholdCapacity--;
			if(thresholdCapacity <= 0){
				thresholdCapacity = 0;
			}
		}
	}

	delay(5); // wait an extra 5ms, that's 10ms between loops
	digitalWrite(ledPin2, 0);
}

void updateState(int state) {
	digitalWrite(ledPin, state); // turn LED on/off

	// only send state if different from previous:
	if(currentSoundState != state){
		currentSoundState = state;

		if( useTCP == true) {
			sendStateOverTcp(state);
		}

		if( useHTTP == true ){
			postState(state);
		}
	}
}

void sendStateOverTcp(int state) {
	if(debug) Serial.println("Sending state over TCP");

	if( !tcpclient.connected() ){
		if(debug) Serial.println("TCP was not connected. Connecting...");
		tcpclient.connect(tcpServer, tcpPort);
	}

	if( !tcpclient.connected() ){
		if(debug) Serial.println("TCP could not connect.");
		return;
	}

	if(debug) Serial.println("TCP connected. Sending state.");

	if(state == 1){
		tcpclient.print("sound");
	}
	if(state == 0){
		tcpclient.print("nosound");
	}
	if(state == -1){
		tcpclient.print("startup");
	}

	tcpclient.flush();

	if(debug) Serial.println("TCP flushed.");
}

void postState(int state) {
	if(debug) Serial.println("Sending state over HTTP POST");


	if( !httpclient.connected() ){
		if(debug) Serial.println("HTTP was not connected. Connecting...");
		httpclient.connect(httpServer, httpPort);
	}

	if( !httpclient.connected() ){
		if(debug) Serial.println("HTTP could not connect.");
		return;
	}

	if(debug) Serial.println("HTTP connected. Sending state.");

	// httpclient.print(postHeader);

	if(state == 1){
		httpclient.print(actionSound);
	}

	if(state == 0){
		httpclient.print(actionNoSound);
	}

	if(state == -1){
		httpclient.print(actionStartup);
	}

	// flush, so the buffer is clear to read response:
	httpclient.flush();
	if(debug) Serial.println("HTTP flushed.");


	// READ RESPONSE
	httpclient.read(responseBuffer, 2048);

	if(debug) Serial.println((char*)responseBuffer);
	if(debug) Serial.println();


	// httpclient.stop(); //break connection (not necessary, server will break connection depending on 'Connection' header)
}
