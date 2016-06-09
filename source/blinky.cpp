/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#include <inttypes.h>
#include "mbed-drivers/mbed.h"
#include "nsdl-c/sn_coap_protocol.h"

#include "example-mbedos-blinky/modem_driver.h"


// ----------------------------------------------------------------
// GENERAL COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

// Macro to get the number of elements in an array
#define ARRAY_COUNT(x) (sizeof(x)/sizeof((x)[0]))

// Things to do with the processor
#define SYSTEM_CONTROL_BLOCK_START_ADDRESS ((uint32_t *) 0xe000ed00)
#define SYSTEM_RAM_SIZE_BYTES 16384


// ----------------------------------------------------------------
// TYPES
// ----------------------------------------------------------------

// Tick callback
typedef void (*TickCallback_t)(uint32_t count);

// Rotator
typedef char RotatorItem_t[2];

// ----------------------------------------------------------------
// GLOBAL VARIABLES
// ----------------------------------------------------------------

// An array that makes a pleasing rotating marker, made up of the marker
// and then a backspace character to prevent the cursor advancing
static const RotatorItem_t gRotator[] = {{'/', '\b'}, {'-', '\b'}, {'\\', '\b'}, {'|', '\b'}};


// All the rest of the RAM in our littul world.
// To get the USED_RAM_SIZE, set the size of gRestOfRam to 1, do a build,
// then run arm-none-eabi-size on the resulting ELF file (which is
// the one without a file extension), add up the data and bss numbers,
// add 256 for the IPC block, subtract 4 for the uint32_t you left in the
// array and that's your answer.

//#define USED_RAM_SIZE 5556
//static uint32_t gRestOfRam[SYSTEM_RAM_SIZE_BYTES / sizeof (uint32_t) - (USED_RAM_SIZE / sizeof (uint32_t))];
static uint32_t gRestOfRam[1];
static coap_s *coap_handle = NULL;


// GPIO to toggle
static DigitalOut gGpio(LED1);

// Flipper to test uS delays
static Ticker gFlipper;

// Hook to let me read stdin
static Serial& gSerial = get_stdio_serial();

static uint8_t*  gMsgPacket;
static uint32_t  gMsgPacketSize;
//static Nbiot *pModem;

// ----------------------------------------------------------------
// FUNCTION PROTOTYPES
// ----------------------------------------------------------------

static void checkCpu(void);
static uint32_t *checkRam(void);
static void blinky(void);
static void flip(void);
static void serialRxCallback(void);

//static void msgTest(void);
//static bool modem(uint8_t *datagram, uint32_t datagramLen);
//static bool msgCoAP(uint8_t *msgPayload, uint16_t msgPayloadSize, sn_coap_msg_type_e  msgType, sn_coap_msg_code_e  msgCode);

// ----------------------------------------------------------------
// STATIC FUNCTIONS
// ----------------------------------------------------------------

static void checkCpu()
{
    uint32_t x = 0x01234567;

    printf("\n*** Printing stuff of interest about the CPU.\r\n");
    if ((*(uint8_t *) &x) == 0x67)
    {
        printf("Little endian.\r\n");
    }
    else
    {
        printf("Big endian.\r\n");
    }

    // Read the system control block
    // CPU ID register
    printf("[checkCpu] CPUID: 0x%08lx.\r\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS));
    // Interrupt control and state register
    printf("[checkCpu] ICSR: 0x%08lx.\r\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 1));
    // VTOR is not there, skip it
    // Application interrupt and reset control register
    printf("[checkCpu] AIRCR: 0x%08lx.\r\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 3));
    // SCR is not there, skip it
    // Configuration and control register
    printf("[checkCpu] CCR: 0x%08lx.\r\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 5));
    // System handler priority register 2
    printf("[checkCpu] SHPR2: 0x%08lx.\r\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 6));
    // System handler priority register 3
    printf("[checkCpu] SHPR3: 0x%08lx.\r\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 7));
    // System handler control and status register
    printf("[checkCpu] SHCSR: 0x%08lx.\r\n", *(SYSTEM_CONTROL_BLOCK_START_ADDRESS + 8));

    printf("[checkCpu] Last stack entry was at 0x%08lx.\r\n", (uint32_t) &x);
    printf("[checkCpu] A static variable is at 0x%08lx.\r\n", (uint32_t) &gRotator);
}


static uint32_t * checkRam(void)
{
    uint32_t * pLocation = NULL;
    uint32_t value;

    // Write a walking 1 pattern
    value = 1;
    pLocation = &(gRestOfRam[0]);
    for (pLocation = &(gRestOfRam[0]); pLocation < &(gRestOfRam[0]) + ARRAY_COUNT(gRestOfRam); pLocation++)
    {
        *pLocation = value;
        value <<= 1;
        if (value == 0)
        {
            value = 1;
        }
    }

    // Read the walking 1 pattern
    value = 1;
    for (pLocation = &(gRestOfRam[0]); (pLocation < &(gRestOfRam[0]) + ARRAY_COUNT(gRestOfRam)) && (*pLocation == value); pLocation++)
    {
        value <<= 1;
        if (value == 0)
        {
            value = 1;
        }
    }

    if (pLocation >= &(gRestOfRam[0]) + ARRAY_COUNT(gRestOfRam))
    {
        // Write an inverted walking 1 pattern
        value = 1;
        for (pLocation = &(gRestOfRam[0]); (pLocation < &(gRestOfRam[0]) + ARRAY_COUNT(gRestOfRam)); pLocation++)
        {
            *pLocation = ~value;
            value <<= 1;
            if (value == 0)
            {
                value = 1;
            }
        }

        // Read the inverted walking 1 pattern
        value = 1;
        for (pLocation = &(gRestOfRam[0]); (pLocation < &(gRestOfRam[0]) + ARRAY_COUNT(gRestOfRam)) && (*pLocation == ~value); pLocation++)
        {
            value <<= 1;
            if (value == 0)
            {
                value = 1;
            }
        }
    }

    if (pLocation >= &(gRestOfRam[0]) + ARRAY_COUNT(gRestOfRam))
    {
        pLocation = NULL;
    }

    return pLocation;
}

static void blinky(void)
{
    static uint8_t rotatorIndex = 0;

    gGpio = !gGpio;

    printf("%.*s", sizeof (gRotator[rotatorIndex]), gRotator[rotatorIndex]);
    rotatorIndex++;
    if (rotatorIndex >= ARRAY_COUNT(gRotator))
    {
        rotatorIndex = 0;
    }
}

static void flip()
{
    gGpio = !gGpio;
}

static void serialRxCallback()
{
    gSerial.putc(gSerial.getc());
}

static void* myMalloc(uint16_t size)
{
	return malloc(size);
}

static void myFree(void* addr)
{
    if( addr ){
        free(addr);
    }
}

static uint8_t tx_callback(uint8_t *a, uint16_t b, sn_nsdl_addr_s *c, void *d)
{
	//printf("[blinky->tx_callback]\r\n");
    return 0;
}

static int8_t rx_callback(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c)
{
	//printf("[blinky->rx_callback]\r\n");
	return 0;
}


static bool modem(uint8_t *datagram, uint32_t datagramLen){
	//printf ("[blinky->modem]\r\n");

	bool status = false;
	bool usingSoftRadio = true; //false;

	Nbiot *pModem ;
	if( !(pModem = new Nbiot()) )
	{
		printf ("[blinky->modem] Out of Memory. \r\n");
	}
	else
	{
		printf ("[blinky->modem] Initialising module... \r\n");
		status = pModem->connect(usingSoftRadio);
		if (status)
		{
			printf ("[blinky->modem] Sending initial datagram \"%*s\".\r\n", datagramLen, datagram);
			status = pModem->send ( (char*)datagram, datagramLen);
			if(status)
			{
				printf ("[blinky->modem] OK.\r\n");
				// Receive a message from the network
				datagramLen = pModem->receive ( (char*)datagram, datagramLen);
				if (datagramLen > 0)
				{
					printf ("[blinky->modem] Datagam received from network: \"%.*s\".\r\n", datagramLen, datagram);
				}
			}
			else
			{
				printf ("[blinky->modem] Failed to send datagram.\r\n");
			}
		}
		else
		{
			printf ("[blinky->modem] Failed to connect to the network. \r\n");
		}
	}
	delete (pModem);
	printf ("[blinky->modem] Exitting ...\r\n\n\n");
	return status;
}



static bool msgCoAP(
		uint8_t             msgPayload,
		uint16_t            msgPayloadSize,
		sn_coap_msg_type_e  msgType,
		sn_coap_msg_code_e  msgCode)
{
	//printf("[blinky->msgCoAP]\r\n");
	bool status = false;

	/* This function sets the memory allocation and must be called first. */
	struct coap_s *handle = sn_coap_protocol_init(myMalloc, myFree, tx_callback, rx_callback);
	if(!handle){
		printf("[blinky->msgCoAP] CoAP init ERROR !!! \r\n");
		//return here
	}
	else{
		printf("[blinky->msgCoAP] CoAP int OK! \r\n");
	}

	/* Destination address where CoAP message will be sent (CoAP builder needs that information for message resending purposes)*/
	sn_nsdl_addr_s msgDestAddr;
	memset(&msgDestAddr, 0, sizeof(sn_nsdl_addr_s));

	msgDestAddr.addr_ptr = (uint8_t*)malloc(5);
	memset(msgDestAddr.addr_ptr, '1', 5);
	//msgDestAddr.addr_len  =  16;
	//msgDestAddr.type  =  SN_NSDL_ADDRESS_TYPE_IPV4;
	//msgDestAddr.port  =  80;

	/* Destination of built Packet data */
	uint8_t* msgPacket = (uint8_t*)malloc(msgPayloadSize + 5);
	memset(msgPacket, '0', msgPayloadSize + 5);

	/* main coap message */
	sn_coap_hdr_s msg;
	memset(&msg, 0, sizeof(sn_coap_hdr_s));
	//msg.coap_status = COAP_STATUS_OK;

	/* msgHeader */
	//msg.msg_type = msgType;
	//msg.msg_code = msgCode;
	//msg.msg_id = 18;

	/* msgOptions */
	/* Here are most often used Options */
/*
	msg.uri_path_len = 0;
	msg.uri_path_ptr = NULL;
	msg.token_len = 0;
	msg.token_ptr = NULL;
	msg.content_type_len = 0;
	msg.content_type_ptr = NULL;*/

	/* Here are not so often used Options */
	//msg.options_list_ptr-> max_age_len = 0;           /**< 0-4 bytes. */
	//msg.options_list_ptr-> max_age_ptr = NULL;        /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> proxy_uri_len = 0;         /**< 1-1034 bytes. */
	//msg.options_list_ptr-> proxy_uri_ptr = NULL;      /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> etag_len = 0;              /**< 1-8 bytes. Repeatable */
	//msg.options_list_ptr-> etag_ptr = NULL;           /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> uri_host_len = 0;          /**< 1-255 bytes. */
	//msg.options_list_ptr-> uri_host_ptr = NULL;       /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> location_path_len = 0;     /**< 0-255 bytes. Repeatable */
	//msg.options_list_ptr-> location_path_ptr = NULL;  /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> uri_port_len = 0;          /**< 0-2 bytes. */
	//msg.options_list_ptr-> uri_port_ptr = NULL;       /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> location_query_len = 0;    /**< 0-255 bytes. Repeatable */
	//msg.options_list_ptr-> location_query_ptr = NULL; /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> observe = NULL;
	//msg.options_list_ptr-> observe_len = 0;           /**< 0-2 bytes. */
	//msg.options_list_ptr-> observe_ptr = NULL;        /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> accept_len = 0;            /**< 0-2 bytes. Repeatable */
	//msg.options_list_ptr-> accept_ptr = NULL;         /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> uri_query_len = 0;         /**< 1-255 bytes. Repeatable */
	//msg.options_list_ptr-> uri_query_ptr = NULL;      /**< Must be set to NULL if not used */
	//msg.options_list_ptr-> block1_len = 0;            /**< 0-3 bytes. */
	//msg.options_list_ptr-> block1_ptr = NULL;         /**< Not for User */
	//msg.options_list_ptr-> block2_len = 0;            /**< 0-3 bytes. */
	//msg.options_list_ptr-> block2_ptr = NULL;         /**< Not for User */
	//msg.options_list_ptr-> size1_len = 0;             /**< 0-4 bytes. */
	//msg.options_list_ptr-> size1_ptr = NULL;          /**< Not for User */


	/* msgPayload */
	msg.payload_ptr = NULL;
	msg.payload_len = 0;

	//coap_transaction_t *transaction_ptr;
	//void *transaction_ptr = NULL;

	/*
	 * \brief:  Builds Packet data from given header structure to be sent
	 * \return: Return value is byte count of built Packet data
	 */
	int16_t  msgPacketBytes = sn_coap_protocol_build(handle, &msgDestAddr, msgPacket, &msg, NULL);
	if( (int)msgPacketBytes == -1)
	{
		 printf("[blinky->msgCoAP] Failure in CoAP header structure\r\n");
	}
	else if( (int)msgPacketBytes == -2)
	{
		 printf("[blinky->msgCoAP] Failure in given pointer (= NULL)\r\n");
	}
	else if( (int)msgPacketBytes == -3)
	{
		 printf("[blinky->msgCoAP] Failure in Reset message\r\n");
	}
	else
	{
		 printf("[blinky->msgCoAP] CoAP packet build OK! (%d Bytes)\r\n" ,(int)msgPacketBytes);
		 gMsgPacket = msgPacket;
		 gMsgPacketSize = msgPacketBytes;
		 status = true;
	}
	free(msgDestAddr.addr_ptr);
	free(msgPacket);
	sn_coap_protocol_destroy(handle);
	return status;
}


static void msgTest(void)
{
	//printf ("[blinky->msgTest]\r\n");
	uint8_t msg =0;
    if(msgCoAP(msg, 1, COAP_MSG_TYPE_CONFIRMABLE, COAP_MSG_CODE_EMPTY))
    {
    	if(!modem(gMsgPacket, gMsgPacketSize))
    	{
    		// try again?
    	}
    	else
    	{
    		printf("[blinky->msgTest] all done :) \r\n");
    	}
    }
}


// ----------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------

void app_start(int, char**)
{
	//printf ("[app_start]\r\n");
    //uint32_t * pRamResult;
    //checkCpu();
    //printf("[app_start] *** Setting up serial echo.\r\n");
    //gSerial.attach (&serialRxCallback);

    //printf("[app_start] *** Checking RAM.\r\n");
    /*
    pRamResult = checkRam();
    if (pRamResult != NULL)
    {
        printf("[app_start] !!! RAM check failure at location 0x%08lx (contents 0x%08lx).\r\n", (uint32_t) pRamResult, *pRamResult);
        while(1) {};
    }*/

    //printf("[app_start] *** Running us_ticker for 2 seconds...\r\n");
    //gFlipper.attach_us(&flip, 100);
    //wait(2);
    //gFlipper.attach_us(NULL, 0);



    //printf("[app_start] *** Handing over to minar, which will check lp_ticker and sleep.\r\n");
    //minar::Scheduler::postCallback(blinky).period(minar::milliseconds(1000));
    minar::Scheduler::postCallback(msgTest).period(minar::milliseconds(5000));
}

























