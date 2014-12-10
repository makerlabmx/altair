#include "Arduino.h"
#include "AquilaServices.h"
#include "Mesh.h"
#include <string.h>

static NWK_DataReq_t nwkPacket;
bool nwkTxBusy = false;

Service services[AQUILASERVICES_MAX];
void (*lastReqCb)(uint16_t srcAddr, uint8_t status, char *data, uint8_t dataSize);
bool waitingResponse;
double lastRequestTime;

static void nwkTxCb(NWK_DataReq_t *req)
{
	nwkTxBusy = false;
	(void) req;
}

static bool rxHandler(NWK_DataInd_t *ind)
{
	if(ind->srcEndpoint != AQUILASERVICES_ENDPOINT || ind->dstEndpoint != AQUILASERVICES_ENDPOINT) return false;

	/*
		ind->srcAddr;
		ind->size;
		ind->data;
		[version][method][nameSize][dataSize][name...][data...]
	*/

	// packet data must be at least 4 bytes for version, method, nameSize and dataSize
	if(ind->size < 4) return false;
	ServicePacket pkt;

	memcpy(&pkt, ind->data, ind->size);

	// Check version
	if(pkt.version != AQUILASERVICES_VERSION) return false;

	// Parsing name
	char name[AQUILASERVICES_MAXNAMESIZE];
	char data[AQUILASERVICES_MAXDATASIZE];

	memcpy(name, pkt.name_data, pkt.nameSize);
	name[pkt.nameSize] = NULL;	// End byte

	memcpy(data, &pkt.name_data[pkt.nameSize], pkt.dataSize);
	data[pkt.dataSize] = NULL;	// End byte for string security

	// Handle responses:
	if(pkt.method == R200 || pkt.method == R404 || pkt.method == R500 || pkt.method == R405 || pkt.method == R408)
	{
		if(lastReqCb != NULL) lastReqCb(ind->srcAddr, pkt.method, data, pkt.dataSize);
		// NULL out lastReqCb so it only works once:
		lastReqCb = NULL;
		waitingResponse = false;
		return true;
	}

	// Handle requests:
	for(uint8_t i = 0; i < AQUILASERVICES_MAX; i++)
	{
		if(services[i].name == NULL) continue;

		if( 0 == strcmp(services[i].name, name) )
		{
			if( NULL != services[i].function ) return services[i].function(ind->srcAddr, pkt.method, data, pkt.dataSize);
		}
	}

	return false;
}

AquilaServices::AquilaServices()
{
	// zeroing services:
	for(uint8_t i = 0; i < AQUILASERVICES_MAX; i++)
	{
		services[i].name = NULL;
		services[i].function = NULL;
	}

	lastReqCb = NULL;

	waitingResponse = false;
}

void AquilaServices::begin()
{
	// suscribe to endpoint 12
	Mesh.openEndpoint(AQUILASERVICES_ENDPOINT, rxHandler);
}

void AquilaServices::loop()
{
	// handle response timeout:
	double now = millis();
	if(lastRequestTime + AQUILASERVICES_TIMEOUT < now)
	{
		// call cb with timeout code
		if(lastReqCb != NULL) lastReqCb(0, R408, NULL, 0);
		// NULL out lastReqCb so it only works once:
		lastReqCb = NULL;
		waitingResponse = false;
	}
}
// Inscribe un servicio a name
void AquilaServices::add(char *name, bool (*function)(uint16_t reqAddr, uint8_t method, char *data, uint8_t dataSize))
{
	for(uint8_t i = 0; i < AQUILASERVICES_MAX; i++)
	{
		if(services[i].name == NULL)
		{
			services[i].name = name;
			services[i].function = function;
			return;
		}
	}
}
// Petición
void AquilaServices::request(uint16_t destAddr, uint8_t method, char *name, void (*callback)(uint16_t srcAddr, uint8_t status, char *data, uint8_t dataSize), char *data, uint8_t dataSize)
{
	// lose packet
	if(nwkTxBusy || waitingResponse) return;

	// Form packetData
	ServicePacket pkt;
	pkt.version = AQUILASERVICES_VERSION;
	pkt.method = method;
	pkt.nameSize = strlen(name);
	pkt.dataSize = dataSize;
	memcpy(pkt.name_data, name, pkt.nameSize);
	memcpy(&pkt.name_data[pkt.nameSize], data, dataSize);

	nwkPacket.dstAddr = destAddr;
	nwkPacket.dstEndpoint = AQUILASERVICES_ENDPOINT;
	nwkPacket.srcEndpoint = AQUILASERVICES_ENDPOINT;
	if(Mesh.getSecurityEnabled())
		nwkPacket.options = NWK_OPT_ENABLE_SECURITY;
	else
		nwkPacket.options = 0;
	nwkPacket.data = (uint8_t*)&pkt;
	nwkPacket.size = pkt.nameSize + pkt.dataSize + 4;
	nwkPacket.confirm = nwkTxCb;
	NWK_DataReq(&nwkPacket);

	nwkTxBusy = true;

	// Wait for response and call callback...
	// will be called on rxHandler
	lastReqCb = callback;
	waitingResponse = true;
	lastRequestTime = millis();
}
// Para usar dentro del servicio
void AquilaServices::response(uint16_t destAddr, uint8_t method, char *data, uint8_t dataSize)
{
	// lose packet
	if(nwkTxBusy) return;

	// Form packetData
	ServicePacket pkt;
	pkt.version = AQUILASERVICES_VERSION;
	pkt.method = method;
	pkt.nameSize = 0;
	pkt.dataSize = dataSize;
	memcpy(pkt.name_data, data, dataSize);

	nwkPacket.dstAddr = destAddr;
	nwkPacket.dstEndpoint = AQUILASERVICES_ENDPOINT;
	nwkPacket.srcEndpoint = AQUILASERVICES_ENDPOINT;
	nwkPacket.options = 0;
	nwkPacket.data = (uint8_t*)&pkt;
	nwkPacket.size = dataSize + 4;
	nwkPacket.confirm = nwkTxCb;
	NWK_DataReq(&nwkPacket);

	nwkTxBusy = true;
}

AquilaServices Services;