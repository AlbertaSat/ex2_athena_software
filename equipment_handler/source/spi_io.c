#include "spi_io.h"
#include "FreeRTOS.h"
#include "HL_gio.h"
#include "HL_het.h"
#include "HL_mibspi.h"
#include "HL_spi.h"
#include "os_semphr.h"
#include "system.h"

/******************************************************************************
 Module Public Functions - Low level SPI control functions
******************************************************************************/
static SemaphoreHandle_t sd_block;

void SPI_Init(void) { sd_block = xSemaphoreCreateBinary(); }

static spiDAT1_t sd_config = {false, false, SPI_FMT_0, SPI_CS_NONE};
//#define SD_INTERRUPT
bool SPI_SendReceive(uint32_t size, uint8_t *src, uint8_t *dest) {
#ifdef SD_INTERRUPT
    spiSendAndGetData(SD_SPI, &sd_config, size, src, dest);
    if (xSemaphoreTake(sd_block, 1000) != pdTRUE) {
        return false;
    }
#else
    spiTransmitAndReceiveData(SD_SPI, &sd_config, size, src, dest);
#endif
    return true;
}

bool SPI_Send(uint32_t size, uint8_t *src) {
#ifdef SD_INTERRUPT
    spiSendData(SD_SPI, &sd_config, size, src);
    if (xSemaphoreTake(sd_block, 1000) != pdTRUE) {
        return false;
    }
#else
    spiTransmitData(SD_SPI, &sd_config, size, src);
#endif
    return true;
}

bool SPI_Receive(uint32_t size, uint8_t *dest) {
#ifdef SD_INTERRUPT
    spiGetData(SD_SPI, &sd_config, size, dest);
    if (xSemaphoreTake(sd_block, portMAX_DELAY) != pdTRUE) {
        return false;
    }
#else
    spiReceiveData(SD_SPI, &sd_config, size, dest);
#endif
    return true;
}

void sd_spiEndNotification(spiBASE_t *spi) {
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sd_block, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void SPI_Release(void) {
    WORD idx;
    uint8_t r = 0;
    for (idx = 512; idx && (r != 0xFF); idx--)
        SPI_Receive(1, &r);
}

inline void SPI_CS_Low(uint8_t bVolNum) {
    if (bVolNum == 0) {
#ifdef IS_ATHENA
        gioSetBit(gioPORTA, 3, 0); // CS LOW
        gioSetBit(hetPORT2, 6, 1); // CS HIGH
#else
        gioSetBit(hetPORT1, 12, 0); // CS LOW
        gioSetBit(hetPORT1, 14, 1); // CS HIGH

#endif
    } else if (bVolNum == 1) {
#ifdef IS_ATHENA
        gioSetBit(hetPORT2, 6, 0); // CS LOW
        gioSetBit(gioPORTA, 3, 1); // CS HIGH
#else
        gioSetBit(hetPORT1, 12, 1); // CS LOW
        gioSetBit(hetPORT1, 14, 0); // CS HIGH
#endif
    }
}

inline void SPI_CS_High(uint8_t bVolNum) {
#ifdef IS_ATHENA
    gioSetBit(gioPORTA, 3, 1); // CS HIGH
    gioSetBit(hetPORT2, 6, 1); // CS HIGH
#else
    gioSetBit(hetPORT1, 12, 1);     // CS HIGH
    gioSetBit(hetPORT1, 14, 1);     // CS HIGH
#endif
}

inline void SPI_Freq_High(void) {}

inline void SPI_Freq_Low(void) {}

void SPI_Timer_On(WORD ms) {
    //    SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;  // Make sure clock is enabled
    //    LPTMR0_CSR = 0;                     // Reset LPTMR settings
    //    LPTMR0_CMR = ms;                    // Set compare value (in ms)
    //    // Use 1kHz LPO with no prescaler
    //    LPTMR0_PSR = LPTMR_PSR_PCS(1) | LPTMR_PSR_PBYP_MASK;
    //    // Start the timer and wait for it to reach the compare value
    //    LPTMR0_CSR = LPTMR_CSR_TEN_MASK;
}

inline BOOL SPI_Timer_Status(void) { return TRUE; }

inline void SPI_Timer_Off(void) {
    //    LPTMR0_CSR = 0;                     // Turn off timer
}
