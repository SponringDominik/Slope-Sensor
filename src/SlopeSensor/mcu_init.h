#ifndef MCU_INIT_H
#define MCU_INIT_H

/* Grundeinstellungen des Microcontrollers */
#include <avr/io.h>
#include <math.h>


#define LCD_EN PIN1_bm
#define TEMP_EN PIN5_bm
#define BUTTON PIN2_bm


void MCU_init(){
	/**** CPU Einstellungen ****/
	// Clock divider auf 8 stellen. -> 16MHz/8 = 2MHz CPU-Takt
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_8X_gc | CLKCTRL_PEN_bm);
	
	/**** PIN Einstellungen ****/
	// Bei nicht verwendeten PINs wird der Input-Buffer deaktiviert,
	// um Strom zu sparen
	PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTB.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	// Ausgänge definieren
	PORTA.DIR = LCD_EN | TEMP_EN;
	// An PINB2 wird der Pull-Up Widerstand aktiviert
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	
	/**** ADC Einstellungen ****/
	ADC0.CTRLA = ADC_ENABLE_bm;					// ADC aktivieren
	ADC0.CTRLB = ADC_PRESC_DIV64_gc;			// Prescaler der ADC-Clock
	// Timebase einstellen: Muss minimal 1µs sein. Vref auf externe Quelle stellen
	#define TIMEBASE_VALUE ((uint8_t) ceil(F_CPU*0.000001))
	ADC0.CTRLC = (TIMEBASE_VALUE << ADC_TIMEBASE_gp) | ADC_REFSEL_VREFA_gc;
	ADC0.COMMAND = ADC_MODE_SINGLE_12BIT_gc;	// Single 12Bit Messung
	ADC0.MUXPOS = ADC_MUXPOS_AIN6_gc;			// Eingang des ADC wählen
	
	/**** TC Einstellungen ****/
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;		// CLK-divider auf 16 stellen
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;			// Interrupt bei overflow
	// Countertop berechnen und setzen. 2MHz/16/1250 = 100Hz
	#define CounterTop ((uint16_t) (F_CPU/16/100 - 1))
	TCA0.SINGLE.PERL = (uint8_t) CounterTop;
	TCA0.SINGLE.PERH = (CounterTop >> 8);
}

/* Fuses */
// Register, die nur über UPDI beschrieben werden können.
// Wichtig! Wenn FUSES beschrieben werden, müssen alle benötigen Bits in allen Registern
// gesetzt werden, da sie sonst mit '0' initialisiert werden
FUSES = {
	// CLK auf 16MHz stellen
	.OSCCFG = FREQSEL_16MHZ_gc,
	// Disable CRC, Disable NVM, Reset-Pin auf UPDI stellen
	.SYSCFG0 = CRCSRC_NOCRC_gc | 0x10 | RSTPINCFG_UPDI_gc,
	// Startup time auf 64ms stellen
	.SYSCFG1 = SUT_64MS_gc
};

#endif /* MCU_INIT_H */