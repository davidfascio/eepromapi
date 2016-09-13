//**********************************************************************
//* I2C_COM_INTERFACE Includes
//**********************************************************************
#include "I2CComInterface.h"

//**********************************************************************
//* I2C_COM_INTERFACE Set and Get Functions
//**********************************************************************

void I2CComInterface_SetModule(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, I2C_MODULE module) {

    if (i2cControl == NULL)
        return;

    i2cControl->module = module;
}

I2C_MODULE I2CComInterface_GetModule(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    return i2cControl->module;
}

void I2CComInterface_SetConfig(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, I2C_CONFIGURATION config) {

    if (i2cControl == NULL)
        return;

    i2cControl->config = config;
}

I2C_CONFIGURATION I2CComInterface_GetConfig(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    return i2cControl->config;
}

void I2CComInterface_SetClockFrequency(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, UINT32 clockFrequency) {

    if (i2cControl == NULL)
        return;

    i2cControl->clockFrequency = clockFrequency;
}

UINT32 I2CComInterface_GetClockFrequency(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    return i2cControl->clockFrequency;
}

void I2CComInterface_SetSlaveAddress(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE eepromAddress) {

    if (i2cControl == NULL)
        return;

    i2cControl->controlCode.address = eepromAddress;
}

BYTE I2CComInterface_GetSlaveAddress(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    return i2cControl->controlCode.address;
}

void I2CComInterface_SetControlCommand(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE command) {

    if (i2cControl == NULL)
        return;

    i2cControl->controlCode.rw = command;
}

BYTE I2CComInterface_GetControlCommand(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    return i2cControl->controlCode.rw;
}

void I2CComInterface_SetControlCode(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE eepromAddress, BYTE command) {

    if (i2cControl == NULL)
        return;

    I2CComInterface_SetSlaveAddress(i2cControl, eepromAddress);
    I2CComInterface_SetControlCommand(i2cControl, command);
}

BYTE I2CComInterface_GetControlCode(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    return i2cControl->controlCode.byte;
}

//**********************************************************************
//* I2C_COM_INTERFACE Functions
//**********************************************************************

void I2CComInterface_Init(  I2C_COM_INTERFACE_CONTROL_PTR i2cControl,
                            I2C_MODULE module,
                            I2C_CONFIGURATION config,
                            UINT32 clockFrequency,
                            BYTE slaveAddress) {

    UINT32 actualClock;

    if (i2cControl == NULL)
        return; //

    I2CComInterface_SetModule(i2cControl, module);
    I2CComInterface_SetConfig(i2cControl, config);
    I2CComInterface_SetClockFrequency(i2cControl, clockFrequency);
    I2CComInterface_SetSlaveAddress(i2cControl, slaveAddress);


    I2CConfigure(I2CComInterface_GetModule(i2cControl), I2CComInterface_GetConfig(i2cControl));

    actualClock = I2CSetFrequency(I2CComInterface_GetModule(i2cControl), GetPeripheralClock(), I2CComInterface_GetClockFrequency(i2cControl));

    if (abs(actualClock - I2CComInterface_GetClockFrequency(i2cControl)) > I2CComInterface_GetClockFrequency(i2cControl) / 10) {
        println_error(SYSTEM_LOG_I2C_ID, "I2C clock frequency (%ld Hz)  error exceeds 10%%", actualClock);
    } else {
        println_debug(SYSTEM_LOG_I2C_ID, "I2C clock frequency set to (%ld Hz)", actualClock);
    }

    I2CEnable(I2CComInterface_GetModule(i2cControl), TRUE);
}

BOOL I2CComInterface_StarTransfer(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BOOL restart) {

    I2C_STATUS status;
    I2C_MODULE module;

    if (i2cControl == NULL)
        return FALSE;

    module = I2CComInterface_GetModule(i2cControl);

    I2CClearStatus(module, I2C_START);

    if (restart) {

        if (I2CRepeatStart(module) != I2C_SUCCESS) {

            println_error(SYSTEM_LOG_I2C_ID, "Bus Collision during starting trasnfer process");
            return FALSE;
        }

    } else {

        while (!I2CBusIsIdle(module));

        if (I2CStart(module) != I2C_SUCCESS) {

            println_error(SYSTEM_LOG_I2C_ID, "Bus Collision during starting trasnfer process");
            return FALSE;
        }
    }

    while (!(status & I2C_START)) {

        status = I2CGetStatus(module);
    }

    return TRUE;
}

void I2CComInterface_StopTransfer(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    I2C_STATUS status;
    I2C_MODULE module;

    if (i2cControl == NULL)
        return; //

    module = I2CComInterface_GetModule(i2cControl);

    I2CClearStatus(module, I2C_STOP);

    I2CStop(module);

    do {
        status = I2CGetStatus(module);

    } while (!(status & I2C_STOP));
}

BOOL I2CComInterface_GetByte(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE * receivedData, BOOL ack) {

    I2C_MODULE module;

    if (i2cControl == NULL)
        return FALSE; //

    module = I2CComInterface_GetModule(i2cControl);

    if (I2CReceiverEnable(module, TRUE) != I2C_SUCCESS) {

        return FALSE;
    }

    while (I2CReceivedDataIsAvailable(module) != TRUE);
    
    DISABLE_WDT();
    asm("di");
    * receivedData = I2CGetByte(module);
    asm("ei");
    CLRWDT();
    ENABLE_WDT();

    I2CAcknowledgeByte(module, ack);

    while (I2CAcknowledgeHasCompleted(module) != TRUE);

    return TRUE;
}

BOOL I2CComInterface_PutByte(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE data) {

    I2C_MODULE module;
    I2C_RESULT error_code;

    if (i2cControl == NULL)
        return FALSE; //

    module = I2CComInterface_GetModule(i2cControl);

    while (!I2CTransmitterIsReady(module));

    DISABLE_WDT();
    asm("di");
    error_code = I2CSendByte(module, data);
    asm("ei");
    CLRWDT();
    ENABLE_WDT();

    if ( error_code != I2C_SUCCESS) {

        return FALSE;
    }

    while (!I2CTransmissionHasCompleted(module));

    return I2CByteWasAcknowledged(module);
}

BOOL I2CComInterface_AcknowledgePolling(I2C_COM_INTERFACE_CONTROL_PTR i2cControl) {

    BOOL acknowledgeHasArrived = FALSE;
    I2C_MODULE module;
    BYTE controlCode;

    if (i2cControl == NULL)
        return FALSE; //

    module = I2CComInterface_GetModule(i2cControl);
    controlCode = I2CComInterface_GetControlCode(i2cControl);

    while (acknowledgeHasArrived != TRUE) {

        if (I2CComInterface_StarTransfer(i2cControl, FALSE) == FALSE) {

            return FALSE;
        }

        acknowledgeHasArrived = I2CComInterface_PutByte(i2cControl, controlCode);

        I2CComInterface_StopTransfer(i2cControl);
    }

    return TRUE;
}

void I2CComInterface_ReceiveData(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE * buffer, WORD bufferSize) {

    WORD index;
    BYTE receivedData;
    BYTE controlCode;
    BYTE * buffer_ptr = buffer;

    if (i2cControl == NULL)
        return; //

    if (!I2CComInterface_StarTransfer(i2cControl, TRUE)) {

        println_error(SYSTEM_LOG_I2C_ID, "it couldnt start tx");
        return;
    }

    I2CComInterface_SetControlCommand(i2cControl, I2C_READ);

    controlCode = I2CComInterface_GetControlCode(i2cControl);

    if (!I2CComInterface_PutByte(i2cControl, controlCode)) {

        println_error(SYSTEM_LOG_I2C_ID, "it couldnt send read command");
        return;
    }

    for (index = 0; index < bufferSize; index++) {

        if (!I2CComInterface_GetByte(i2cControl, &receivedData, index < (bufferSize - 1))) {

            println_error(SYSTEM_LOG_I2C_ID, "index: %d", index);
            return;
        }

        *buffer_ptr = receivedData;
        buffer_ptr++;
    }

    I2CComInterface_StopTransfer(i2cControl);


    println_log(SYSTEM_LOG_I2C_ID, "Data Read (%d Bytes): ", bufferSize);
    //Com485Interface_PrintData(buffer, bufferSize);
    print_buffer_log(SYSTEM_LOG_I2C_ID, buffer, bufferSize);
}

void I2CComInterface_SendData(I2C_COM_INTERFACE_CONTROL_PTR i2cControl, BYTE * buffer, WORD bufferSize, BOOL startTransfer, BOOL stopTransfer) {

    WORD index;

    if (i2cControl == NULL)
        return;

    println_log(SYSTEM_LOG_I2C_ID, "Send Data (%d Bytes):", bufferSize);
    //! Com485Interface_PrintData(buffer, bufferSize);
    print_buffer_log(SYSTEM_LOG_I2C_ID, buffer, bufferSize);

    if (startTransfer == TRUE) {

        if (!I2CComInterface_StarTransfer(i2cControl, FALSE)) {

            println_error(SYSTEM_LOG_I2C_ID, "It couldnt start tx");
            return;
        }
    }

    for (index = 0; index < bufferSize; index++) {

        if (!I2CComInterface_PutByte(i2cControl, * buffer)) {

            println_error(SYSTEM_LOG_I2C_ID, "index: %d", index);
            return;
        }

        buffer++;
    }

    if (stopTransfer == TRUE) {

        I2CComInterface_StopTransfer(i2cControl);
    }
}
