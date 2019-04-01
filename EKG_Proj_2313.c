/*
 * EKG_Proj_2313.c
 *
 * Created: 22.04.2012 17:28:47
 *  Author: Joachim KLug
 */ 

#include "main.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

//Definitions
#define dp PB0
#define c PB1
#define d PB2
#define e PB3
#define f PB4
#define b PB5
#define g PB6
#define a PB7
#define init_runs 5                                                                               
#define sleeptime 50
#define max_prog 7                                                                              
#define UPPPER_BOUND 10
#define RAND_MAX 110
#define BLINK_LICHT PB0

//Normal Sinus
const uint8_t PROGMEM SINUS_ARRAY[118] =
{54, 60, 64, 67, 75, 75, 75, 75, 67, 64, 60, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 30, 
 30, 60, 120, 170, 171, 171, 170, 120, 43, 30, 20, 20, 25, 35, 41, 45, 49, 52, 54, 54,
 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 
 54, 54, 54, 54, 59, 63, 66, 70, 74, 77, 80, 82, 84, 85, 85, 85, 85, 84, 82, 80, 77, 74,
 70, 65, 59, 54, 54, 54, 54, 54, 54, 54, 54, 54};
 
//Schrittmacher
const uint8_t PROGMEM SCHRITT_ARRAY[171] =
{104, 106, 106, 110, 110, 90, 80, 65, 60, 60, 65, 90, 115, 127, 130, 160, 160, 160, 150, 140, 130, 120,	
 104, 104, 85, 70, 45, 30, 30, 30, 28, 27, 8, 7, 7, 7, 7, 7, 7, 7, 8, 8, 9, 20, 30, 40, 50, 60, 60, 70,	
 80, 90, 90, 100, 110, 120, 130, 140, 150, 155, 160, 160, 164, 168, 168, 169, 169, 173, 173, 173, 173, 
 175, 180, 185, 185, 190, 195, 203, 205, 209, 211, 215, 211, 211, 208, 205, 200, 195, 190, 190, 180, 170, 
 170, 160, 150, 150, 140, 130, 130, 120, 119, 118, 117, 114, 113, 112, 110, 105, 105, 105, 97, 97, 97, 95, 
 95, 90, 90, 90, 93, 95, 100, 100, 105, 108, 112, 112, 115, 118, 118, 120, 123, 123, 126, 128, 128, 130, 
 130, 130, 130, 129, 127, 127, 125, 124, 123, 122, 122, 120, 118, 117, 115, 112, 110, 108, 108, 108, 104, 
 104, 108, 108, 104, 104, 104, 104, 104};
 
// Flimmern
const uint8_t PROGMEM FLIMMER_ARRAY[16] =
{70, 90, 110, 128, 140, 140, 128, 110, 90, 70, 50, 32, 20, 20, 32, 50};

// Nullinie variabel
uint8_t UP_AND_DOWN[2] = {1, 1};
  
//Variablen
volatile uint8_t zeiger; 					// zum Auslesen des Arrays
volatile uint16_t zaehler = 4;				// Zähler für Verzögerung
volatile uint8_t current_state = 1;
volatile uint8_t next_state = 1;
volatile uint8_t zufall = 3;

// Normal Sinus 
void showEKG(void)
{
	uint8_t data=0;
	
	if (current_state != next_state)
	{
		current_state = next_state;
		zeiger = 0;
		zaehler = 3;	
	}

	zaehler--;
	if (zaehler < 1)
	{
		zaehler = 3;										//3fach-Verzögerung des Tabellenauslesens
		zeiger++;											//zeiger erhöhen
		if (zeiger > 117)
		{
			zeiger = 0;										//zeiger zurücksetzen
			switch (current_state) {
				case 1:	zaehler = 130;						// Normal Sinus
						OCR0A = 170;
						break;
				case 2:	zaehler = 850;						// Bradykard
						OCR0A = 180;
						break;
				case 3:	zaehler = 20;						// Tachykard
						OCR0A = 150;
						break;
				case 4:	zaehler = 130;						// Schrittmacher
						OCR0A = 170;
			}
		}
		if (current_state < 4)
			data = (pgm_read_byte(&SINUS_ARRAY[zeiger]))+50;
		else
			data = (pgm_read_byte(&SCHRITT_ARRAY[zeiger]));
			if ((zeiger > 40) && (zeiger <= 55))			// LED einschalten
				PORTB &= ~(1<<BLINK_LICHT);
			else
				PORTB |= (1<<BLINK_LICHT);	
		OCR0B = data;										// Daten ins PWM-Register
	}
}

// Extrasystolen
void showEKG_5(void)
{
	uint8_t data=0;
	
	if (current_state != next_state)
	{
		current_state = next_state;
		zeiger = 0;
		zaehler = 3;
		OCR0A = 170;	
	}

	zaehler--;
	if (zaehler < 1)
	{
		zaehler = 3;										
		zeiger++;											
		if (zeiger > 117)
		{
			zeiger = 0;
			zufall--;
			if (zufall < 1) 
			{			
				zaehler = 20;
				zufall = 6;	
			}				
			else									
				zaehler = 230;									
		}
		data = (pgm_read_byte(&SINUS_ARRAY[zeiger]))+50;
			if ((zeiger > 40) && (zeiger <= 55))			
				PORTB &= ~(1<<BLINK_LICHT);
			else
				PORTB |= (1<<BLINK_LICHT);	
		OCR0B = data;										
	}								
}

// Kammerflimmern
void showEKG_6(void)
{
	uint8_t data=0;
	
	if (current_state != next_state)
	{
		current_state = next_state;
		zeiger = 0;
		zaehler = 8;
		OCR0A = 170;
		UP_AND_DOWN[0]=1;
		UP_AND_DOWN[1]=0;	
	}

	zaehler--;
	if (zaehler < 1)
	{
		zaehler = 8;										
		zeiger++;											
		if (zeiger > 15)
		{
			zeiger = 0;										
			zaehler = 8;			
		}
			if (UP_AND_DOWN[0] == 1)
				UP_AND_DOWN[1] = UP_AND_DOWN[1]+3;				
			else
				UP_AND_DOWN[1] = UP_AND_DOWN[1]-3;
			if (UP_AND_DOWN[1] > 15)
				UP_AND_DOWN[0]=0;	
			if (UP_AND_DOWN[1] < 2)
				UP_AND_DOWN[0]=1;
		data = (pgm_read_byte(&FLIMMER_ARRAY[zeiger]))+UP_AND_DOWN[1];
		if ((zeiger > 4) && (zeiger <= 10))			
			PORTB &= ~(1<<BLINK_LICHT);
		else
			PORTB |= (1<<BLINK_LICHT);	
		OCR0B = data;										
	}
}

// Nullinie
void showEKG_7(void)
{	
	if (current_state != next_state)
	{
		current_state = next_state;
		zeiger = 0;
		zaehler = 3;
		OCR0A = 170;	
		UP_AND_DOWN[0]=1;
		UP_AND_DOWN[1]=90;
		PORTB &= ~(1<<BLINK_LICHT);
	}
	
	zaehler--;
	if (zaehler < 1)
	{	
		zaehler = 120;
		if (UP_AND_DOWN[0] == 1) 
		{
			UP_AND_DOWN[1] = UP_AND_DOWN[1]+3;
			if (UP_AND_DOWN[1] > 110)
			UP_AND_DOWN[0] = 0;
		}			
		if (UP_AND_DOWN[0] == 0) 
		{
			UP_AND_DOWN[1] = UP_AND_DOWN[1]-3;
			if (UP_AND_DOWN[1] < 70)
			UP_AND_DOWN[0] = 0;
		}		
	}
	OCR0B = UP_AND_DOWN[1];								
}


ISR (TIMER0_OVF_vect)
{
	switch(next_state)
	{
		case 1: showEKG();
				break;
		case 2: showEKG();
				break;
		case 3: showEKG();
				break;
		case 4: showEKG();
				break;
		case 5: showEKG_5();
				break;
		case 6: showEKG_6();
				break;
		case 7: showEKG_7();
				break;
		default: setSegment(200);
				showEKG();	
	}
}

// 7-Segment Definitions
void setSegment(uint8_t zahl)
{
	PORTB = 0xFF;
	switch (zahl) 
	{	
		case 0:                 
				PORTB &= ~( (1<<a) | (1<<b) | (1<<c) | (1<<d) | (1<<e) | (1<<f) ); 
				break;
		case 1:                 
				PORTB &= ~( (1<<b) | (1<<c) ); 
				break;
		case 2:
				PORTB &= ~( (1<<a) | (1<<b) | (1<<d) | (1<<e) | (1<<g) );
				break;
		case 3:
				PORTB &= ~( (1<<a) | (1<<b) | (1<<c) | (1<<d) | (1<<g) );
				break;
		case 4:
				PORTB &= ~( (1<<b) | (1<<c)| (1<<f) | (1<<g) );
				break;
		case 5:
				PORTB &= ~( (1<<a) | (1<<c) | (1<<d) | (1<<f) | (1<<g) );
				break;
		case 6:
				PORTB &= ~( (1<<a) | (1<<c) | (1<<d) | (1<<e) | (1<<f) | (1<<g) );
				break;
		case 7:
				PORTB &= ~( (1<<a) | (1<<b)| (1<<c) );
				break;
		case 8:
				PORTB &= ~( (1<<a) | (1<<b) | (1<<c) | (1<<d) | (1<<e) | (1<<f) | (1<<g) );
				break;
		case 9:
				PORTB &= ~( (1<<a) | (1<<b) | (1<<c) | (1<<d) | (1<<f) | (1<<g) );
				break;
		default:
				PORTB &= ~( (1<<a) | (1<<d) | (1<<e) | (1<<f) | (1<<g) );
		  break;
	}
}

// 7-Segment Spielerei
void lightshow(uint8_t loops)
{ 
	for(uint8_t i = 0; i < loops; i++)
	{
		PORTB = 0xFF;
		_delay_ms(sleeptime);
		PORTB &= ~(1 << a);
		_delay_ms(sleeptime);
		PORTB &= ~(1 << b);
		_delay_ms(sleeptime);
		PORTB &= ~(1 << c);
		_delay_ms(sleeptime);
		PORTB &= ~(1 << d);
		_delay_ms(sleeptime);
		PORTB &= ~(1 << e);
		_delay_ms(sleeptime);
		PORTB &= ~(1 << f);
		_delay_ms(sleeptime);		
	}
}

// Taster Betätigung
ISR (INT1_vect)
{
	if (next_state < max_prog)
		next_state++;
	else
		next_state = 1;
	setSegment(next_state);
}	

int main(void)
{
//Initialisierung Ports
DDRD = 0x00;							// Alle Ports als Eingänge
PORTD |= (1<<PD3);						// Pull up für Schalter aktivieren
DDRD |= (1<<PD5);			// Puls und Signal als Ausgang
DDRB = 0xFF;							// 7-Segment Anzeige Ausgänge schalten
lightshow(init_runs);
setSegment(current_state);				// Alle LED ausschalten

// Externer Interrrupt für Schalter
MCUCR |= (1<<ISC11);					// Int 1 falling edge
MCUCR &= ~(1 <<ISC10);
GIMSK |= (1<<INT1);						// Ext Interrrupt 1 enable


//TIMER0 Init für PWM-EKG, PWM-Frequenz ~730Hz
//Berechnung 1MHz/8(Prescaler)/171(CTC)= PWM-Frequenz

OCR0A = 170;									//CTC-Register mit 171 laden
TCCR0A = ((1<<COM0B1)|(1<<WGM01)|(1<<WGM00));	//Clear on Compare Match, Fast PWM
TCCR0B = ((1<<WGM02)|(1<<CS01));				//Fast PWM, Prescaler = 8
TIMSK |= (1<<TOIE0);							//OVF-Interrupt freischalten

//Sleep_Mode Init
set_sleep_mode(SLEEP_MODE_IDLE);	//Init Sleep-Modus auf IDLE

sei();
	while(1)						//#### Main-loop ####
	{
		sleep_mode();				//AVR schlafen legen
	}
return(0);
}