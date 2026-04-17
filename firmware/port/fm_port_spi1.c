/**
 * @file    fm_port_spi1.c
 * @brief   Platform SPI1 helpers for device transport.
 */

#include "fm_port_spi1.h"

#include "main.h"

/* Private Data */
static SPI_HandleTypeDef hspi1;

/* Private Prototypes */
static SPI_HandleTypeDef *fm_port_spi1_handle_get_(void);

/* Private Bodies */
static SPI_HandleTypeDef *fm_port_spi1_handle_get_(void)
{
    return &hspi1;
}

void FM_PORT_Spi1_Init(void)
{
    SPI_HandleTypeDef *spi_handle = fm_port_spi1_handle_get_();
    SPI_AutonomousModeConfTypeDef auto_mode_cfg = { 0 };

    spi_handle->Instance = SPI1;
    spi_handle->Init.Mode = SPI_MODE_MASTER;
    spi_handle->Init.Direction = SPI_DIRECTION_2LINES;
    spi_handle->Init.DataSize = SPI_DATASIZE_8BIT;
    spi_handle->Init.CLKPolarity = SPI_POLARITY_LOW;
    spi_handle->Init.CLKPhase = SPI_PHASE_1EDGE;
    spi_handle->Init.NSS = SPI_NSS_SOFT;
    spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    spi_handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi_handle->Init.TIMode = SPI_TIMODE_DISABLE;
    spi_handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_handle->Init.CRCPolynomial = 0x7;
    spi_handle->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    spi_handle->Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    spi_handle->Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    spi_handle->Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    spi_handle->Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    spi_handle->Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    spi_handle->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    spi_handle->Init.IOSwap = SPI_IO_SWAP_DISABLE;
    spi_handle->Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
    spi_handle->Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;

    if (HAL_SPI_Init(spi_handle) != HAL_OK)
    {
        Error_Handler();
    }

    auto_mode_cfg.TriggerState = SPI_AUTO_MODE_DISABLE;
    auto_mode_cfg.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
    auto_mode_cfg.TriggerPolarity = SPI_TRIG_POLARITY_RISING;

    if (HAL_SPIEx_SetConfigAutonomousMode(spi_handle, &auto_mode_cfg) != HAL_OK)
    {
        Error_Handler();
    }
}

bool FM_PORT_Spi1_Write(const uint8_t *p_data, uint16_t len, uint32_t timeout_ms)
{
    if ((p_data == NULL) || (len == 0U))
    {
        return false;
    }

    return (HAL_SPI_Transmit(fm_port_spi1_handle_get_(), (uint8_t *) p_data, len, timeout_ms) == HAL_OK);
}
