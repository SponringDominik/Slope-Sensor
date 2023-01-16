/************************************************************************/
/* Hilfs-Funktionen für den Beschleunigungssensor MC3632                */
/************************************************************************/
#include <util/delay.h>
#include "i2c_master.h"

// Adresse des MC3632
#define ACCEL_ADRESS 0x4c


/* Schreibe Daten-Byte in Adresse */
uint8_t accel_write(uint8_t address, uint8_t data){
	// Starte Busverbindung. Bei Fehler -> abbruch
	if(i2c_start_write(ACCEL_ADRESS)) return 1;
	// Schreibe Zieladresse. Bei Fehler -> abbruch
	if(i2c_write(address)) return 1;
	// Schreibe Dateninhalt. Bei Fehler -> abbruch
	if(i2c_write(data)) return 1;
	// Beende Busverbindung
	i2c_stop();
	return 0;
}

/* Initialisierung des MC3632 */
void accel_init(void){
	// Vorgegebene Initialisierung:
	accel_write(0x10, 0b00000001);	// MC3632 in standby verstzen
	accel_write(0x24, 0b01000000);	// Initialization Register 1 gesetzt
	_delay_ms(5);
	accel_write(0x0d, 0b01000000);	// I2C als Bus aktivieren
	accel_write(0x0f, 0b01000010);	// Initialization Register 1 gesetzt
	accel_write(0x20, 0b00000001);	// Drive-X Register wird zur Initialisierung gesetzt
	accel_write(0x21, 0b10000000);	// Drive-Y Register wird zur Initialisierung gesetzt
	accel_write(0x28, 0b00000000);	// Initialization Register 2 gesetzt
	accel_write(0x1a, 0b00000000);	// Initialization Register 3 gesetzt
	// Eigene Initialisierung:
	accel_write(0x1c, 0b00000100);	// Power-Mode wird zu Precision-Mode gesetzt
	accel_write(0x11, 0b00000101);	// OutputDataRate wird auf 14Hz gesetzt
	accel_write(0x15, 0b00000101);	// Resolution wird auf 14Bit gesetzt
}


/* MC3632 Sleep-Mode aktivieren */
void accel_gosleep(void){
	accel_write(0x10, 0b00000000);	// MC3632 in SLEEP verstzen
	_delay_ms(5);
}


/* MC3632 aktivieren */
void accel_wakeup(void){
	accel_write(0x10, 0b00000101);	// MC3632 in CWAKE verstzen
}


/* Lese bestimmte Adresse des MC3632 */
uint8_t accel_read_address(uint8_t address){
	// Starte Busverbindung. Bei Fehler -> abbruch
	if(i2c_start_write(ACCEL_ADRESS)) return 1;
	// Schreibe Zieladresse. Bei Fehler -> abbruch
	if(i2c_write(address)) return 1;
	// Erzeuge Restart
	i2c_restart();
	// Starte erneut Busverbindung. Bei Fehler -> abbruch
	if(i2c_start_read(ACCEL_ADRESS)) return 1;
	// Lese Dateninhalt
	uint8_t data = i2c_read();
	// Beende Busverbindung
	i2c_stop_NACK();
	return data;
}


/* Lese Beschleunigungs-Register des MC3632 */
uint8_t *read_acceleration(void){
	// Insgesamt 6 Byte
	#define SIZE 6
	// Array mit Beschleunigungswerten
	static uint8_t accel[SIZE];
	// Starte Busverbindung. Bei Fehler -> abbruch
	if(i2c_start_write(ACCEL_ADRESS)) return 1;
	// Schreibe Adresse des ersten Registers. Bei Fehler -> abbruch
	if(i2c_write(0x2)) return 1;
	// Erzeuge Restart
	i2c_restart();
	// Starte erneut Busverbindung. Bei Fehler -> abbruch
	if(i2c_start_read(ACCEL_ADRESS)) return 1;
	// Lese alle 6 Register
	for(uint8_t i=0; i<SIZE; i++){
		accel[i] = i2c_read();
		// send ACK. Außer nach letztem Byte
		if(i < SIZE-1) i2c_ACK();
	}
	// Beende Busverbindung
	i2c_stop_NACK();
	return accel;
}