// Serial port driver for NB-IoT example application

#ifndef _SERIAL_DRIVER_H_
#define _SERIAL_DRIVER_H_
#include "stdint.h"
#include "stdio.h"
#include "mbed-drivers/mbed.h"

// ----------------------------------------------------------------
// CLASSES
// ----------------------------------------------------------------

// Serial port interface
class SerialPort {
public:
    SerialPort(PinName tx=UART1_TX, PinName rx=UART1_RX, int baudrate=9600);
    ~SerialPort();

#ifdef YOTTA_CFG_MBED_OS
    // mbed OS
    bool connect(const char * pPortName);
#else
    bool connect(const TCHAR * pPortName);
#endif

    // Disconnect from the current serial port.
    void disconnect(void);

    // Transmit lenBuf characters from pBuf over the serial port.
    // Returns TRUE on success, otherwise FALSE.
    bool transmitBuffer(const char * pBuf, uint32_t lenBuf);
    
    // Receive up to lenBuf characters into pBuf over the serial port.
    // Returns the number of characters received.
    uint32_t receiveBuffer(char * pBuf, uint32_t lenBuf);
    
    // Receive a single character from the serial port.
    // Returns -1 if there are no characters, otherwise it
    // returns the character (i.e. it can be cast to char).
    int32_t receiveChar();
    
    // Clear the serial port buffers, both transmit and receive.
    void clear();


protected:
#ifdef YOTTA_CFG_MBED_OS
    // mbed OS
    Serial *pgUart = NULL;
    static void uartCallBack(void);
#else
    // not mbed OS
    HANDLE gSerialPortHandle;
#endif

};

#endif

// End Of File
