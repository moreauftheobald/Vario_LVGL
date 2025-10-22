#ifndef __I2C_H
#define __I2C_H

#include <stdio.h>
#include <string.h>
#include "driver/i2c_master.h"
#include "esp_log.h"


typedef struct {
  i2c_master_bus_handle_t bus;
  i2c_master_dev_handle_t dev;
} DEV_I2C_Port;

DEV_I2C_Port DEV_I2C_Init();

void DEV_I2C_Set_Slave_Addr(i2c_master_dev_handle_t *dev_handle, uint8_t Addr);

void DEV_I2C_Write_Byte(i2c_master_dev_handle_t dev_handle, uint8_t Cmd, uint8_t value);

uint8_t DEV_I2C_Read_Byte(i2c_master_dev_handle_t dev_handle);

uint16_t DEV_I2C_Read_Word(i2c_master_dev_handle_t dev_handle, uint8_t Cmd);

void DEV_I2C_Write_Nbyte(i2c_master_dev_handle_t dev_handle, uint8_t *pdata, uint8_t len);

void DEV_I2C_Read_Nbyte(i2c_master_dev_handle_t dev_handle, uint8_t Cmd, uint8_t *pdata, uint8_t len);

// Ajouter a la fin du fichier i2c.h
/**
 * @brief Get the existing I2C bus handle
 * @return The I2C bus handle
 */
DEV_I2C_Port DEV_I2C_Get_Handle();

#endif