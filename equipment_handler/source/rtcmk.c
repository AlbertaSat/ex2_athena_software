/*
 * rtcmk.c
 *
 *  Created on: May 28, 2018
 *  Most recent edit: Aug 11, 2020
 *      Author: sdamk, jdlazaru
 *
 *
 *
 *      TODO: Check if interrupts are needed/how to implement
 */

#include "HL_i2c.h"
#include "rtcmk.h"

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Set content of a register.
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
int RTCMK_RegisterSet(uint8_t addr,
                      RTCMK_Register_TypeDef reg,
                      uint8_t val)
{

  uint8_t data[2];

  data[0] = reg;
  data[1] = val;

  i2cSetDirection(RTCMK_PORT, I2C_TRANSMITTER);
  i2cSetCount(RTCMK_PORT, 2);
  i2cSetMode(RTCMK_PORT, I2C_MASTER);
  i2cSetStop(RTCMK_PORT);
  i2cSetStart(RTCMK_PORT);
  i2cSend(RTCMK_PORT, 2, data);

  /* Wait until Bus Busy is cleared */
  while(i2cIsBusBusy(RTCMK_PORT) == true);

  /* Wait until Stop is detected */
  while(i2cIsStopDetected(RTCMK_PORT) == 0);

  /* Clear the Stop condition */
  i2cClearSCD(RTCMK_PORT);

//  unsigned int retries = 0;

//  while (I2C_getStatus(i2c) == i2cTransferInProgress)
//  {
//    /* Enter EM1 while waiting for I2C interrupt */
//    if(MAX_RETRIES < retries++)break;
//    /* Enter EM1 while waiting for I2C interrupt */
//    EMU_EnterEM1();
//    /* Could do a timeout function here. */
//  }
  
  return((int)i2cIsBusBusy(RTCMK_PORT));
}

/***************************************************************************//**
 * @brief
 *   Get current content of a register.
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
int RTCMK_RegisterGet(uint8_t addr,
                      RTCMK_Register_TypeDef reg,
                      uint8_t *val)
{
  /* Configure address of Slave to talk to */
    i2cSetSlaveAdd(RTCMK_PORT, addr);

    i2cSetDirection(RTCMK_PORT, I2C_TRANSMITTER);
    i2cSetCount(RTCMK_PORT, 1);
    i2cSetMode(RTCMK_PORT, I2C_MASTER);
    i2cSetStop(RTCMK_PORT);
    i2cSetStart(RTCMK_PORT);
    i2cSend(RTCMK_PORT, 1, &reg);

    while(i2cIsBusBusy(RTCMK_PORT) == true);
    while(i2cIsStopDetected(RTCMK_PORT) == 0);
    i2cClearSCD(RTCMK_PORT);

    int temp;
    for (temp = 0; temp < 0x100; temp++);//temporary fix... don't want delay down the road

    i2cSetSlaveAdd(RTCMK_PORT, addr);
    /* Set direction to receiver */
    i2cSetDirection(RTCMK_PORT, I2C_RECEIVER);
    i2cSetCount(RTCMK_PORT, 1);
    /* Set mode as Master */
    i2cSetMode(RTCMK_PORT, I2C_MASTER);
    i2cSetStop(RTCMK_PORT);
    /* Transmit Start Condition */
    i2cSetStart(RTCMK_PORT);
    *val = i2cReceiveByte(RTCMK_PORT);

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
    while(i2cIsBusBusy(RTCMK_PORT) == true);

    /* Wait until Stop is detected */
    while(i2cIsStopDetected(RTCMK_PORT) == 0);

    /* Clear the Stop condition */
    i2cClearSCD(RTCMK_PORT);

  return((int)i2cIsBusBusy(RTCMK_PORT));
}

/***************************************************************************//**
 * @brief
 *   Write 0's to time and calender registors (0x00 to 0x06).
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ResetTime(uint8_t addr)
{


    uint8_t data[8] = {0};

    data[0] = ((uint8_t)RTCMK_RegSec) << 1;

    i2cSetSlaveAdd(RTCMK_PORT, addr);
    i2cSetDirection(RTCMK_PORT, I2C_TRANSMITTER);
    i2cSetCount(RTCMK_PORT, 8);
    i2cSetMode(RTCMK_PORT, I2C_MASTER);
    i2cSetStop(RTCMK_PORT);
    i2cSetStart(RTCMK_PORT);
    i2cSend(RTCMK_PORT, 8, data);
    while(i2cIsBusBusy(RTCMK_PORT) == true);
    while(i2cIsStopDetected(RTCMK_PORT) == 0);
    i2cClearSCD(RTCMK_PORT);

  //  while (I2C_getStatus(i2c) == i2cTransferInProgress)
  //  {
  //    /* Enter EM1 while waiting for I2C interrupt */
  //    EMU_EnterEM1();
  //    /* Could do a timeout function here. */
  //  }

    return((int)i2cIsBusBusy(RTCMK_PORT));
}

/***************************************************************************//**
 * @brief
 *   Returns current content of seconds register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadSeconds(uint8_t addr,
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
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadMinutes(uint8_t addr,
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
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit. 
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadHours(uint8_t addr,
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
 *   Returns current content of hours register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadWeek(uint8_t addr,
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
 *   Returns current content of hours register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadMonth(uint8_t addr,
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
 *   Returns current content of hours register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadYear(uint8_t addr,
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

/***************************************************************************//**
 * @brief
 *   Returns current content of hours register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadDay(uint8_t addr,
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
 *   Sets the content of day register register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetDay(uint8_t addr,
                 uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegDay,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Sets the content of hour register register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetHour(uint8_t addr,
                  uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegHour,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}
/***************************************************************************//**
 * @brief
 *   Sets the content of minute register register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetMinute(uint8_t addr,
                    uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegMin,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Sets the content of second register register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetSecond(uint8_t addr,
                    uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegSec,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Sets the content of year register register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetYear(uint8_t addr,
                  uint8_t val)
{
  int ret = -1;

  //val |= _RTCMK_DAY_DAY_MASK;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegYear,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Sets the content of week register register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetWeek(uint8_t addr,
                  uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegWeek,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of minutes alarm register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadMinutesAlarm(uint8_t addr,
                           uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegMinAlarm,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_MINALARM_MIN_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of hour alarm register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadHourAlarm(uint8_t addr,
                        uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegHourAlarm,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_HOURALARM_HOUR_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of week day alarm register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadWeekAlarm(uint8_t addr,
                        uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegWeekDayAlarm,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_WEEKDAYALARM_WEEKDAY_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of select register.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadSelect(uint8_t addr,
                     uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegSelect,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  tmp &= _RTCMK_COUNTER_COUNTER_MASK;

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}
/***************************************************************************//**
 * @brief
 *   Returns current content of flag register.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadFlag(uint8_t addr,
                   uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegFlag,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Returns current content of control register.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[out] val
 *   Reference to place result.
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_ReadControl(uint8_t addr,
                      uint8_t *val)
{
  int ret = -1;

  uint8_t tmp = 0;

  ret = RTCMK_RegisterGet(addr,RTCMK_RegControl,&tmp);
  if (ret < 0)
  {
    return(ret);
  }

  *val = ((tmp & 0xF0) >> 4) * 10 + (tmp & 0x0F);

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Sets the content of week alarm register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetWeekAlarm(uint8_t addr,
                       uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegWeekDayAlarm,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Sets the content of minute alarm register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetMinAlarm(uint8_t addr,
                      uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegMinAlarm,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}

/***************************************************************************//**
 * @brief
 *   Sets the content of hour alarm register in decimal.
 *
 * @param[in] addr
 *   I2C address, in 8 bit format, where LSB is reserved for R/W bit.
 *
 * @param[in] val
 *   Value to set to
 *
 * @return
 *   Returns 0 if registers written, <0 if unable to write to registers.
 ******************************************************************************/
int RTCMK_SetHourAlarm(uint8_t addr,
                       uint8_t val)
{
  int ret = -1;

  ret = RTCMK_RegisterSet(addr,RTCMK_RegHourAlarm,val);
  if (ret < 0)
  {
    return(ret);
  }

  return(ret);
}
