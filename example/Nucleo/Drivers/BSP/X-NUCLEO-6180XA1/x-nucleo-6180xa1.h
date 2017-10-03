/*
 * @file x-nucleo-6180xa1.h
 * $Date: 2015-03-30 16:27:10 +0200 (Mon, 30 Mar 2015) $
 * $Revision: 2215 $
 */

#ifndef XNUCLEO6180XA1_H_
#define XNUCLEO6180XA1_H_


#define SWITCH_VAL_RANGING  0

/* function below must be provided  */
void XNUCLEO6180XA1_WaitMilliSec(int SegDelayMs);
/* optional list */
void XNUCLEO6180XA1_UserIntHandler(void);

/* provided */
void XNUCLEO6180XA1_GPIO_Init(void);
void XNUCLEO6180XA1_I2C1_Init(I2C_HandleTypeDef *hi2c1);
void XNUCLEO6180XA1_DisplayString(const char *str, int SegDelayMs);

void XNUCLEO6180XA1_EnableInterrupt(void);
void XNUCLEO6180XA1_DisableInterrupt(void);
void XNUCLEO6180XA1_ClearInterrupt(void);

int XNUCLEO6180XA1_GetSwitch(void);
void XNUCLEO6180XA1_Reset(int state);

/**
 * @brief  get version of Expansion board
 * @warning to be called only aftrer @a XNUCLEO6180XA1_I2C1_Init() get called
 * @return True if connected shield is v2
 */
int XNUCLEO6180XA1_IsV2(void);

/**
 * @brief  tio be called by HAL EXTI handler dispatecher
 * @warning to be called only aftrer @a XNUCLEO6180XA1_I2C1_Init() get called
 * @param pin
 * @return  non 0 if the GPIO_pin is the GPIO pin for active VL6180x GPIO int line
 */
int XNUCLEO6180XA1_EXTI_CallBackHandle(uint16_t GPIO_Pin);


/*all function below only valid for v2 */
#define XNUCLEO6180XA1_ID_TOP   0
#define XNUCLEO6180XA1_ID_BOT   1
#define XNUCLEO6180XA1_ID_LEFT  2
#define XNUCLEO6180XA1_ID_RIGHT 3
int XNUCLEO6180XA1_ResetId(int state, int id);

#endif /* XNUCLEO6180XA1_H_ */
