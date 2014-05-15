/**
 ******************************************************************************
 * @file    application.cpp
 * @authors  Sam Decrock
 *
 * Detect sound and print its value to the serial port
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "application.h"

/* Function prototypes -------------------------------------------------------*/


/* Variables -----------------------------------------------------------------*/
int ledPin = D0;
int ledPin2 = D1;
int soundPin = A0;
int currentSoundValue;
int highestSoundValue = 0;
int counter = 0;
int buttonPin = D2;

bool buttonState = 0;
bool buttonPressed = false;


/* This function is called once at start up ----------------------------------*/
void setup()
{
	pinMode(ledPin, OUTPUT);
	pinMode(soundPin, INPUT);
	pinMode(buttonPin, INPUT);

	Serial.begin(9600);

	digitalWrite(ledPin2, 1);
}

/* This function loops forever (every 5ms) ------------------------------------*/
void loop()
{
	// BUTTON TEST
	buttonState = digitalRead(buttonPin);

	if(buttonState == 1 && buttonPressed == false)
	{
		buttonPressed = true;

		Serial.println("Button in");
	}

	if(buttonPressed == true && buttonState == 0)
	{
		buttonPressed = false;

		Serial.println("Button out");
	}


	// LISTEN FOR SOUND:
	currentSoundValue = analogRead(soundPin);
	if(currentSoundValue > highestSoundValue)
	{
		highestSoundValue = currentSoundValue;
	}

	// only print the highest value after 200 ticks:
	if(counter == 200)
	{
		Serial.println(highestSoundValue);
		counter = 0;
		highestSoundValue = 0;
	}

	counter++;


	delay(5); // wait an extra 5ms, that's 10ms between loops

	// indicates machine has booted:
	digitalWrite(ledPin2, 0);
}
