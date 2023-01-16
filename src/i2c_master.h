/************************************************************************/
/* I2C-Master-Library für ATTINY der Serie2                             */
/************************************************************************/
#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_



/* Sende ACK-Zeichen gefolgt von Stop-Zeichen */
void i2c_stop(void);

/* Sende NACK-Zeichen gefolgt von Stop-Zeichen */
void i2c_stop_NACK(void);

/* Sende ACK-Zeichen gefolgt von Start-Zeichen */
void i2c_restart(void);

/* Sende ACK-Zeichen */
void i2c_ACK(void);

/* Sende NACK-Zeichen */
void i2c_NACK(void);

/* Initialisierung des I2C (Master-Mode) */
void i2c_init(void);

/* Starte Busverbindung in Lese-Modus */
uint8_t i2c_start_read(uint8_t);

/* Starte Busverbindung in Schreibe-Modus */
uint8_t i2c_start_write(uint8_t);

/* Sende Daten-Byte */
uint8_t i2c_write(uint8_t);

/* Lese Daten */
uint8_t i2c_read(void);



#endif /* I2C_MASTER_H_ */