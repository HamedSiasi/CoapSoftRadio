// Serial port driver for NB-IoT example application

#include "stdint.h"
#include "stdio.h"
#include "mbed-drivers/mbed.h"
#include "example-mbedos-blinky/serial_driver.h"

#ifdef YOTTA_CFG_MBED_OS
// mbed OS
#else
// not mbed OS
#include "windows.h"
#endif

// ----------------------------------------------------------------
// CLASSES/METHODS
// ----------------------------------------------------------------

// Constructor.
SerialPort::SerialPort(PinName tx/*UART1_TX*/,  PinName rx/*UART1_RX*/,  int baudrate/*9600*/)
{
	//printf ("[serial->Constructor]\r\n");
#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
    pgUart = new Serial(tx, rx);
	pgUart->baud(baudrate);
	pgUart->format(8, SerialBase::None, 1);
	//pgUart->attach(uartCallBack);
#else
	// not mbed OS
	gSerialPortHandle = INVALID_HANDLE_VALUE;
#endif
}



// Destructor.
SerialPort::~SerialPort()
{
	//printf ("[serial->Destructor]\r\n");
#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
	delete (pgUart);
#else
	// not mbed OS
	if (gSerialPortHandle != INVALID_HANDLE_VALUE)
	{
			CloseHandle(gSerialPortHandle);
	}
	gSerialPortHandle = INVALID_HANDLE_VALUE;
#endif
}



// CallBack function to call whenever a serial interrupt is generated.
void SerialPort::uartCallBack(void)
{
	printf ("[serial->uartCallBack]\r\n");
}



// Disconnect from the port.
void SerialPort::disconnect(void)
{
	printf ("[serial->disconnect]\r\n");
#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
#else
	// not mbed OS
	CloseHandle(gSerialPortHandle);
	gSerialPortHandle = INVALID_HANDLE_VALUE;
#endif

}



void SerialPort::clear()
{
	printf ("[serial->clear]\r\n");
#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
#else
	// not mbed OS
	PurgeComm (gSerialPortHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
#endif

}





// Make a connection to a named port.
bool SerialPort::connect(const char * pPortName)
{
	printf ("[serial->connect]\r\n");
#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
    bool success = true;
#else
    // not mbed OS
    bool success = false;
    DCB dcb;
    COMMTIMEOUTS timeouts;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = 57600;
    dcb.Parity = NOPARITY;
    dcb.fParity = 0;
    dcb.StopBits = ONESTOPBIT;
    dcb.ByteSize = 8;
    memset(&timeouts, 0, sizeof(timeouts));
     // read() times out immediately, write() never times out
    timeouts.ReadIntervalTimeout = MAXDWORD;
    // Get a handle on the serial port
    gSerialPortHandle = CreateFile(pPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        // Set the comms port parameters and the timeouts
        if (SetCommState(gSerialPortHandle, &dcb) && SetCommTimeouts(gSerialPortHandle, &timeouts))
        {
            success = true;
        }
    }
    else
    {
        uint32_t err = GetLastError();
#ifdef _MSC_VER
        wchar_t * pMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &pMsgBuf, 0, NULL);
        printf("[serial->connect]  Error connecting to serial port '%S': %S. \r\n", pPortName, pMsgBuf);
#else
        printf("[serial->connect]  Error %d connecting to serial port %s. \r\n", err, pPortName);
#endif
    }

    if (!success)
    {
        disconnect();
    }
    else
    {
        clear();
    }
#endif

    return success;
}






// Send lenBuf bytes from pBuf over the serial port, returning true
// in the case of success.
bool SerialPort::transmitBuffer(const char *pBuf, uint32_t lenBuf)
{
	unsigned long result = 0;

#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
    if(pgUart->writeable())
    {
    	result = pgUart->printf(pBuf);
    	//result = pgUart->printf("AT\r\n");
        if (!result)
        {
            printf ("[serial->transmitBuffer]  Transmit failed !!! \r\n");
        }
    }
#else
    // not mbed OS
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        WriteFile(gSerialPortHandle, pBuf, lenBuf, &result, NULL);
        if (!result)
        {
            printf ("[serial->receiveBuffer]  Transmit failed with error code %ld.\r\n", GetLastError());
        }
    }
#endif
    return (bool) result;
}






// Get up to lenBuf bytes into pBuf from the serial port,
// returning the number of characters actually read.
uint32_t SerialPort::receiveBuffer (char *pBuf, uint32_t lenBuf)
{
	//printf ("[serial->receiveBuffer]\r\n");
	unsigned long result = 0;

#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
    if(pgUart->readable())
    {
    	result = pgUart->scanf("%s",pBuf);
        if (!result)
        {
            printf ("[serial->receiveBuffer] Receive failed !!!.\r\n");
        }
    }
#else
    // not mbed OS
    unsigned long readLength;
    readLength = 0;
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        result = ReadFile(gSerialPortHandle, pBuf, lenBuf, &readLength, NULL);
        if (!result)
        {
            printf ("[serial->receiveBuffer]  Receive failed with error code %ld.\r\n", GetLastError());
        }
    }
#endif

    return (uint32_t) result;
}











// Read a single character from the serial port, returning
// -1 if no character is read.
int32_t SerialPort::receiveChar()
{
	int32_t returnChar = -1;

#ifdef YOTTA_CFG_MBED_OS
	// mbed OS
	if( pgUart->readable() )
    {
    	returnChar = (int32_t) pgUart->getc();
    	//printf("%c \r\n", returnChar);
    }
#else
    // not mbed OS
    char readChar = 0;
    unsigned long result = 0;
    unsigned long readLength;
    readLength = 0;
    if (gSerialPortHandle != INVALID_HANDLE_VALUE)
    {
        result = ReadFile(gSerialPortHandle, &readChar, sizeof (readChar), &readLength, NULL);
        if (result)
        {
            if (readLength > 0)
            {
                returnChar = (int32_t) readChar;
            }
        }
        else
        {
            printf ("[serial->receiveChar]  Receive failed with error code %ld.\r\n", GetLastError());
        }
    }
#endif

    return (int32_t)returnChar;
}
//END FILE








