#ifndef _BROWSERSERVER_h
#define _BROWSERSERVER_h

/*
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif*/
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <IPAddress.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "Core.h"
#include "SlaveScales.h"
#include "Scale.h"
#include "master_config.h"

#define SECRET_FILE "/secret.json"
#define TEXT_HTML	"text/html"

//#define MY_HOST_NAME "master"
//#define SOFT_AP_SSID "SCALES_TRAVERS"
#define SOFT_AP_PASSWORD "23232323"

// DNS server
#define DNS_PORT 53

typedef struct {
	String wwwUsername;
	String wwwPassword;
} strHTTPAuth;



class AsyncWebServer;

class BrowserServerClass : public AsyncWebServer{
	protected:
		strHTTPAuth _httpAuth;
		bool _saveHTTPAuth();		
		bool _downloadHTTPAuth();		
	public:
	
		BrowserServerClass(uint16_t port);
		~BrowserServerClass();
		void begin();
		void init();
		//static String urldecode(String input); // (based on https://code.google.com/p/avr-netino/)
		//static unsigned char h2int(char c);
		void send_wwwauth_configuration_html(AsyncWebServerRequest *request);
		//void restart_esp();		
		String getContentType(String filename);	
		//bool isValidType(String filename);		
		bool checkAdminAuth(AsyncWebServerRequest * request);
		bool isAuthentified(AsyncWebServerRequest * request);
		String getName(){ return _httpAuth.wwwUsername;};
		String getPass(){ return _httpAuth.wwwPassword;};			
		void stop(){_server.end();};
		//friend CoreClass;
		//friend BrowserServerClass;
};

class CaptiveRequestHandler : public AsyncWebHandler {
	public:
	CaptiveRequestHandler() {}
	virtual ~CaptiveRequestHandler() {}
	
	bool canHandle(AsyncWebServerRequest *request){
		if (!request->host().equalsIgnoreCase(WiFi.softAPIP().toString())){
			return true;
		}
		return false;
	}

	void handleRequest(AsyncWebServerRequest *request) {
		AsyncWebServerResponse *response = request->beginResponse(302,"text/plain","");
		response->addHeader("Location", String("http://") + WiFi.softAPIP().toString());
		request->send ( response);
	}
};

class TapeRequestHandler : public AsyncWebHandler {
	public:
	TapeRequestHandler() {}
	virtual ~TapeRequestHandler() {}
	
	virtual bool canHandle(AsyncWebServerRequest *request) override final;
	void handleRequest(AsyncWebServerRequest *request) {
		request->send(204, TEXT_HTML, "");
	}
};

//extern ESP8266HTTPUpdateServer httpUpdater;
extern DNSServer dnsServer;
extern IPAddress apIP;
extern IPAddress netMsk;
extern IPAddress lanIp;			// Надо сделать настройки ip адреса
extern IPAddress gateway;
extern BrowserServerClass browserServer;
extern AsyncWebSocket ws;

//void send_update_firmware_values_html();
//void setUpdateMD5();
//void handleFileReadAdmin(AsyncWebServerRequest*);
void handleFileReadAuth(AsyncWebServerRequest*);

#ifdef HTML_PROGMEM
	void handleBatteryPng(AsyncWebServerRequest*);
	void handleScalesPng(AsyncWebServerRequest*);
#endif

void handleSettings(AsyncWebServerRequest * request);
void handleScaleProp(AsyncWebServerRequest*);
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void onSsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

#endif







