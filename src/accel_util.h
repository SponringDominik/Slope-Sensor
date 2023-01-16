/************************************************************************/
/* Hilfs-Funktionen für den Beschleunigungssensor MC3632                */
/************************************************************************/
#ifndef ACCEL_UTIL_H_
#define ACCEL_UTIL_H_



/* Initialisierung des MC3632 */
void accel_init(void);

/* MC3632 Sleep-Mode aktivieren */
void accel_gosleep(void);

/* MC3632 aktivieren */
void accel_wakeup(void);

/* Lese bestimmte Adresse des MC3632 */
uint8_t accel_read_address(uint8_t);

/* Lese Beschleunigungs-Register des MC3632 */
uint8_t *read_acceleration(void);



#endif /* ACCEL_UTIL_H_ */