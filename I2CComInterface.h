#ifndef __I2C_COM_INTERFACE_H__
#define __I2C_COM_INTERFACE_H__

//**********************************************************************
//* I2C_COM_INTERFACE Includes
//**********************************************************************
#include "Compiler.h"
#include "HardwareProfile.h"
#include "SystemLog.h"

//**********************************************************************
//* I2C_COM_INTERFACE Datatypes
//**********************************************************************

typedef struct {
    I2C_MODULE module;
    I2C_CONFIGURATION config;
    UINT32 clockFrequency;
    I2C_7_BIT_ADDRESS controlCode;
} I2C_COM_INTERFACE_CONTROL, * I2C_COM_INTERFACE_CONTROL_PTR;

//**********************************************************************
//* I2C_COM_INTERFACE Set and Get Functions
//**********************************************************************

void I2CComInterface_SetModule(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, I2C_MODULE module);
I2C_MODULE I2CComInterface_GetModule(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);

void I2CComInterface_SetConfig(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, I2C_CONFIGURATION config);
I2C_CONFIGURATION I2CComInterface_GetConfig(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);

void I2CComInterface_SetClockFrequency(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, UINT32 clockFrequency);
UINT32 I2CComInterface_GetClockFrequency(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);

void I2CComInterface_SetSlaveAddress(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE eepromAddress);
BYTE I2CComInterface_GetSlaveAddress(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);

void I2CComInterface_SetControlCommand(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE command);
BYTE I2CComInterface_GetControlCommand(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);

void I2CComInterface_SetControlCode(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE eepromAddress, BYTE command);
BYTE I2CComInterface_GetControlCode(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);

//**********************************************************************
//* I2C_COM_INTERFACE Functions
//**********************************************************************

void I2CComInterface_Init(  I2C_COM_INTERFACE_CONTROL_PTR i2cControl,
                            I2C_MODULE module,
                            I2C_CONFIGURATION config,
                            UINT32 clockFrequency,
                            BYTE slaveAddress);
BOOL I2CComInterface_StarTransfer(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BOOL restart);
void I2CComInterface_StopTransfer(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);
BOOL I2CComInterface_GetByte(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE * receivedData, BOOL ack);
BOOL I2CComInterface_PutByte(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE data);
BOOL I2CComInterface_AcknowledgePolling(I2C_COM_INTERFACE_CONTROL_PTR i2cControl);

void I2CComInterface_ReceiveData(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE * buffer, WORD bufferSize);
void I2CComInterface_SendData(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE * buffer, WORD bufferSize, BOOL startTransfer, BOOL stopTransfer);

#endif /* __I2C_COM_INTERFACE_H__ */
