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
// ---------------------------------------------------
#include "mbed-drivers/mbed.h"
#include "nsdl-c/sn_coap_protocol.h"
#include "example-mbedos-blinky/modem_driver.h"


// ----------------------------------------------------------------
// GENERAL COMPILE-TIME CONSTANTS
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// TYPES
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// GLOBAL VARIABLES
// ----------------------------------------------------------------
static coap_s *coap_handle = NULL;
static uint8_t*  gMsgPacket;
static uint32_t  gMsgPacketSize;

// ----------------------------------------------------------------
// FUNCTION PROTOTYPES
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// STATIC FUNCTIONS
// ----------------------------------------------------------------

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
	printf("[blinky->tx_callback]\r\n");
    return 0;
}

static int8_t rx_callback(sn_coap_hdr_s *a, sn_nsdl_addr_s *b, void *c)
{
	printf("[blinky->rx_callback]\r\n");
	return 0;
}


static bool modem(char *datagram, uint32_t datagramLen){

	bool status = false;
	bool usingSoftRadio = true;

	Nbiot *pModem = NULL;
	if( !(pModem = new Nbiot()) ){
		printf ("[blinky->modem]  Out of Memory !!! \r\n");
	}
	else{
		printf ("[blinky->modem]  Initialising module ... \r\n");
		status = pModem->connect(usingSoftRadio);
		if (status)
		{
			status = pModem->send ( datagram, datagramLen);
			if(status)
			{
				printf ("[blinky->modem]  OK.\r\n");
				// Receive a message from the network
				datagramLen = pModem->receive (datagram, datagramLen);
				if (datagramLen > 0)
				{
					printf ("[blinky->mod]  RX: \"%.*s\".\r\n", datagramLen, datagram);
					printf ("[blinky->mod]  RX \r\n");
				}
			}
			else
			{
				printf ("[blinky->modem]  Failed to send datagram !!!\r\n");
			}
		}
		else
		{
			printf ("[blinky->modem]  Failed to connect to the network !!! \r\n");
		}
	}
	delete (pModem);
	printf ("\r\n");
	return status;
}



static bool msgCoAP(uint8_t msgPayload, uint16_t msgPayloadSize, sn_coap_msg_type_e  msgType,sn_coap_msg_code_e  msgCode)
{
	bool status = false;

	struct coap_s *handle = sn_coap_protocol_init(myMalloc, myFree, tx_callback, rx_callback);
	if(handle)
	{
		printf("[blinky->msgCoAP] CoAP int OK! \r\n");

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
		msg.coap_status = COAP_STATUS_OK;

		/* msgHeader */
		msg.msg_type = msgType;
		msg.msg_code = msgCode;
		msg.msg_id = 18;

		/* msgOptions */
		/* Here are most often used Options */

		msg.uri_path_len = 0;
		msg.uri_path_ptr = NULL;
		msg.token_len = 0;
		msg.token_ptr = NULL;
		msg.content_type_len = 0;
		msg.content_type_ptr = NULL;

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
		msg.payload_ptr = &msgPayload;
		msg.payload_len = msgPayloadSize;

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
	}
	return status;
}


static void msgTest(void)
{
	uint8_t msg =7;
    if(msgCoAP(msg, 1, COAP_MSG_TYPE_CONFIRMABLE, COAP_MSG_CODE_REQUEST_POST))
    {
    	if( !modem( (char*)gMsgPacket, gMsgPacketSize) )
    	{
    		// try again?
    	}
    }
}

static void msgTest2(void)
{
	printf ("----\r\n");
}

// ----------------------------------------------------------------
// PUBLIC FUNCTIONS
// ----------------------------------------------------------------

void app_start(int, char**)
{
    minar::Scheduler::postCallback(msgTest).period(minar::milliseconds(5000));
}

























