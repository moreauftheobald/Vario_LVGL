#ifndef __IO_EXTENSION_H
#define __IO_EXTENSION_H

#include "src/i2c/i2c.h"

typedef struct _io_extension_obj_t {
  i2c_master_dev_handle_t addr;
  uint8_t Last_io_value;
  uint8_t Last_od_value;
} io_extension_obj_t;

void IO_EXTENSION_Init();

void IO_EXTENSION_Output(uint8_t pin, uint8_t value);

uint8_t IO_EXTENSION_Input(uint8_t pin);

void IO_EXTENSION_Pwm_Output(uint8_t Value);

uint16_t IO_EXTENSION_Adc_Input();

#endif