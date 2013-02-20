/*
 Copyright (C) 2012 Sebastien Jean <baz dot jean at gmail dot com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the version 3 GNU General Public License as
 published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <Arduino.h>
#include <FSK600BaudTA900TB1500Mod.h>
#include <defs.h>

#define SENSOR_STRING_LENGTH 40

#define KIWI_FRAME_LENGTH 11

// FSK modulator
FSK600BaudTA900TB1500Mod fskMod(FSK_MOD_TX);

// sensor data, as ASCII
char sensorString[SENSOR_STRING_LENGTH];

// absolute pressure sensor value
int absValue = 0;

// differential pressure sensor value
int diffValue = 0;

// internal temperature sensor value
int tempInValue = 0;

// external temperature sensor value
int tempOutValue = 0;

// battery voltage sensor value
int voltageValue = 0;

// Kiwi (Planete-Sciences/ CNES format) Frame
unsigned char kiwiFrame[KIWI_FRAME_LENGTH];

/**
 * Application's main (what else to say?)
 * @return (never)
 */
int main(void) {
	init();

	setup();

	for (;;)
		loop();

	return 0;
}

/**
 * Initializes LEDs wirings
 */
void initLEDs()
{
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_ORANGE, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
}

/**
 * Initializes SD shield
 */


/**
 * Initializes User switch
 */
void initUserButton()
{
	pinMode(USER_BUTTON, INPUT);
	digitalWrite(USER_BUTTON, HIGH);
}

/**
 * Initializes serial debug communication
 */
void initDebug()
{
	Serial.begin(600);
}


/**
 * Resets KIWI frame contents
 */
void resetKiwiFrame()
{
	for (int i = 0; i < 11; i++)
		kiwiFrame[i] = 0x00;
	kiwiFrame[0] = 0xFF;
}

/**
 * Plays LEDs startup sequence
 */
void showLEDsStartupSequence()
{
	digitalWrite(LED_RED, HIGH);
	digitalWrite(LED_ORANGE, HIGH);
	digitalWrite(LED_GREEN, HIGH);
	digitalWrite(LED_BLUE, HIGH);
	delay(1000);
	digitalWrite(LED_RED, LOW);
	digitalWrite(LED_ORANGE, LOW);
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, LOW);
}

/**
 * Displays status (OK/KO) using red/green LEDs
 */
void showStatus(int status)
{
	if (status)
	{
		digitalWrite(LED_GREEN,HIGH);
		digitalWrite(LED_RED,LOW);
	}
	else
	{
		digitalWrite(LED_GREEN,LOW);
		digitalWrite(LED_RED,HIGH);
	}
}

void quicklyMakeSomeLedBlinkSeveralTimes(int led, int times)
{
	for (int i=0;i<times;i++)
	{
		digitalWrite(led, HIGH);
		delay(100);
		digitalWrite(led, LOW);
		delay(100);
	}
}



/**
 * Arduino's setup function, called once at startup, after init
 */
void setup()
{
	initLEDs();

	showLEDsStartupSequence();

	initUserButton();

	initDebug();


	resetKiwiFrame();

	// wdt_enable(WDTO_8S);
}

/**
 * Arduino's loop function, called in loop (incredible, isn't it ?)
 */
void loop() {
	int sensorStringOffset = 0;
	unsigned char chk = 0;

	// millis since last reset processing
	itoa(millis() / 1000, sensorString, 10);
	sensorStringOffset = strlen(sensorString);
	sensorString[sensorStringOffset++] = ',';

	// absolute pressure processing
	absValue = analogRead(ABS_P);
	itoa(absValue, sensorString + sensorStringOffset, 10);
	sensorStringOffset = strlen(sensorString);
	sensorString[sensorStringOffset++] = ',';
	kiwiFrame[1] = (unsigned char) (absValue / 4);
	if (kiwiFrame[1] == 0xFF)
		kiwiFrame[1] = 0xFE;

	// differential pressure processing
	diffValue = analogRead(DIFF_P);
	itoa(diffValue, sensorString + sensorStringOffset, 10);
	sensorStringOffset = strlen(sensorString);
	sensorString[sensorStringOffset++] = ',';
	kiwiFrame[2] = (unsigned char) (diffValue / 4);
	if (kiwiFrame[2] == 0xFF)
		kiwiFrame[2] = 0xFE;

	// internal temperature pressure processing
	tempInValue = analogRead(TEMPIN);
	itoa(tempInValue, sensorString + sensorStringOffset, 10);
	sensorStringOffset = strlen(sensorString);
	sensorString[sensorStringOffset++] = ',';
	kiwiFrame[3] = (unsigned char) (tempInValue / 4);
	if (kiwiFrame[3] == 0xFF)
		kiwiFrame[3] = 0xFE;

	// external temperature pressure processing
	tempOutValue = analogRead(TEMPOUT);
	itoa(tempOutValue, sensorString + sensorStringOffset, 10);
	sensorStringOffset = strlen(sensorString);
	sensorString[sensorStringOffset++] = ',';
	kiwiFrame[4] = (unsigned char) (tempOutValue / 4);
	if (kiwiFrame[4] == 0xFF)
		kiwiFrame[4] = 0xFE;

	// battery voltage processing
	voltageValue = analogRead(VOLTAGE);
	itoa(voltageValue, sensorString + sensorStringOffset, 10);
	sensorStringOffset = strlen(sensorString);
	sensorString[sensorStringOffset++] = '\r';
	sensorString[sensorStringOffset++] = '\n';
	kiwiFrame[5] = (unsigned char) (voltageValue / 4);
	if (kiwiFrame[5] == 0xFF)
		kiwiFrame[5] = 0xFE;
	kiwiFrame[9] = (unsigned char) (voltageValue / 8);

	for (int cpt = 1; cpt < KIWI_FRAME_LENGTH - 1; cpt++)
		chk = (unsigned char) ((chk + kiwiFrame[cpt]) % 256);

	chk = (unsigned char) (chk / 2);
	kiwiFrame[KIWI_FRAME_LENGTH] = chk;

	// Kiwi Frame transmission
	for (int cpt = 0; cpt < KIWI_FRAME_LENGTH; cpt++)
		fskMod.write(kiwiFrame[cpt]);
	fskMod.off();

		//wdt_reset();

		// Sensor data processing

		// Debug
	Serial.print(sensorString);

	// Transmission
	for (int cpt = 0; cpt < strlen(sensorString); cpt++)
		fskMod.write(sensorString[cpt]);
	fskMod.off();
}

