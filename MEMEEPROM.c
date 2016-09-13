#include "MEMEEPROM.h"
#include "ComInterfaces/Com485Interface.h"
#include "zNVM.h"

//**********************************************************************
//* MEM_EEPROM Variables
//**********************************************************************
I2C_COM_INTERFACE_CONTROL eepromComInterfaceControl;

//**********************************************************************
//* MEM_EEPROM Functions
//**********************************************************************
void MEM_EEPROM_Init(void) {

    I2CComInterface_Init(&eepromComInterfaceControl,
                        MEM_EEPROM_I2C_HARDWARE_MODULE,
                        MEM_EEPROM_I2C_HARDWARE_CONFIGURATION,
                        MEM_EEPROM_I2C_HARDWARE_CLOCK_FREQUENCY,
                        MEM_EEPROM_I2C_HARDWARE_SLAVE_ADDRESS);
}

WORD MEM_EEPROM_BuildHeader(WORD address, BYTE * buffer, WORD bufferSize) {

    BYTE * eepromHeader_ptr = buffer;

    I2CComInterface_SetControlCommand(&eepromComInterfaceControl, I2C_WRITE);

    * eepromHeader_ptr = I2CComInterface_GetControlCode(&eepromComInterfaceControl);
    eepromHeader_ptr++;

    MemReverseCopy((BYTE *) & address, eepromHeader_ptr, sizeof (address));
    eepromHeader_ptr += sizeof (address);

    return eepromHeader_ptr - buffer;
}

void MEM_EEPROM_ReadPage(WORD address, BYTE * buffer, WORD bufferSize) {

    BYTE eepromHeader[MEM_EEPROM_HEADER_SIZE];
    WORD eepromHeaderSize;

    println_info(SYSTEM_LOG_MEMEEPROM_ID, "Read Page Data into address: %04X, size: %d", address, bufferSize);

    eepromHeaderSize = MEM_EEPROM_BuildHeader(address, eepromHeader, sizeof (eepromHeader));
    
    I2CComInterface_SendData(&eepromComInterfaceControl, eepromHeader, eepromHeaderSize, TRUE, FALSE);
    I2CComInterface_ReceiveData(&eepromComInterfaceControl, buffer, bufferSize);
}

void MEM_EEPROM_Read(WORD address, BYTE * buffer, WORD bufferSize) {

    /*WORD bufferPageSize = MEM_EEPROM_MAX_BUFFER_PAGE_SIZE;

    print_info("Read Data into address: %04X, Total Size: %d", address, bufferSize);

    while(bufferSize){

        if(bufferSize < bufferPageSize)
            bufferPageSize = bufferSize;

        MEM_EEPROM_ReadPage(address, buffer, bufferPageSize);

        bufferSize -= bufferPageSize;
        buffer += bufferPageSize;
        address += bufferPageSize;
    }*/

    MEM_EEPROM_ReadPage(address, buffer, bufferSize);
}

void MEM_EEPROM_WritePage(WORD address, BYTE * buffer, WORD bufferSize) {

    BYTE eepromHeader[MEM_EEPROM_HEADER_SIZE];
    WORD eepromHeaderSize;  

    println_info(SYSTEM_LOG_MEMEEPROM_ID, "Write Page Data into address: %04X, size: %d", address, bufferSize);
    eepromHeaderSize = MEM_EEPROM_BuildHeader(address, eepromHeader, sizeof (eepromHeader));

    I2CComInterface_SendData(&eepromComInterfaceControl, eepromHeader, eepromHeaderSize, TRUE, FALSE);
    I2CComInterface_SendData(&eepromComInterfaceControl, buffer, bufferSize, FALSE, TRUE);
    I2CComInterface_AcknowledgePolling(&eepromComInterfaceControl);   
}

void MEM_EEPROM_Write(WORD address, BYTE * buffer, WORD bufferSize) {

    WORD bufferPageSize;

    println_info(SYSTEM_LOG_MEMEEPROM_ID, "Write Data into address: %04X, Total Size: %d", address, bufferSize);

    while(bufferSize){

        bufferPageSize = MEM_EEPROM_MAX_BUFFER_PAGE_SIZE - (address % MEM_EEPROM_MAX_BUFFER_PAGE_SIZE);

        if(bufferSize < bufferPageSize){

            bufferPageSize = bufferSize;
        }
            
        MEM_EEPROM_WritePage(address, buffer, bufferPageSize);

        bufferSize -= bufferPageSize;
        buffer += bufferPageSize;
        address += bufferPageSize;
    }
}
