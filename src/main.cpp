/*
Dit programma heeft controlle over een button, een bar display, een piezo buzzer en een DHT11
Het programma laat zien hoelang je vlees nog op de bbq moet liggen

Author: Billy de Graaf
Studentnummer: 1081483
Aangemaakt: 01-10-2019
Last edit: 15-10-2019
Versienummer: 1.8

*/

#include <avr/io.h>
#define __DELAY_BACKWARD_COMPATIBLE__ //om variabelen aan de delay te geven
#include <util/delay.h>
#include "dhtxx.h"  //DHT library van : https://github.com/efthymios-ks/AVR-DHT-C
#include <Arduino.h>

volatile int barNumber = 0; //bijhouden hoeveel bars de display heeft

//deze functie speelt een geluid af op basis van hertz en tijdVanNoot
//@params float tijdVanNoot = hoelang wordt de noot afgespeeld
//        float hertzFreq = frequentie van noot
//return: geen return value
void playSound(float tijdVanNoot, float hertzFreq){
  // inits
 long int i,cycles;
 float half_timer;
 float wavelength;


wavelength=(1/hertzFreq)*1000;
// hoelang 1 toon duurt
cycles=tijdVanNoot/wavelength;
 // aantal keer aan en uit
half_timer = wavelength/2;
 // tijd tussen uit en aan

//elke cycle in de forloop betekent een toggle op de speaker pin
for (i=0;i<cycles;i++)
 {
  _delay_ms(half_timer); //wacht de helft van de wave
   PORTB |= (1 << PB5); //speaker pin op 1

  _delay_ms(half_timer); //wacht de helft van de wave
   PORTB &= 0b11011111; // 1 means dont change, 0 means change
   PORTB |= (0 << PB5); //write 1 bit to PB0 without changing the rest of the register
 }
}

//functie die de bardisplay controlled en daarnaast geluiden aanroept bij bepaalde values
//@param barNumber is int die aangeeft hoeveel strepen op de bardisplay
//return: geen return value
void setBarDisplay(int barNumber){

  // PORTB = 0x00; //alles PORTB weer uit
  // PORTD = 0b00000111; //alles PORTD uit behalve waar de button zit, voor het pull up register
  //


		if(barNumber == 1)
      PORTB |= (1 << PB4);

		if(barNumber == 2)
		  PORTB |= (1 << PB4) | (1 << PB3);

		if(barNumber == 3)
		  PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2);

		if(barNumber == 4)
		  PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1);

		if(barNumber == 5)
		  PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);

		if(barNumber == 6){
      PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);
      PORTD |= (1 << PD7);
    }

		if(barNumber == 7){
  		PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);
      PORTD |= (1 << PD7) | (1 << PD6);
      playSound(200, 1000); //succes sound
      playSound(200, 1500);
    }

		if(barNumber == 8){ // error sound 1
      PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);
      PORTD |= (1 << PD7) | (1 << PD6) | (1 << PD5);
      playSound(200, 500); //error sound 1
      playSound(200, 300);
    }

		if(barNumber == 9){ //error sound 2
      PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);
      PORTD |= (1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4);
      playSound(200, 500); //error sound 2
      playSound(200, 300);
      playSound(200, 500);
      playSound(200, 300);
    }

		if(barNumber == 10){ //error sound heel lang
      PORTB |= (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);
      PORTD |= (1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4) | (1 << PD3);
      playSound(200, 500); //error sound 3, het vlees begint te verbranden!
      playSound(200, 300);
      playSound(200, 500);
      playSound(200, 300);
      playSound(200, 500);
      playSound(200, 300);
    }
}


//initiatie van interrupt op INT0
//geen params of return waarde
void initINT0Interrupt(void){
  // ISC1[1:0] op 01 genereerd interrupt op logical change INT0
  EICRA = (1 << ISC00);

  // Enable de external interrupt request op INT0
  EIMSK = (1 << INT0);

  // Zet interrupts in de Atmega aan
  sei();
}

//de functie die wordt uitgevoerd bij een interrupt op INT0
ISR(INT0_vect){
  // Bij button klik zet ik de waarde van barNumber op 0
  barNumber = 0;
  PORTB = 0x00; //alles PORTB weer uit
  PORTD = 0b00000111; //alles PORTD uit behalve waar de button zit, voor het pull up register
  Serial.println("interrupt");
  playSound(200, 1000); //succes sound
  playSound(200, 1500);
}


int main( )
{
  //initialisatie variabelen

	unsigned char ec; //Exit code
	int temp, humid; //Temperature and humidity
  float vleesGaarTemperatuur = 24; //graden waarop het vlees voor een x aantal minuten moet bakken
  int hoeLangMoetHetVleesBakken = 180; //3 minuten lang op x graden
  float gewensteLuchtvochtigheid = 45; //luchtvochtigheid

  float vleesGaarTemperatuurLow = vleesGaarTemperatuur - (0.05*vleesGaarTemperatuur); //temp mag een foutmarge hebben van 5%
  float vleesGaarTemperatuurHigh = vleesGaarTemperatuur + (0.05*vleesGaarTemperatuur);

  float gewensteLuchtvochtigheidLow = gewensteLuchtvochtigheid - (0.1*gewensteLuchtvochtigheid);
  float gewensteLuchtvochtigheidHigh = gewensteLuchtvochtigheid + (0.1*gewensteLuchtvochtigheid); //lucht mag een foutmarge hebben van 10%
  Serial.begin(9600);

  //initialisatie port en register
   DDRB = 0xFF;     //B port register als output
   DDRD = 0b111111000; //PINB 7,6,5,4,3 als output
   PORTB = 0x00;      //bit naar alle pins in B register, default state is alle leds uit
   PORTD = 0x00;      //bit naar alle pins in D register, default state is alle leds uit

   PORTD = (1 << PD2); //pull up resistor voor button op int0, oftewel PD2

   initINT0Interrupt();
	while( 1 )  //oneindige loop
	{
    Serial.println(barNumber);
		//Request DHT sensor to give it time to prepare data
		dhtxxconvert( DHTXX_DHT11, &PORTC, &DDRC, &PINC, ( 1 << 3 ) );

		_delay_ms( hoeLangMoetHetVleesBakken*1000 / 100 ); //elk 100ste van de gesette tijd wordt er gekeken naar de temperatuur

		//Read data from sensor to variables `temp` and `humid` (`ec` is exit code)
		ec = dhtxxread( DHTXX_DHT11, &PORTC, &DDRC, &PINC, ( 1 << 3 ), &temp, &humid );
    Serial.println("temperatuur is: ");
    Serial.println( temp/10);
    Serial.println("luchtvochtigheid is: ");
    Serial.println(humid/10);
    switch(ec){ //switch op de EC, oftewel is het een error of een goede value
      case(0): //DHTXX_ERROR_OK oftewel geen error
        //check of de temp en luchtvochtigheid nog goed gaan
        if(temp/10 >= vleesGaarTemperatuurLow && temp/10 <= vleesGaarTemperatuurHigh && humid/10 >= gewensteLuchtvochtigheidLow && humid/10 <= gewensteLuchtvochtigheidHigh){
          barNumber++; //+1 bij bar elke keer als het de goede temp en humid is
          setBarDisplay(barNumber);
        }else if(temp/10 > vleesGaarTemperatuurHigh || humid/10 > gewensteLuchtvochtigheidHigh){ // als de temp heel hoog is gaart het vlees sneller, hetzelfde met de humidity
          barNumber =+ 3; //vlees gaat sneller garen
          setBarDisplay(barNumber);
          playSound(200, 500); //alsnog een error sound, te hoge temp
          playSound(200, 300);
        }else{
          barNumber--; //vlees is niet aan het garen
          setBarDisplay(barNumber);
          playSound(200, 500); //error sound
          playSound(200, 300);
        }
        break;
      case(1): //DHTXX_ERROR_COMM oftewel communicatie error
        barNumber--; //vlees is niet aan het garen
        setBarDisplay(barNumber);
        playSound(200, 500); //error sound
        playSound(200, 300);
        break;
      case(2): //DHTXX_ERROR_CHECKSUM oftewel checksum error
        barNumber--; //vlees is niet aan het garen
        setBarDisplay(barNumber);
        playSound(200, 500); //error sound
        playSound(200, 300);
        break;
      case(3): //DHTXX_ERROR_OTHER error, maar niet één van de bovenstaande
        barNumber--; //vlees is niet aan het garen
        setBarDisplay(barNumber);
        playSound(200, 500); //error sound
        playSound(200, 300);
        break;
    }
	}

	return 0;
}
