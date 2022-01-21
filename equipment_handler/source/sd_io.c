/*
 *  File: sd_io.c
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */

#include "sd_io.h"
#define FALSE 0
#define TRUE 1
#define HIGH 1

/******************************************************************************
 Private Methods Prototypes - Direct work with SD card
******************************************************************************/

SD_DEV devs[2];

/**
    \brief Simple function to calculate power of two.
    \param e Exponent.
    \return Math function result.
*/
DWORD __SD_Power_Of_Two(BYTE e);

/**
     \brief Assert the SD card (SPI CS low).
 */
inline void __SD_Assert(uint8_t bVolNum);

/**
    \brief Deassert the SD (SPI CS high).
 */
inline void __SD_Deassert(uint8_t bVolNum);

/**
    \brief Change to max the speed transfer.
    \param throttle
 */
void __SD_Speed_Transfer(BYTE throttle);

/**
    \brief Send SPI commands.
    \param cmd Command to send.
    \param arg Argument to send.
    \return R1 response.
 */
BYTE __SD_Send_Cmd(uint8_t bVolNum, BYTE cmd, DWORD arg);

/**
    \brief Write a data block on SD card.
    \param dat Storage the data to transfer.
    \param token Inidicates the type of transfer (single or multiple).
 */
SDRESULTS __SD_Write_Block(uint8_t bVolNum, const void *dat, BYTE token);

/**
    \brief Get the total numbers of sectors in SD card.
    \param dev Device descriptor.
    \return Quantity of sectors. Zero if fail.
 */
DWORD __SD_Sectors(uint8_t bVolNum);

/******************************************************************************
 Private Methods - Direct work with SD card
******************************************************************************/

DWORD __SD_Power_Of_Two(BYTE e) {
    DWORD partial = 1;
    BYTE idx;
    for (idx = 0; idx != e; idx++)
        partial *= 2;
    return (partial);
}

inline void __SD_Assert(uint8_t bVolNum) { SPI_CS_Low(bVolNum); }

inline void __SD_Deassert(uint8_t bVolNum) { SPI_CS_High(bVolNum); }

void __SD_Speed_Transfer(BYTE throttle) {
    if (throttle == HIGH)
        SPI_Freq_High();
    else
        SPI_Freq_Low();
}

BYTE __SD_Send_Cmd(uint8_t bVolNum, BYTE cmd, DWORD arg) {
    BYTE crc, res;
    // ACMD«n» is the command sequence of CMD55-CMD«n»
    if (cmd & 0x80) {
        cmd &= 0x7F;
        res = __SD_Send_Cmd(bVolNum, CMD55, 0);
        if (res > 1)
            return (res);
    }

    // Select the card
    uint8_t b = 0xFF; // Dummy variable;
    __SD_Deassert(bVolNum);
    SPI_Send(1, &b);
    __SD_Assert(bVolNum);
    SPI_Send(1, &b);

    // Send complete command set
    uint8_t cmd_arr[6] = {0};
    cmd_arr[0] = cmd;
    cmd_arr[1] = (uint8_t)(arg >> 24);
    cmd_arr[2] = (uint8_t)(arg >> 16);
    cmd_arr[3] = (uint8_t)(arg >> 8);
    cmd_arr[4] = (uint8_t)(arg >> 0);

    // CRC?
    crc = 0x01; // Dummy CRC and stop
    if (cmd == CMD0)
        crc = 0x95; // Valid CRC for CMD0(0)
    if (cmd == CMD8)
        crc = 0x87; // Valid CRC for CMD8(0x1AA)
    cmd_arr[5] = crc;

    SPI_Send(6, cmd_arr);

    // Receive command response
    // Wait for a valid response in timeout of 5 milliseconds
    SPI_Timer_On(5);
    do {
        SPI_Receive(1, &res);
    } while ((res & 0x80) && (SPI_Timer_Status() == TRUE));
    SPI_Timer_Off();
    // Return with the response value
    return (res);
}

SDRESULTS __SD_Write_Block(uint8_t bVolNum, const void *dat, BYTE token) {
    BYTE line;
    // Send token (single or multiple)
    SPI_Send(1, &token);
    // Single block write?
    if (token != 0xFD) {

        // Send block data
        SPI_Send(SD_BLK_SIZE, (uint8_t *)dat);

        /* Dummy CRC */
        uint8_t crc[2] = {0xFF};
        SPI_Send(2, crc);
        // If not accepted, returns the reject error
        uint8_t res;
        SPI_Receive(1, &res);
        if ((res & 0x1F) != 0x05)
            return (SD_REJECT);
    }
    // Waits until finish of data programming with a timeout
    SPI_Timer_On(SD_IO_WRITE_TIMEOUT_WAIT);
    do {
        SPI_Receive(1, &line);
    } while ((line == 0) && (SPI_Timer_Status() == TRUE));
    SPI_Timer_Off();
    if (line == 0)
        return (SD_BUSY);
    else
        return (SD_OK);
}

DWORD __SD_Sectors(uint8_t bVolNum) {
    SD_DEV *dev = bVolNum ? &devs[0] : &devs[1];
    BYTE csd[18];
    DWORD ss = 0;
    WORD C_SIZE = 0;
    BYTE C_SIZE_MULT = 0;
    BYTE READ_BL_LEN = 0;
    if (__SD_Send_Cmd(bVolNum, CMD9, 0) == 0) {
        // Wait for response
        BYTE b;
        do {
            SPI_Receive(1, &b);
        } while (b == 0xFF);

        SPI_Receive(18, csd);

        SPI_Release();
        if (dev->cardtype & SDCT_SD1) {
            ss = csd[0];
            // READ_BL_LEN[83:80]: max. read data block length
            READ_BL_LEN = (csd[5] & 0x0F);
            // C_SIZE [73:62]
            C_SIZE = (csd[6] & 0x03);
            C_SIZE <<= 8;
            C_SIZE |= (csd[7]);
            C_SIZE <<= 2;
            C_SIZE |= ((csd[8] >> 6) & 0x03);
            // C_SIZE_MULT [49:47]
            C_SIZE_MULT = (csd[9] & 0x03);
            C_SIZE_MULT <<= 1;
            C_SIZE_MULT |= ((csd[10] >> 7) & 0x01);
        } else if (dev->cardtype & SDCT_SD2) {
            // C_SIZE [69:48]
            C_SIZE = (csd[7] & 0x3F);
            C_SIZE <<= 8;
            C_SIZE |= (csd[8] & 0xFF);
            C_SIZE <<= 8;
            C_SIZE |= (csd[9] & 0xFF);
            // C_SIZE_MULT [--]. don't exits
            C_SIZE_MULT = 0;
        }
        ss = (C_SIZE + 1);
        ss *= __SD_Power_Of_Two(C_SIZE_MULT + 2);
        ss *= __SD_Power_Of_Two(READ_BL_LEN);
        ss /= SD_BLK_SIZE;
        return (ss);
    } else
        return (0); // Error
}

/******************************************************************************
 Public Methods - Direct work with SD card
******************************************************************************/

SDRESULTS SD_Init(uint8_t bVolNum) {
    SD_DEV *dev = bVolNum ? &devs[0] : &devs[1];
    BYTE n, cmd, ct, ocr[4];
    BYTE init_trys;
    ct = 0;
    for (init_trys = 0; ((init_trys != SD_INIT_TRYS) && (!ct)); init_trys++) {
        // Initialize SPI for use with the memory card
        SPI_Init();

        SPI_CS_High(bVolNum);
        SPI_Freq_Low();

        // 80 dummy clocks
        uint8_t dummy[10] = {0xFF};
        SPI_Send(10, dummy);
        //        SPI_Timer_On(500);
        //        while(SPI_Timer_Status()==TRUE);
        //        SPI_Timer_Off();

        dev->mount = FALSE;
        SPI_Timer_On(500);
        while ((__SD_Send_Cmd(bVolNum, CMD0, 0) != 1) && (SPI_Timer_Status() == TRUE))
            ;
        SPI_Timer_Off();
        // Idle state
        if (__SD_Send_Cmd(bVolNum, CMD0, 0) == 1) {
            // SD version 2?
            if (__SD_Send_Cmd(bVolNum, CMD8, 0x1AA) == 1) {
                // Get trailing return value of R7 resp
                SPI_Receive(4, ocr);
                // VDD range of 2.7-3.6V is OK?
                if ((ocr[2] == 0x01) && (ocr[3] == 0xAA)) {
                    // Wait for leaving idle state (ACMD41 with HCS bit)...
                    SPI_Timer_On(1000);
                    while ((SPI_Timer_Status() == TRUE) && (__SD_Send_Cmd(bVolNum, ACMD41, 1UL << 30)))
                        ;
                    SPI_Timer_Off();
                    // CCS in the OCR?
                    if ((SPI_Timer_Status() == TRUE) && (__SD_Send_Cmd(bVolNum, CMD58, 0) == 0)) {
                        SPI_Receive(4, ocr);
                        // SD version 2?
                        ct = (ocr[0] & 0x40) ? SDCT_SD2 | SDCT_BLOCK : SDCT_SD2;
                    }
                }
            } else {
                // SD version 1 or MMC?
                if (__SD_Send_Cmd(bVolNum, ACMD41, 0) <= 1) {
                    // SD version 1
                    ct = SDCT_SD1;
                    cmd = ACMD41;
                } else {
                    // MMC version 3
                    ct = SDCT_MMC;
                    cmd = CMD1;
                }
                // Wait for leaving idle state
                SPI_Timer_On(250);
                while ((SPI_Timer_Status() == TRUE) && (__SD_Send_Cmd(bVolNum, cmd, 0)))
                    ;
                SPI_Timer_Off();
                if (SPI_Timer_Status() == FALSE)
                    ct = 0;
                if (__SD_Send_Cmd(bVolNum, CMD59, 0))
                    ct = 0; // Deactivate CRC check (default)
                if (__SD_Send_Cmd(bVolNum, CMD16, 512))
                    ct = 0; // Set R/W block length to 512 bytes
            }
        }
    }
    if (ct) {
        dev->cardtype = ct;
        dev->mount = TRUE;
        dev->last_sector = __SD_Sectors(bVolNum) - 1;
        dev->last_sector = 3717120;

        __SD_Speed_Transfer(HIGH); // High speed transfer
    }
    SPI_Release();
    return (ct ? SD_OK : SD_NOINIT);
}

SDRESULTS SD_Read(uint8_t bVolNum, void *dat, DWORD sector, WORD ofs, WORD cnt) {
    SD_DEV *dev = bVolNum ? &devs[0] : &devs[1];
    SDRESULTS res;
    BYTE tkn;
    WORD remaining;
    res = SD_ERROR;
    if ((sector > dev->last_sector) || (cnt == 0))
        return (SD_PARERR);
    // Convert sector number to byte address (sector * SD_BLK_SIZE)
    if (__SD_Send_Cmd(bVolNum, CMD17, sector * SD_BLK_SIZE) == 0) {
        SPI_Timer_On(100); // Wait for data packet (timeout of 100ms)
        do {
            SPI_Receive(1, &tkn);
        } while ((tkn == 0xFF) && (SPI_Timer_Status() == TRUE));
        SPI_Timer_Off();
        // Token of single block?
        if (tkn == 0xFE) {
            SPI_Receive(cnt, dat);
            // Skip remaining
            remaining = 2;
            do {
                SPI_Receive(1, &tkn);
            } while (--remaining);
            res = SD_OK;
        }
    }
    SPI_Release();
    return (res);
}

SDRESULTS SD_Write(uint8_t bVolNum, const void *dat, DWORD sector) {
    // Convert sector number to bytes address (sector * SD_BLK_SIZE)
    if (__SD_Send_Cmd(bVolNum, CMD24, sector * SD_BLK_SIZE) == 0)
        return (__SD_Write_Block(bVolNum, dat, 0xFE));
    else
        return (SD_ERROR);
}

SDRESULTS SD_Status(uint8_t bVolNum) { return (__SD_Send_Cmd(bVolNum, CMD0, 0) ? SD_OK : SD_NORESPONSE); }

// «sd_io.c» is part of:
/*----------------------------------------------------------------------------/
/  ulibSD - Library for SD cards semantics            (C)Nelson Lombardo, 2015
/-----------------------------------------------------------------------------/
/ ulibSD library is a free software that opened under license policy of
/ following conditions.
/
/ Copyright (C) 2015, ChaN, all right reserved.
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/

// Derived from Mister Chan works on FatFs code (http://elm-chan.org/fsw/ff/00index_e.html):
/*----------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.11                 (C)ChaN, 2015
/-----------------------------------------------------------------------------/
/ FatFs module is a free software that opened under license policy of
/ following conditions.
/
/ Copyright (C) 2015, ChaN, all right reserved.
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/
