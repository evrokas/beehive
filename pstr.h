
/*
 * pstr.h - program memory strings
 *
 * Beehive Monitoring Project - BeeWatch
 *
 * (c) Evangelos Rokas, (c) 2018
 */

#ifndef __PSTR_H__
#define __PSTR_H__

#include <avr/pgmspace.h>
#include "bms.h"


#define RCF(str)	reinterpret_cast<const __FlashStringHelper *>(str)

const prog_char pATCIPSHUTrn [] PROGMEM	= "AT+CIPSHUT\r\n";
const prog_char pSHUTOK [] PROGMEM			= "SHUT OK";
const prog_char pCONNECT [] PROGMEM			= "CONNECT";
const prog_char pSEND [] PROGMEM				= "SEND";

const prog_char pATCIPMUX0rn [] PROGMEM	= "AT+CIPMUX=0\r\n";
const prog_char pOK [] PROGMEM					= "OK";
const prog_char pATSAPBR31CONTYPE [] PROGMEM		= "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n";
const prog_char pATSAPBR31APN [] PROGMEM				= "AT+SAPBR=3,1,\"APN\",\"";
const prog_char pATSAPBR31USER [] PROGMEM				= "AT+SAPBR=3,1,\"USER\",\"";
const prog_char pATSAPBR31PWD [] PROGMEM				= "AT+SAPBR=3,1,\"PWD\",\"";
const prog_char pATSAPBR11rn [] PROGMEM					= "AT+SAPBR=1,1\r\n";
const prog_char pATSAPBR21rn [] PROGMEM					= "AT+SAPBR=2,1\r\n";
const prog_char pATSAPBR01rn [] PROGMEM					= "AT+SAPBR=0,1\r\n";

const prog_char pATCSTTQ [] PROGMEM							= "AT+CSTT=\"";
const prog_char pQcommaQ [] PROGMEM							= "\",\"";


const prog_char pATCIPSTATUSrn [] PROGMEM				= "AT+CIPSTATUS\r\n";
const prog_char pIPSTART [] PROGMEM							= "IP START";
const prog_char pATCIICRrn [] PROGMEM						= "AT+CIICR\r\n";
const prog_char pIPGPRSACT [] PROGMEM						= "IP GPRSACT";
const prog_char pATCIFSRrn [] PROGMEM						= "AT+CIFSR\r\n";

const prog_char pDOT [] PROGMEM									= ".";
const prog_char pCOMMA [] PROGMEM								= ",";
const prog_char pDDOT [] PROGMEM								= ":";
const prog_char pQ [] PROGMEM										= "\"";
const prog_char pAmp [] PROGMEM										= "&";
const prog_char pEq [] PROGMEM									= "=";
const prog_char pQddQ [] PROGMEM								= "\":\"";
const prog_char pCURLOPEN [] PROGMEM						= "{";
const prog_char pCURLCLOSE [] PROGMEM						= "}";
const prog_char pCRLF [] PROGMEM								= "\r\n";
const prog_char pCtrlZ [] PROGMEM								= "\032";

const prog_char pATCDNSGIPQ [] PROGMEM					= "AT+CDNSGIP=\"";
const prog_char pCDNSGIP [] PROGMEM							= "+CDNSGIP:";

const prog_char pATCIPSTART [] PROGMEM					= "AT+CIPSTART=\"TCP\",\"";
const prog_char pATCIPSEND [] PROGMEM						= "AT+CIPSEND\r\n";

const prog_char pATHTTPINITrn [] PROGMEM				= "AT+HTTPINIT\r\n";
const prog_char pATHTTPPARACIDrn [] PROGMEM			= "AT+HTTPPARA=\"CID\", 1\r\n";
const prog_char pATHTTPTERMrn [] PROGMEM				= "AT+HTTPTERM\r\n";
const prog_char pATHTTPPARAUArn [] PROGMEM			= "AT+HTTPPARA=\"UA\",\"BEEHIVE SIM MODULE\"\r\n";
const prog_char pATHTTPPARAURLQ [] PROGMEM			= "AT+HTTPPARA=\"URL\",\"http://";

const prog_char pGET [] PROGMEM				= "GET ";
const prog_char pPOST [] PROGMEM			= "POST ";
const prog_char pHTTP11 [] PROGMEM		= " HTTP/1.1\n";
const prog_char pHOST [] PROGMEM			= "Host: ";
const prog_char pUserAgent [] PROGMEM	= "User-Agent: beewatch/0.1 arduino/181\n";

const prog_char pACTION [] PROGMEM		= "action";
const prog_char pGETCONF [] PROGMEM		= "getconf";

const prog_char pDAT [] PROGMEM						= "dat";
const prog_char pGSM [] PROGMEM						= "gsm";
const prog_char PGPS [] PROGMEM						= "gps";


#if defined( API_STRING_KEYS )

//const prog_char pAPIKEY [] PROGMEM				= "apikey";
const prog_char pnodeId [] PROGMEM				= "nodeId";
const prog_char pentryType [] PROGMEM			= "entryType";
const prog_char pbatVolt [] PROGMEM				= "batVolt";
const prog_char pbhvTemp [] PROGMEM				= "bhvTemp";
const prog_char pbhvHumid [] PROGMEM			= "bhvHumid";
const prog_char pbhvWeight [] PROGMEM			= "bhvWeight";
const prog_char pgsmSig [] PROGMEM				= "gsmSig";
const prog_char pgsmVolt [] PROGMEM				= "gsmVolt";
const prog_char pgsmPDur [] PROGMEM				= "gsmPDur";
const prog_char pgpsLon [] PROGMEM				= "gpsLon";
const prog_char pgpsLat [] PROGMEM				= "gpsLat";
const prog_char prtcDateTime [] PROGMEM		= "rtcDateTime";

#elif defined( API_NUMERIC_KEYS )

//const prog_char pAPIKEY [] PROGMEM				= "1";			//"apikey";
const prog_char pnodeId [] PROGMEM				= "0";			//"nodeId";
const prog_char pentryType [] PROGMEM			= "1";			//"entryType";
const prog_char pbatVolt [] PROGMEM				= "2";			//"batVolt";
const prog_char pbhvTemp [] PROGMEM				= "3";			//"bhvTemp";
const prog_char pbhvHumid [] PROGMEM			= "4";			//"bhvHumid";
const prog_char pbhvWeight [] PROGMEM			= "5";			//"bhvWeight";
const prog_char pgsmSig [] PROGMEM				= "6";			//"gsmSig";
const prog_char pgsmVolt [] PROGMEM				= "7";			//"gsmVolt";
const prog_char pgsmPDur [] PROGMEM				= "8";			//"gsmPDur";
const prog_char pgpsLon [] PROGMEM				= "9";			//"gpsLon";
const prog_char pgpsLat [] PROGMEM				= "10";			//"gpsLat";
const prog_char prtcDateTime [] PROGMEM		= "11";			//"rtcDateTime";

#else

#error	Please define API_STRING_KEYS or API_NUMERIC_KEYS in bms.h

#endif



const prog_char pATHTTPACTION0rn [] PROGMEM	= "AT+HTTPACTION=0\r\n";
const prog_char pHTTPACTION [] PROGMEM			= "+HTTPACTION:";
const prog_char pATHTTPREADrn [] PROGMEM 		= "AT+HTTPREAD\r\n";
const prog_char pHTTPREAD [] PROGMEM				= "+HTTPREAD: ";

const prog_char pATIrn [] PROGMEM				= "ATI\r\n";
const prog_char pATCPINeq [] PROGMEM		= "AT+CPIN=";
const prog_char pATrn [] PROGMEM				= "AT\r\n";
const prog_char pRN [] PROGMEM					= "\r\n";
const prog_char pLF [] PROGMEM						= "\n";

const prog_char pATCCIDrn [] PROGMEM		= "AT+CCID\r\n";
const prog_char pATCREGqrn [] PROGMEM		= "AT+CREG?\r\n";
//const prog_char pCREG [] PROGMEM				= "+CREG: ";
const prog_char pATCFUNC0rn [] PROGMEM	= "AT+CFUNC=0\r\n";
const prog_char pATCFUNC1rn [] PROGMEM	= "AT+CFUNC=1\r\n";
const prog_char pATCBCrn [] PROGMEM			= "AT+CBC\r\n";
const prog_char pCBC [] PROGMEM					= "+CBC: ";
const prog_char pATCSQrn [] PROGMEM			= "AT+CSQ\r\n";
const prog_char pCSQ [] PROGMEM					= "+CSQ: ";
const prog_char pATCIPGSMLOC11rn [] PROGMEM	= "AT+CIPGSMLOC11\r\n";
const prog_char pCIPGSMLOC [] PROGMEM					= "+CIPGSMLOC:";		/* take care not to include the last space */
const prog_char pATCIPGSMLOC21rn [] PROGMEM	= "AT+CIPGSMLOC21\r\n";

const prog_char pNoReceivedLength [] PROGMEM	= "no received bytes length";
const prog_char pErrorCPIN [] PROGMEM					= "error setting SIM pin";

const prog_char pQrn [] PROGMEM			= "\"\r\n";

const prog_char pNodeId [] PROGMEM	= "Node ID: ";
const prog_char pWrongParameter [] PROGMEM = "wrong parameter";
const prog_char pErrorCouldNot [] PROGMEM = "error: could not ";
const prog_char pErrorSendDATblock [] PROGMEM = "send data block";
const prog_char pErrorSendGSMblock [] PROGMEM = "send gsm block";
const prog_char pErrorInitiateGet [] PROGMEM = "initiate get request";
const prog_char pErrorActivateBearer [] PROGMEM ="activate bearer profile";
const prog_char pErrorInitiatePost [] PROGMEM = "initiate post request";


#endif	/* __PSTR_H__ */
 