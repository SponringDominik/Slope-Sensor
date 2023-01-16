/************************************************************************/
/* Hilfs-Funktionen für den LCD-Driver BU91796                          */
/************************************************************************/
#include <avr/io.h>
#include <stdlib.h>
#include "i2c_master.h"

// Adresse des LCD-Driver
#define LCD_ADRESS 0x3e
// Befehle
#define LCD_ON	0b11001000		// Display on
#define ALL_SEG_ON 0b11111110	// Alle Segmente einschalten
#define LCD_BLINK 0b11110001	// Blinken 0.5Hz



/* Übertrage Daten-Array */
uint8_t tx_data(uint8_t *data, uint8_t size){
	// Starte Busverbindung. Bei Fehler -> abbruch
	if(i2c_start_write(LCD_ADRESS)) return 1;
	// Sende jedes Byte in Array. Bei Fehler -> abbruch
	for(uint8_t i=0; i<size; i++){
		if(i2c_write(data[i])) return 1;
	}
	// Beende Busübertragung
	i2c_stop();
	return 0;
}

/* Initierungsvorgang des LCD-Displays */
void lcd_init(void){
	// Daten-Array, das übertragen werden soll
	uint8_t data[] = {
		0b11101010,		// Software-Reset
		0b11101000,		// Oszilator auf intern stellen
		0b10111100,		// 53Hz, Frame inversion, Power-Save-Mode
		0b11000000,		// Display off
		0b11110000,		// Blinken ausschalten
		0b11111100};	// All-Pixel-Control ausschalten
	// Übertrage Daten
	tx_data(data, sizeof(data));
}


/* Beschreiben des LCD-Speichers. Display wird außerdem eingeschalten */
void lcd_write(int8_t data){
	// data: Integer der angezeigt werden soll. (-99 < data < 99)
	
	// Überprüfe, ob data innerhalb Spezifikation liegt
	if(abs(data) > 99) return;
	// Splitte data in zwei Digits auf
	uint8_t digit_1 = abs(data) / 10;
	uint8_t digit_2 = abs(data) % 10;
	// Look-Up-Table welche Segmente bei gegebener Zahl eingeschalten werden sollen
	uint8_t Segment[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
		                   0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};
	// Daten-Array definieren. N_COMMAND ist die Anzahl an Kommandos, die vorher gesendet werden.
	#define N_COMMAND 2
	uint8_t LCD_Data[10+N_COMMAND] = {
		0b11001000,		// Display on
		0b00000000,		// DDRAM-Adresse 0x00
		// Speicherinnhalt:
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	// Entscheiden, ob Segment eingeschalten werden soll:
	// Speicher besteht aus 20x4 Bit Blöcken. Es wird immer nur das erste Bit jedes Blockes
	// verwedet. Daher reicht es nur dieses zu setzen (0x80 oder 0x08).
	// Es wird abgefragt, ob entsprechendes Bit in der Look-Up-Table gesetzt ist.
	// Bit 0 entspricht Segment A ... Bit 6 entspricht Segment G
	// Nummerierung der LCD-Digits: 1,2,3
	// Verwendet wird nur 2 und 3. Segment G1 wird jedoch als Vorzeichen verwendet.
	if(Segment[digit_1] & (1<<5)) LCD_Data[0+N_COMMAND] |= 0x80;  // F2
	if(Segment[digit_1] & (1<<0)) LCD_Data[0+N_COMMAND] |= 0x08;  // A2
	if(Segment[digit_2] & (1<<1)) LCD_Data[1+N_COMMAND] |= 0x80;  // B3
	if(Segment[digit_2] & (1<<1)) LCD_Data[1+N_COMMAND] |= 0x80;  // B3
	if(Segment[digit_2] & (1<<0)) LCD_Data[1+N_COMMAND] |= 0x08;  // A3
	if(Segment[digit_2] & (1<<5)) LCD_Data[2+N_COMMAND] |= 0x80;  // F3
	if(Segment[digit_2] & (1<<6)) LCD_Data[2+N_COMMAND] |= 0x08;  // G3
	if(Segment[digit_1] & (1<<1)) LCD_Data[3+N_COMMAND] |= 0x80;  // B2
	// Segment 7-10 unverwendet
	if(Segment[digit_2] & (1<<2)) LCD_Data[5+N_COMMAND] |= 0x08;  // C3
	if(Segment[digit_2] & (1<<3)) LCD_Data[6+N_COMMAND] |= 0x80;  // D3
	if(Segment[digit_2] & (1<<4)) LCD_Data[6+N_COMMAND] |= 0x08;  // E3
	// LCD_Data[7] |= 0x80;                             // DP2
	if(Segment[digit_1] & (1<<2)) LCD_Data[7+N_COMMAND] |= 0x08;  // C2
	if(Segment[digit_1] & (1<<3)) LCD_Data[8+N_COMMAND] |= 0x80;  // D2
	if(Segment[digit_1] & (1<<4)) LCD_Data[8+N_COMMAND] |= 0x08;  // E2
	if(data < 0) LCD_Data[9+N_COMMAND] |= 0x80;                   // G1
	if(Segment[digit_1] & (1<<6)) LCD_Data[9+N_COMMAND] |= 0x08;  // G2
	/* Führende Null wird ausgeblendet */
	if(digit_1==0){
		LCD_Data[0+N_COMMAND]  = 0x00;
		LCD_Data[3+N_COMMAND] &= 0x0f;
		LCD_Data[7+N_COMMAND] &= 0xf0;
		LCD_Data[8+N_COMMAND]  = 0x00;
		/* Vorzeichen wird nachgerückt */
		if(data < 0) LCD_Data[9+N_COMMAND] = 0x08;
		else         LCD_Data[9+N_COMMAND] = 0x00;
	}
	// Erzeugte Displaydaten werden an Controller gesendet:
	// Als erstes wird die Speicheradresse gesendet (hier erste Adresse 0x00)
	// und der Display eingeschalten. Anschließend werden der Reihe nach alle Daten gesendet.
	// Controller inkrementiert selbst Speicheradresse. Bei Überlauf beginnt sie wieder bei 0x00
	tx_data(LCD_Data, sizeof(LCD_Data));
}