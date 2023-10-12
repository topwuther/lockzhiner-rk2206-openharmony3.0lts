//-- Includes -----------------------------------------------------------------
#include "bh1750.h"

//-- Defines ------------------------------------------------------------------
// I2C bus number
#define I2C_BUS 0

//-- Global variables ---------------------------------------------------------
static u8t _i2cAddress; // I2C Address
static I2cBusIo m_i2cBus = {
    .scl = {
        .gpio = GPIO0_PA1,
        .func = MUX_FUNC3,
        .type = PULL_NONE,
        .drv = DRIVE_KEEP,
        .dir = LZGPIO_DIR_KEEP,
        .val = LZGPIO_LEVEL_KEEP},
    .sda = {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};

static unsigned int m_i2c_freq = 400000;

//-- Static function prototypes -----------------------------------------------
static etError BH1750_WriteCommand(bhCommands command);

//-----------------------------------------------------------------------------
void BH1750_Init(u8t i2cAddress) /* -- adapt the init for your uC -- */
{
  BH1750_SetI2cAdr(i2cAddress);
  /* 初始化i2c */
  I2cIoInit(m_i2cBus);
  LzI2cInit(I2C_BUS, m_i2c_freq);

  PinctrlSet(GPIO0_PA1, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);
  PinctrlSet(GPIO0_PA0, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);

}

//-----------------------------------------------------------------------------
void BH1750_SetI2cAdr(u8t i2cAddress)
{
  _i2cAddress = i2cAddress;
}

static etError BH1750_WriteCommand(bhCommands command)
{
  etError error; // error code

  u8t send_buffer[1] = {command};
  error = LzI2cWrite(I2C_BUS, _i2cAddress, send_buffer, 1);
  return error;
}

//-----------------------------------------------------------------------------
etError BH1750_StartPeriodicMeasurment()
{
  etError error; // error code

  error = BH1750_WriteCommand(CMD_MEAS_PERI_HR_M);

  return error;
}

//-----------------------------------------------------------------------------
etError BH1750_PowerOn()
{
  etError error; // error code

  error = BH1750_WriteCommand(CMD_POWERON);

  return error;
}

etError BH1750_ReadMeasurementBuffer(ft *light)
{
  etError error;     // error code
  uint8_t recv_data[2] = {0};;
  error = LzI2cRead(I2C_BUS,_i2cAddress,recv_data,2);
  *light = (float)(((recv_data[0]<<8)|recv_data[1])/1.2);
  return error;
}
