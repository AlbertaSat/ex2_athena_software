/*
 * rtcmk.c
 *
 *  Created on: May 28, 2018
 *  Most recent edit: Aug 11, 2020
 *      Author: sdamk, jdlazaru, Dustin Wagner
 *
 *
 *
 *      TODO: Check if interrupts are needed/how to implement
 */

#include "HL_i2c.h"
#include "rtcmk.h"

#include <time.h>

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Set content of a register.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] reg
 *   Register to write (input register cannot be written).
 *
 * @param[in] val
 *   Value used when writing to register.
 *
 * @return
 *   Returns 0 if register written, <0 if unable to write to register.
 ******************************************************************************/
int RTCMK_RegisterSet(/* I2C_TypeDef *i2c, */
                         uint8_t addr,
                         RTCMK_Register_TypeDef reg,
                         uint8_t val)
{

  uint8_t data;

  data = val;

  i2cSetDirection(i2cREG2, I2C_TRANSMITTER);
  i2cSetCount(i2cREG2, 1);
  i2cSetMode(i2cREG2, I2C_MASTER);
  i2cSetStop(i2cREG2);
  i2cSetStart(i2cREG2);
  i2cSend(i2cREG2, 1, &data);

  /* Wait until Bus Busy is cleared */
  while(i2cIsBusBusy(i2cREG2) == true);

  /* Wait until Stop is detected */
  while(i2cIsStopDetected(i2cREG2) == 0);

  /* Clear the Stop condition */
  i2cClearSCD(i2cREG2);

//  unsigned int retries = 0;

//  while (I2C_getStatus(i2c) == i2cTransferInProgress)
//  {
//    /* Enter EM1 while waiting for I2C interrupt */
//    if(MAX_RETRIES < retries++)break;
//    /* Enter EM1 while waiting for I2C interrupt */
//    EMU_EnterEM1();
//    /* Could do a timeout function here. */
//  }
  
  return((int)i2cIsBusBusy(i2cREG2));
}

/***************************************************************************//**
 * @brief
 *   Get current content of a register.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] reg
 *   Register to read.
 *
 * @param[out] val
 *   Reference to place register read.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_RegisterGet(/*I2C_TypeDef *i2c,*/
                         uint8_t addr,
                         RTCMK_Register_TypeDef reg,
                         uint8_t *val)
{
  /* Configure address of Slave to talk to */
    i2cSetSlaveAdd(i2cREG2, addr);

    i2cSetDirection(i2cREG2, I2C_TRANSMITTER);
    i2cSetCount(i2cREG2, 1);
    i2cSetMode(i2cREG2, I2C_MASTER);
    i2cSetStop(i2cREG2);
    i2cSetStart(i2cREG2);
    i2cSend(i2cREG2, 1, &reg);

    while(i2cIsBusBusy(i2cREG2) == true);
    while(i2cIsStopDetected(i2cREG2) == 0);
    i2cClearSCD(i2cREG2);

    int temp;
    for (temp = 0; temp < 0x10000; temp++);//temporary fix... don't want delay down the road

    i2cSetSlaveAdd(i2cREG2, addr);
    /* Set direction to receiver */
    i2cSetDirection(i2cREG2, I2C_RECEIVER);
    i2cSetCount(i2cREG2, 1);
    /* Set mode as Master */
    i2cSetMode(i2cREG2, I2C_MASTER);
    i2cSetStop(i2cREG2);
    /* Transmit Start Condition */
    i2cSetStart(i2cREG2);
    *val = i2cReceiveByte(i2cREG2);

//  unsigned int retries = 0;
//  while (I2C_getStatus(i2c) == i2cTransferInProgress)
//  {
//    /* Enter EM1 while waiting for I2C interrupt */
//    if(MAX_RETRIES < retries++)break;
//    /* Enter EM1 while waiting for I2C interrupt */
//    EMU_EnterEM1();
//    /* Could do a timeout function here. */
//  }

    /* Wait until Bus Busy is cleared */
    while(i2cIsBusBusy(i2cREG2) == true);

    /* Wait until Stop is detected */
    while(i2cIsStopDetected(i2cREG2) == 0);

    /* Clear the Stop condition */
    i2cClearSCD(i2cREG2);

  return((int)i2cIsBusBusy(i2cREG2));
}

/***************************************************************************//**
 * @brief
 *   Write 0's to time and calender registors (0x00 to 0x06).
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ResetTime(/*I2C_TypeDef *i2c,*/
                         uint8_t addr)
{


    uint8_t data[8] = {0};

    data[0] = ((uint8_t)RTCMK_RegSec) << 1;

    i2cSetSlaveAdd(i2cREG2, addr);
    i2cSetDirection(i2cREG2, I2C_TRANSMITTER);
    i2cSetCount(i2cREG2, 8);
    i2cSetMode(i2cREG2, I2C_MASTER);
    i2cSetStop(i2cREG2);
    i2cSetStart(i2cREG2);
    i2cSend(i2cREG2, 8, data);
    while(i2cIsBusBusy(i2cREG2) == true);
    while(i2cIsStopDetected(i2cREG2) == 0);
    i2cClearSCD(i2cREG2);

  //  while (I2C_getStatus(i2c) == i2cTransferInProgress)
  //  {
  //    /* Enter EM1 while waiting for I2C interrupt */
  //    EMU_EnterEM1();
  //    /* Could do a timeout function here. */
  //  }

    return((int)i2cIsBusBusy(i2cREG2));
}

/******************************************************************************/
/***********************************Readers************************************/
/******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Returns current content of seconds register in decimal.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_ReadSeconds(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegSec,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_SEC_SEC_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of minutes register in decimal.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_ReadMinutes(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegMin,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_MIN_MIN_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of hours register in decimal.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_ReadHours(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegHour,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_HOUR_HOUR_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of weeks register in decimal.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_ReadWeeks(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegWeek,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_WEEK_WEEK_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of days register in decimal.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_ReadDays(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegDay,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_DAY_DAY_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of months register in decimal.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_ReadMonths(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegMonth,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_MONTH_MONTH_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of years register in decimal.
 *
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if register read, <0 if unable to read register.
 ******************************************************************************/
int RTCMK_ReadYears(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegYear,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_YEAR_YEAR_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}


/******************************************************************************/
/***********************************Writers************************************/
/******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Writes value given in decimal to seconds register.
 * @attention
 *   Value limited to 61*. *generally 0-59. extra is for leap seconds
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *   
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[in] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_WriteSeconds(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t val)
{
  int ret = -1;
  
  if (val > 61) {
    return(ret);
  }

  uint8_t tmp = ((val / 10) << 4) + (val % 10);

  tmp &= _RTCMK_SEC_SEC_MASK;

  return RTCMK_RegisterSet(addr,RTCMK_RegSec,tmp);
}

/***************************************************************************//**
 * @brief
 *   Writes value given in decimal to minutes register.
 * @attention
 *   Value limited to 59 
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *   
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[in] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_WriteMinutes(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t val)
{
  int ret = -1;
  
  if (val > 59) {
    return(ret);
  }

  uint8_t tmp = ((val / 10) << 4) + (val % 10);

  tmp &= _RTCMK_MIN_MIN_MASK;

  return RTCMK_RegisterSet(addr,RTCMK_RegMin, tmp);
}

/***************************************************************************//**
 * @brief
 *   Writes value given in decimal to hours register.
 * @attention
 *   Value limited to 24 
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *   
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[in] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_WriteHours(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t val)
{
  int ret = -1;
  
  if (val > 24) {
    return(ret);
  }

  uint8_t tmp = ((val / 10) << 4) + (val % 10);

  tmp &= _RTCMK_HOUR_HOUR_MASK;

  return RTCMK_RegisterSet(addr,RTCMK_RegHour,tmp);
}

/***************************************************************************//**
 * @brief
 *   Writes value given in decimal to weeks register.
 * @attention
 *   Value limited to 6. 0 indexed at Sunday
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *   
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[in] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_WriteWeeks(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t val)
{
  int ret = -1;
  
  if (val > 6) {
    return(ret);
  }

  uint8_t tmp = ((val / 10) << 4) + (val % 10);

  tmp &= _RTCMK_WEEK_WEEK_MASK;

  return RTCMK_RegisterSet(addr,RTCMK_RegWeek,tmp);
}

/***************************************************************************//**
 * @brief
 *   Writes value given in decimal to days register.
 * @attention
 *   Value limited to 31. Be careful of months that shouldn't have 31 days 
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *   
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[in] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_WriteDays(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t val)
{
  int ret = -1;
  
  if (val > 31) {
    return(ret);
  }

  uint8_t tmp = ((val / 10) << 4) + (val % 10);

  tmp &= _RTCMK_DAY_DAY_MASK;

  return RTCMK_RegisterSet(addr,RTCMK_RegDay,tmp);
}

/***************************************************************************//**
 * @brief
 *   Writes value given in decimal to months register.
 * @attention
 *   Value limited to 12
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *   
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[in] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_WriteMonths(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t val)
{
  int ret = -1;
  
  if (val > 12) {
    return(ret);
  }

  uint8_t tmp = ((val / 10) << 4) + (val % 10);

  tmp &= _RTCMK_MONTH_MONTH_MASK;

  return RTCMK_RegisterSet(addr,RTCMK_RegMonth,tmp);
}

/***************************************************************************//**
 * @brief
 *   Writes value given in decimal to years register.
 * @attention
 *   Value limited to 99. should be last 2 digits of year
 * @param[in] i2c
 *   Pointer to I2C peripheral register block.
 *   
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[in] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_WriteYears(/*I2C_TypeDef *i2c,*/
                       	uint8_t addr,
                       	uint8_t val)
{
  int ret = -1;
  
  if (val > 99) {
    return(ret);
  }

  uint8_t tmp = ((val / 10) << 4) + (val % 10);

  tmp &= _RTCMK_YEAR_YEAR_MASK;

  return RTCMK_RegisterSet(addr,RTCMK_RegYear,tmp);
}


/******************************Generic functions*******************************/

/***************************************************************************//**
 * @brief
 *    Writes given UNIX 32 bit timestamp to RTC
 *
 * @param[in] unix_timestamp
 *   Timestamp to use for setting clock
 *
 * @return
 *   Returns 0 if success, <0 if unable to acquire value.
 ******************************************************************************/

int RTC_set_unix_time(uint32_t unix_timestamp) {
  int ret = -1;
  struct tm t = {};
  t = *gmtime(&unix_timestamp);
  if (!RTCMK_WriteSeconds(RTCMK_ADDR, (uint8_t)t.tm_sec)) return(ret);
  if (!RTCMK_WriteMinutes(RTCMK_ADDR, (uint8_t)t.tm_min)) return(ret);
  if (!RTCMK_WriteHours(RTCMK_ADDR, (uint8_t)t.tm_hour)) return(ret);
  if (!RTCMK_WriteWeeks(RTCMK_ADDR, (uint8_t)t.tm_wday)) return(ret);
  if (!RTCMK_WriteDays(RTCMK_ADDR, (uint8_t)t.tm_mday)) return(ret);
  if (!RTCMK_WriteMonths(RTCMK_ADDR, (uint8_t)t.tm_mon)) return(ret);
  //t.tm_year is years since 1900. Subtract 100 to get last 2 digits of current
  if (!RTCMK_WriteYears(RTCMK_ADDR, (uint8_t)t.tm_year - 100)) return(ret);
  return 0;
}


/***************************************************************************//**
 * @brief
 *    Reads UNIX timestamp from RTC into given variable
 *
 * @param[out] unix_timestamp
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if success, <0 if unable to acquire value.
 ******************************************************************************/

int RTC_get_unix_time(uint32_t *unix_timestamp) {
  int ret = -1;
  struct tm t = {};
  time_t unix_time;
  if (!RTCMK_ReadSeconds(RTCMK_ADDR, (uint8_t*)&t.tm_sec)) return(ret);
  if (!RTCMK_ReadMinutes(RTCMK_ADDR, (uint8_t*)&t.tm_min)) return(ret);
  if (!RTCMK_ReadHours(RTCMK_ADDR, (uint8_t*)&t.tm_hour)) return(ret);
  if (!RTCMK_ReadWeeks(RTCMK_ADDR, (uint8_t*)&t.tm_wday)) return(ret);
  if (!RTCMK_ReadDays(RTCMK_ADDR, (uint8_t*)&t.tm_mday)) return(ret);
  if (!RTCMK_ReadMonths(RTCMK_ADDR, (uint8_t*)&t.tm_mon)) return(ret);
  if (!RTCMK_ReadYears(RTCMK_ADDR, (uint8_t*)&t.tm_year)) return(ret);
  t.tm_year += 100; //adjust year to be years since 1900
  *unix_timestamp = mktime(&t);
  return 0;
}
