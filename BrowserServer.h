﻿#ifndef _BROWSERSERVER_h
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

#define SECRET_FILE "/secret.json"
#define TEXT_HTML	"text/html"

#define MY_HOST_NAME "master"
#define SOFT_AP_SSID "SCALES_TRAVERS"
#define SOFT_AP_PASSWORD "23232323"

// DNS server
#define DNS_PORT 53

typedef struct {
	String wwwUsername;
	String wwwPassword;
} strHTTPAuth;

const char index_html[] PROGMEM = R"(<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1.0, user-scalable=no'/> <meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/> <meta http-equiv='Pragma' content='no-cache'/> <title>WEB SCALES</title> <link rel='stylesheet' type='text/css' href='global.css'> <link rel='shortcut icon' href='favicon.png' type='image/png'> <style>img.bat{width:10px;height:12px;}#w_style{background:#fff;font-size:80px;font-family:Arial,sans-serif;color:#618ad2;margin-left:auto;margin-right:auto;}table{width:100%;}input{font-size:20px;text-align:center;}</style> <script>var w; function ScalesSocket(h, p, fm, fe){var host=h;var protocol=p;var timerWeight;var timerSocket;var ws; var startWeightTimeout=function(){clearTimeout(timerWeight); timerWeight=setTimeout(function (){fe();},5000);}; this.getWeight=function (){ws.send('/wt'); startWeightTimeout();}; this.openSocket=function(){ws=new WebSocket(host,protocol); ws.onopen=this.getWeight; ws.onclose=function(e){clearTimeout(timerWeight);starSocketTimeout();fe();}; ws.onerror=function(e){fe();}; ws.onmessage=function(e){fm(JSON.parse(e.data));}}; var starSocketTimeout=function (){clearTimeout(timerSocket); timerSocket=setTimeout(function(){this.prototype.openSocket();},5000);}}function go(){document.getElementById('weight').innerHTML='---'; document.getElementById('buttonZero').style.visibility='visible';}function setZero(){document.getElementById('buttonZero').style.visibility='hidden'; var request=new XMLHttpRequest(); document.getElementById('weight').innerHTML='...'; request.onreadystatechange=function(){if (this.readyState===4 && this.status===204){document.getElementById('buttonZero').style.visibility='visible'; w.getWeight();}}; request.open('GET','/tp',true); request.timeout=5000; request.ontimeout=function(){go();}; request.onerror=function(){go();}; request.send(null);}window.onload=function(){onBodyLoad();}; function onBodyLoad(){w=new ScalesSocket('ws://'+document.location.host+'/ws',['arduino'],function(e){try{document.getElementById('weight').innerHTML=e.w; document.getElementById('mw_id').innerHTML=e.ms.w; document.getElementById('sw_id').innerHTML=e.sl.w; document.getElementById('mc_id').innerHTML=e.ms.c+'%'; document.getElementById('sc_id').innerHTML=e.sl.c+'%'; if(e.ms.s && e.sl.s){document.getElementById('w_style').setAttribute('style','background: #fff;');}else{document.getElementById('w_style').setAttribute('style','background: #C4C4C4;');}}catch(e){go();}finally{w.getWeight();}},function(){go(); w.openSocket();}); w.openSocket();}</script></head><body><div align='center'> <table><tr><td ><img src='scales.png' style='height: 42px;'/></td><td align='right'><h3 id='brand_name'>scale.in.ua</h3></td></tr></table> <p hidden='hidden' id='dnt'></p></div><hr><div align='right' id='w_style'> <b id='weight'>---</b></div><hr><table> <tr> <td> <h5> <table> <tr> <td>MASTER</td><td align='center' style='width:100px' id='mw_id'></td><td style='width:1%; white-space: nowrap'><img class='bat' src='battery.png' alt='B'/></td><td><h3 id='mc_id' style='display: inline'>--%</h3></td></tr><tr> <td>SLAVE</td><td align='center' id='sw_id'></td><td style='width:1%; white-space: nowrap'><img class='bat' src='battery.png' alt='B'/></td><td><h3 id='sc_id' style='display: inline'>--%</h3></td></tr></table> </h5> </td><td align='right'><b><a href='javascript:setZero()' id='buttonZero' class='btn btn--s btn--blue'>&lt;0&gt;</a></b></td></tr></table><hr><table><tr><td><a href='/events.html' class='btn btn--m btn--blue'>события</a><br></td></tr><tr><td><br/><a href='/settings.html'>настройки</a><br></td></tr></table><hr><footer align='center'>2018 © Powered by www.scale.in.ua</footer></body></html>)";

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
/*
#if! HTML_PROGMEM
	void handleAccessPoint(AsyncWebServerRequest*);
#endif*/
void handleScaleProp(AsyncWebServerRequest*);
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void onSsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

#endif






