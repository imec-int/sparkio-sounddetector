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
void sendStateOverTcp(int state);
void postState(int state);
void readIncommingHttpData();
void monitorIncommingHTTPConnections();


/* Variables -----------------------------------------------------------------*/
bool useTCP = false;
bool useHTTP = true;
bool enableHTTPserver = true;

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

// TCP:
TCPClient tcpclient;
IPAddress tcpServer(10,100,11,7);
int tcpPort = 8000;

// HTTP:
TCPClient httpclient;
IPAddress httpServer(10,100,11,7);
int httpPort = 3000;
uint8_t *responseBuffer;

const char *actionSound   = "POST /rest/soundstate HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 12\r\n\r\naction=sound\r\n\0";
const char *actionNoSound = "POST /rest/soundstate HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 14\r\n\r\naction=nosound\r\n\0";
const char *actionStartup = "POST /rest/soundstate HTTP/1.1\r\nHost: somehost\r\nConnection: keep-alive\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 14\r\n\r\naction=startup\r\n\0";


// HTTP Server
TCPServer server(80);
const char *rootPage = "HTTP/1.1 200 OK\r\nServer: samserver/0.0.1\r\nContent-Type: text/html\r\nContent-Length: 885\r\nConnection: close\r\n\r\n<!DOCTYPE html><html lang=\"nl\"><head><title>MiX Sound detector</title><meta charset=\"utf-8\"><link rel=\"stylesheet\" type=\"text/css\" href=\"//netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css\"><script src=\"//ajax.googleapis.com/ajax/libs/jquery/1.11.0/jquery.min.js\"></script></head><body><div class=\"container\"><h1>MiX Sound detector</h1><form class=\"form-horizontal\" role=\"form\"><div class=\"form-group\"><label for=\"inputthreshold\" class=\"col-sm-2 control-label\">input threshold (mV)</label><div class=\"col-sm-10\"><input type=\"text\" class=\"form-control\" id=\"inputthreshold\" placeholder=\"eg 2100\"></div></div><div class=\"form-group\"><label for=\"sensitivyThreshold\" class=\"col-sm-2 control-label\">sensitivy threshold (ticks)</label><div class=\"col-sm-10\"><input type=\"text\" class=\"form-control\" id=\"sensitivyThreshold\" placeholder=\"eg 3\"></div></div></form></div></body></html>\0";

bool debug = true;

/* This function is called once at start up ----------------------------------*/
void setup()
{
	pinMode(ledPin, OUTPUT);
	pinMode(soundPin, INPUT);
	pinMode(buttonPin, INPUT);

	Serial.begin(9600);

	responseBuffer = new uint8_t[1024];

	updateState(-1);

	server.begin();

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

	if( useHTTP )
	{
		readIncommingHttpData();
	}

	if( enableHTTPserver )
	{
		monitorIncommingHTTPConnections();
	}


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

		if( useTCP == true)
		{
			sendStateOverTcp(state);
		}

		if( useHTTP == true )
		{
			postState(state);
		}
	}
}

void sendStateOverTcp(int state)
{
	if(debug) Serial.println("Sending state over TCP");

	if( !tcpclient.connected() )
	{
		if(debug) Serial.println("TCP was not connected. Connecting...");
		tcpclient.connect(tcpServer, tcpPort);
	}

	if( !tcpclient.connected() )
	{
		if(debug) Serial.println("TCP still not connected. Trying a second time...");
		tcpclient.connect(tcpServer, tcpPort);
	}

	if( !tcpclient.connected() )
	{
		if(debug) Serial.println("TCP could not connect.");
		return;
	}

	if(debug) Serial.println("TCP connected. Sending state.");

	if(state == 1)
	{
		tcpclient.print("sound\0");
	}
	if(state == 0)
	{
		tcpclient.print("nosound\0");
	}
	if(state == -1)
	{
		tcpclient.print("startup\0");
	}

	tcpclient.flush();

	if(debug) Serial.println("TCP flushed.");
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

void monitorIncommingHTTPConnections()
{

	// HTTP Server:
	while(true)
	{
		// loop over clients:
		TCPClient client = server.available();

		if(!client) break; //stop loop

		if(!client.connected()) continue;

		if(!client.available()) continue;

		uint8_t buffer[1024] = {0};

		if(debug) Serial.println("Server: reading first 1023 bytes");

		client.read(buffer, 1023);

		if(debug) Serial.println("Server: reading the rest");

		while( client.available() )
		{
			client.read();
		}

		if(debug) Serial.println((char*)buffer);

		if (strncmp((char*)buffer, "GET / HTTP/", 11) == 0)
		{
			if(debug) Serial.println("receiving http / request... responding with root page");
			client.print(rootPage);
			client.flush();
			// give the client time to receive the data
			delay(10);
		}

		// close the connection:
		if(debug) Serial.println("Server: closing connection with browser");
		client.stop();
	}
}


