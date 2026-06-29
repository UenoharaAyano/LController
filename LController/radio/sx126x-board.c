#include "myRadio_gpio.h"
#include "radio.h"
#include "sx126x.h"
#include "sx126x-board.h"

void HAL_Delay_nMS(uint32_t time_ms)
{
    HAL_Delay(time_ms);
}
void SX126xReset( void )
{
    HAL_Delay_nMS( 10 );
    RF_SX126x_RST_L();
    HAL_Delay_nMS( 20 );
    RF_SX126x_RST_H();
    HAL_Delay_nMS( 10 );
}

void SX126xWaitOnBusy( void )
{
   while(READ_RF_SX126x_BUSY());
}


void SX126xWakeup( void )
{
    BOARD_SPI_NSS_L();
    
    SpiReadWrite(RADIO_GET_STATUS);
    SpiReadWrite(0);
    
    BOARD_SPI_NSS_H();

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{

    SX126xCheckDeviceReady( );

    BOARD_SPI_NSS_L();

    SpiReadWrite(command );
    SpiWriteData(buffer, size);
    BOARD_SPI_NSS_H();
    
    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

void SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    BOARD_SPI_NSS_L();
    SpiReadWrite(command );
    SpiReadWrite(0x00);
    SpiReadData(buffer, size);
    BOARD_SPI_NSS_H();

    SX126xWaitOnBusy( );
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    BOARD_SPI_NSS_L();
    SpiReadWrite(RADIO_WRITE_REGISTER);
    SpiReadWrite(( address & 0xFF00 ) >> 8);
    SpiReadWrite(address & 0x00FF);
    SpiWriteData(buffer, size);
    BOARD_SPI_NSS_H();

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    SX126xCheckDeviceReady( );

    BOARD_SPI_NSS_L();

    SpiReadWrite(RADIO_READ_REGISTER);
    SpiReadWrite(( address & 0xFF00 ) >> 8);
    SpiReadWrite(address & 0x00FF);
    SpiReadWrite(0);
    SpiReadData(buffer, size);

    BOARD_SPI_NSS_H();

    SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    BOARD_SPI_NSS_L();
    SpiReadWrite(RADIO_WRITE_BUFFER);
    SpiReadWrite(offset);
    SpiWriteData(buffer, size);
    BOARD_SPI_NSS_H();

    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    SX126xCheckDeviceReady( );

    BOARD_SPI_NSS_L();

    SpiReadWrite(RADIO_READ_BUFFER);
    SpiReadWrite(offset);
    SpiReadWrite(0);
    SpiReadData(buffer, size);
    BOARD_SPI_NSS_H();
    
    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_200_US );
}

uint8_t SX126xGetPaSelect( uint32_t channel )
{
//    if( GpioRead( &DeviceSel ) == 1 )
//    {
//        return SX1261;
//    }
//    else
//    {
//        return SX1262;
//    }
  
  return SX1262;
}

void SX126xAntSwOn( void )
{
    //GpioInit( &AntPow, ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SX126xAntSwOff( void )
{
   // GpioInit( &AntPow, ANT_SWITCH_POWER, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}
