/************************************************************************/
/* Utility functions for acceleration sesnor MC3632                */
/************************************************************************/
#ifndef ACCEL_UTIL_H_
#define ACCEL_UTIL_H_



/* initialise MC3632 */
void accel_init(void);

/* activate sleep mode */
void accel_gosleep(void);

/* activate sensor */
void accel_wakeup(void);

/* read certain address of MC3632 */
uint8_t accel_read_address(uint8_t);

/* read accerlation register of MC3632 */
uint8_t *read_acceleration(void);



#endif /* ACCEL_UTIL_H_ */