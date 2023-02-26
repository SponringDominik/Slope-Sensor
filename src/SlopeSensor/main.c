/************************************************************************/
/* SlopeSensor															*/
/************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "mcu_init.h"
#include "i2c_master.h"
#include "accel_util.h"
#include "lcd_util.h"

/*------------------------------------------------------------------------------------*/
// Zeit, die der Microcontrollers nach Tasterbetätigung wach bleibt in 10ms
#define WAKE_TIME 2000
// Entprell-Zeit des Tasters in 10ms
#define DEBOUNCE 10
// Vorwiderstand des Thermistors R3:
//	0: NTC-Variante
//	1: PTC-Variante
#if (0)
#  define R3_PTC 46250
#else
#  define R3_NTC 10700
#endif



/* Globale Variablen -----------------------------------------------------------------*/
// Laufzeit in 10ms
uint16_t millisecond_10;
// Laufzeit, bis der Microcontroller schlafen geht in 10ms
uint16_t sleep_at_millis = WAKE_TIME;
// Bis zu dieser Zeit wird eine Zustandsänderung des Tasters ignoriert (Prellen)
uint16_t wait_time = 0;
// Zustand des Sensors:
//		0: Winkel messen
//		1: Temperatur messen
uint8_t State = 0;


/*------------------------------------------------------------------------------------*/
/* Power-Save aktivieren */
void go_sleep(void){
	// Timer Stoppen
	TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;	
	// Accelerometer ausschalten
	accel_gosleep();
	// Display ausschalten
	PORTA.OUTCLR = LCD_EN;	// Schalte Versorgung für LCD aus
	// Zustände wieder zurücksetzen
	millisecond_10 = 0;
	State = 0;
	sleep_at_millis = WAKE_TIME;
	wait_time = 0;
	// Interrupt für Flankenänderung aktivieren
	PORTB.PIN2CTRL &= ~PORT_ISC_gm;
	PORTB.PIN2CTRL |=PORT_ISC_BOTHEDGES_gc;	
	// Schlafen gehen
	// Sleep-Mode auf Power-Down stellen und Sleep-Enable aktivieren
	SLPCTRL.CTRLA = SLPCTRL_SEN_bm | SLPCTRL_SMODE_PDOWN_gc;
	// Assembler Befehl "Sleep" startet ausgewälten Sleep-Modus
	// Sleep-Enable muss davor gesetzt werden (möglichst direkt davor)
	asm("SLEEP");
	
	
	// Wake-Up Sequenz
	// Accelerometer aktivieren
	accel_wakeup();
	// Versorgung für LCD einschalten
	PORTA.OUTSET = LCD_EN;
	lcd_init();
	// Timer zurücksetzen und starten
	TCA0.SINGLE.CNTL = 0;
	TCA0.SINGLE.CNTH = 0;
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

/* Ruhemodus aktivieren */
void go_idle(void){
	// Sleep-Mode auf IDLE stellen und Sleep-Enable aktivieren
	SLPCTRL.CTRLA = SLPCTRL_SEN_bm | SLPCTRL_SMODE_IDLE_gc;
	// Assembler Befehl "Sleep" startet ausgewälten Sleep-Modus
	// Sleep-Enable muss davor gesetzt werden (möglichst direkt davor)
	asm("SLEEP");
}


#ifdef R3_NTC
/* Lese ADC aus und berechne Temperatur */
int8_t get_temperature(void){
	// Tabelle des NTC:
	// Enthält Widerstandswerte zu bekannten Temperaturen von -40°C bis 40°C in 5°C Schritten
	uint32_t R_NTC_Table[17] = {205200, 154800, 117900, 90690, 70370, 55070, 43440, 34530, 
								27640, 22270, 18060, 14740, 12110, 10000, 8309, 6941, 5828};
	// Starte AD Konvertierung
	ADC0.COMMAND = ADC0.COMMAND | ADC_START_IMMEDIATE_gc;
	// Warte auf Ergebniss
	while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
	// Lese ADC aus
	uint32_t raw_ADC = ADC0.RESULT;
	// Berechne Widerstandswert
	uint32_t R_NTC = raw_ADC*R3_NTC/(4096 - raw_ADC);
	
	// Berechne Temperatur mithilfe linearer Interpolation aus der TEMP-Tabelle
	uint32_t X1, X2;	// Unterer und oberer Widerstandswert
	// Falls Widerstand größer als größter Wert in Tabelle:
	X1 = R_NTC_Table[0];
	if(R_NTC > X1) return -88;
	// Suche ersten Wert, der größer als der gemessene Wert ist
	for(uint8_t i=1; i<17; i++){
		X2 = R_NTC_Table[i];
		if(R_NTC > X2){
			X1 = R_NTC_Table[i-1];
			// Interpoliere zwischen Werten
			return lrint(-45 + i*5 + ((float)(X1 - R_NTC) / (X1 - X2)) * 5);
		}
	}
	// Falls Widerstand kleiner als kleinster Wert in Tabelle:
	return 88;
}
#endif
#ifdef R3_PTC
/* Lese ADC aus und berechne Temperatur */
int8_t get_temperature(void){
	// Tabelle des PTC:
	// Enthält Widerstandswerte zu bekannten Temperaturen von -40°C bis 40°C in 5°C Schritten
	uint16_t R_PTC_Table[17] = {30230, 31234, 32268, 33332, 34427, 35554, 36713, 37905, 39129,
								40387, 41679, 43006, 44367, 45763, 47194, 48661, 50164};
	// Starte AD Konvertierung
	ADC0.COMMAND = ADC0.COMMAND | ADC_START_IMMEDIATE_gc;
	// Warte auf Ergebniss
	while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
	// Lese ADC aus
	uint16_t raw_ADC = ADC0.RESULT;
	// Berechne Widerstandswert
	uint16_t R_PTC = raw_ADC*R3_PTC/(4096 - raw_ADC);
	
	// Berechne Temperatur mithilfe linearer Interpolation aus der TEMP-Tabelle
	uint16_t X1, X2;	// Unterer und oberer Widerstandswert
	// Falls Widerstand kleiner als kleinster Wert in Tabelle:
	X1 = R_PTC_Table[0];
	if(R_PTC < X1) return -88;
	// Suche ersten Wert, der größer als der gemessene Wert ist 
	for(uint8_t i=1; i<17; i++){
		X2 = R_PTC_Table[i];
		if(R_PTC < X2){
			X1 = R_PTC_Table[i-1];
			// Interpoliere zwischen Werten
			return lrint(-45 + i*5 + ((float)(R_PTC - X1) / (X2 - X1)) * 5);
		}
	}
	// Falls Widerstand größer als größter Wert in Tabelle:
	return 88;
}
#endif


/* Berechne den Neigungswinkel */
uint8_t calc_theta(int32_t X, int32_t Y, int32_t Z){
	int8_t theta = lrint(asin(X/sqrt(X*X + Y*Y + Z*Z))*180/M_PI);
	return abs(theta);
}




/* Main Program ----------------------------------------------------------------------*/
int main(void){
	MCU_init();			// Initierung des Microcontrollers
	i2c_init();			// Initierung des I2C-Bus
	accel_init();		// Initierung des Beschleunigungssensors

	// Interrupts aktivieren
	sei();
	// Nach Initialisierung -> Sleep
	go_sleep();
	
	
/* Main Loop --------------------------------------------------------------------------*/
	while (1){
		// Wenn Zeit abgelaufen -> Sleep
		if(millisecond_10 > sleep_at_millis) go_sleep();
		
		// Verhindert zu langes Warten beim Umschalten von 
		// Winkelmessung zu langsamer Temp-Messung
		static bool instantRun = false;
		// Alter Tasterzustand
		static bool button_old = true;
		// Neuer Tasterzustand (Taster schaltet auf Gnd)
		bool button = !(PORTB.IN & BUTTON);
		// Positive Flanke & außerhalb der Prell-Zeit:
		if(button && !button_old && (millisecond_10 > wait_time)){
			// Wartezeit zum entprellen setzen
			wait_time = millisecond_10 + DEBOUNCE;
			// Wachzeit verlängern
			sleep_at_millis = millisecond_10 + WAKE_TIME;
			// Zustand umschalten
			if(State == 0){
				State = 1;
				instantRun = true;
			}else{
				if(State == 1) State = 0;
			}
		}
		button_old = button;
		
		// Winkel messen
		if(State == 0){
			// Alle 0.1s wird Wert aktualisiert
			if(millisecond_10%20 == 1){
				// Lese Beschleunigungswerte
				uint8_t* v_accel = read_acceleration();
				// Erstelle aus den 6 Byte 3 Beschleunigungswerte
				int16_t X, Y, Z;
				X = v_accel[0] | (v_accel[1] << 8);
				Y = v_accel[2] | (v_accel[3] << 8);
				Z = v_accel[4] | (v_accel[5] << 8);
				// Berechne Neigungswinkel der Längsachse
				uint8_t theta_Y = calc_theta(Y, X, Z);
				// Neigungswinkel anzeigen
				lcd_write(theta_Y);
			}
		}
		// Temperatur messen
		if(State == 1){
			// Alle 1s wird Wert aktualisiert oder direkt nach umschalten des Zustands
			if(millisecond_10%100 == 1 || instantRun){
				instantRun = false;
				// Stromversorgung aktivieren
				PORTA.OUTSET = TEMP_EN;
				// Temperaturwert messen
				int8_t temperature = get_temperature();
				// Stromversorgung deaktivieren
				PORTA.OUTCLR = TEMP_EN;
				// Temperaturwert anzeigen
				lcd_write(temperature);
			}
		}
		// Ruhemodus aktivieren

		go_idle();
	}
}


/* Interrupts -------------------------------------------------------------------------*/

/* Interrupt für Flankenänderung an PINB2 -> Taster gedrückt */
ISR(PORTB_PORT_vect){
	uint8_t intflags = PORTB.INTFLAGS;
	PORTB.INTFLAGS = intflags;
	// Interrupt deaktivieren
	PORTB.PIN2CTRL &= ~PORT_ISC_gm;
	// Sleep disable
	SLPCTRL.CTRLA = 0;
}
/* Interrupt für Timer-Overflow -> 1ms vergangen */
ISR(TCA0_OVF_vect){
	uint8_t intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	// Zeit um 1ms erhöhen
	millisecond_10 += 1;
}