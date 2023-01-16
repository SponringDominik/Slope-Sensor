/************************************************************************/
/* I2C-Master-Library für ATTINY der Serie2                             */
/************************************************************************/
#include <avr/io.h>
#include <math.h>
#include <util/delay.h>

// Definiere CPU Frequenz in Hz, falls nicht schon definiert
#ifndef F_CPU
#define F_CPU 1000000UL
#endif
// I2C Frequenz in Hz
#define F_SCL  50000L




/* Sende ACK-Zeichen gefolgt von Stop-Zeichen */
void i2c_stop(void){
	TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_STOP_gc;
}


/* Sende NACK-Zeichen gefolgt von Stop-Zeichen */
void i2c_stop_NACK(void){
	TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
}


/* Sende ACK-Zeichen gefolgt von Start-Zeichen */
void i2c_restart(void){
	TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_REPSTART_gc;
}


/* Sende ACK-Zeichen */
void i2c_ACK(void){
	TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_RECVTRANS_gc;
}


/* Sende NACK-Zeichen */
void i2c_NACK(void){
	TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_RECVTRANS_gc;
}


/* Initialisierung des I2C (Master-Mode) */
void i2c_init(void){
	// Mit MBAUD kann die Bus-Frequenz eingestellt werden.
	// BAUD muss innerhalb gewisser Grenzen liegen (siehe Datenblatt)
	// t_R = 1µs
	// t_OF = 250ns
	// t_LOW_min = 4.7µs
	#define BAUD ((uint8_t) ceil(F_CPU/(2*F_SCL) - (5 + F_CPU*1E-6/2)))
	//#define BAUD ((uint8_t) ceil(F_CPU*(4.7E-6 + 250E-9) - 5))
	TWI0.MBAUD = BAUD;
	TWI0.MCTRLA = TWI_ENABLE_bm;				// Master aktivieren
	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;		// Bus-state auf Idle stellen
}


/* Kontrolliere den Status des I2C */
uint8_t i2c_check(void){
	uint8_t state;
	// Warte auf Antwort der beiden Interrupt-Flags
	while(!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))){
		// Lese Status-Register und überprüfe, ob Fehler erkannt wurde
		state = TWI0.MSTATUS;
		if(state & TWI_BUSERR_bm) return 1;
		if(state & TWI_ARBLOST_bm) return 1;
	}
	// Lese Status-Register
	state = TWI0.MSTATUS;
	// Wenn NACK empfangen -> Stop-Zeichen senden und abbrechen
	if(state & TWI_RXACK_bm){
		i2c_stop();
		return 1;
	}
	// Ist I2C-Master BUS-Owner?
	if((state & TWI_BUSSTATE_gm) == TWI_BUSSTATE_OWNER_gc){
		return 0;
	}
	else{
		return 1;
	}
}


/* Starte Busverbindung mit 8-Bit Adresse (R/W includiert) */
uint8_t i2c_start(uint8_t address){
	// Falls Bus-State Busy ist, Neustart des Bus und 5ms warten
	if((TWI0.MSTATUS & TWI_BUSSTATE_gm) == TWI_BUSSTATE_BUSY_gc){
		TWI0.MCTRLB |= TWI_FLUSH_bm;
		_delay_ms(5);
	}
	// Adresse senden und Bus-State überprüfen
	TWI0.MADDR = address;
	return i2c_check();
}


/* Starte Busverbindung in Lese-Modus */
uint8_t i2c_start_read(uint8_t address){
	return i2c_start((address << 1) + 1);
}


/* Starte Busverbindung in Schreibe-Modus */
uint8_t i2c_start_write(uint8_t address){
	return i2c_start(address << 1);
}


/* Sende Daten */
uint8_t i2c_write(uint8_t data){
	// Daten senden und Bus-State überprüfen
	TWI0.MDATA = data;
	return i2c_check();
}


/* Lese Daten */
uint8_t i2c_read(void){
	// Warte auf Daten
	while(!(TWI0.MSTATUS & TWI_RIF_bm)){
		// Lese Status-Register und überprüfe, ob Fehler erkannt wurde
		uint8_t state = TWI0.MSTATUS;
		if(state & TWI_BUSERR_bm) return 1;
		if(state & TWI_ARBLOST_bm) return 1;
	}
	return TWI0.MDATA;
}