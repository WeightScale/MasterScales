//#include <ESP8266WebServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <AsyncJson.h>
#include <functional>
#include <DNSServer.h>
#include "BrowserServer.h"
#include "tools.h"
#include "Core.h"
#include "Version.h"
#include "DateTime.h"
#include "HttpUpdater.h"
#include "SlaveScales.h"
#include "master_config.h"

using namespace std::placeholders;

/* */
//ESP8266HTTPUpdateServer httpUpdater;
/* Soft AP network parameters */
IPAddress apIP(192,168,4,1);
IPAddress netMsk(255, 255, 255, 0);

IPAddress lanIp;			// Надо сделать настройки ip адреса
IPAddress gateway;

BrowserServerClass browserServer(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
//holds the current upload
//File fsUploadFile;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */


BrowserServerClass::BrowserServerClass(uint16_t port) : AsyncWebServer(port) {}

BrowserServerClass::~BrowserServerClass(){}
	
void BrowserServerClass::begin() {
	/* Setup the DNS server redirecting all the domains to the apIP */
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());	
	_downloadHTTPAuth();
	ws.onEvent(onWsEvent);
	addHandler(&ws);
	addHandler(&SlaveScales);
	//addHandler(new TapeRequestHandler());
	CORE = new CoreClass(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str());	
	addHandler(CORE);
	addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
	addHandler(new SPIFFSEditor(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str()));	
	addHandler(new HttpUpdaterClass("sa", "654321"));
	init();
	AsyncWebServer::begin(); // Web server start
}

void BrowserServerClass::init(){
	on("/wt",HTTP_GET, [](AsyncWebServerRequest * request){					/* Получить вес и заряд. */
		AsyncResponseStream *response = request->beginResponseStream("text/json");
		DynamicJsonBuffer jsonBuffer;
		JsonObject &json = jsonBuffer.createObject();
		JsonObject& master = json.createNestedObject("ms");
		JsonObject& slave = json.createNestedObject("sl");
		
		char b[10];
		float f = Scale.getTest() + SlaveScales.getWeigt();
		Scale.formatValue(f,b);
		String str = SlaveScales.isConnected()?String(b):String("slave???");
		json["w"] = str;
		
		master["w"]= String(Scale.getBuffer());
		master["c"]= BATTERY.getCharge();
		master["s"]= Scale.getStableWeight();
		
		slave["w"]= SlaveScales.s_weight;
		slave["c"]= SlaveScales.getCharge();
		slave["s"]= SlaveScales.getStable();
		json.printTo(*response);
		request->send(response);
		//request->send(200, "text/html", String("{\"w\":\""+String(Scale.getBuffer())+"\",\"c\":"+String(BATTERY.getCharge())+",\"s\":"+String(Scale.getStableWeight())+"}"));	
	});			
	on("/rc", reconnectWifi);																					/* Пересоединиться по WiFi. */
	//on("/sn",HTTP_GET,handleAccessPoint);															/* Установить Настройки точки доступа */
	//on("/sn",HTTP_POST, std::bind(&CoreClass::handleSetAccessPoint, CORE, _1));					/* Установить Настройки точки доступа */
	//on("/settings.html", HTTP_ANY, std::bind(&CoreClass::saveValueSettingsHttp, CORE, _1));							/* Открыть страницу настроек или сохранить значения. */
	//on("/settings.json", handleFileReadAuth);
	on("/settings.json",HTTP_ANY, handleSettings);
	on("/sv", handleScaleProp);																						/* Получить значения. */
	on("/admin.html", std::bind(&BrowserServerClass::send_wwwauth_configuration_html, this, _1));
	on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/plain", SlaveScales.getUrl());
	});	
	on("/rssi",[](AsyncWebServerRequest *request){
		request->send(200, TEXT_HTML, String(WiFi.RSSI()));
	});
	/*on("/generate_204",HTTP_GET, [](AsyncWebServerRequest * request){									//Android captive portal. Maybe not needed. Might be handled by notFound handler.
		request->send(SPIFFS, "/index.html");		
	}).setFilter(ON_AP_FILTER); 
	on("/fwlink", HTTP_GET, [](AsyncWebServerRequest * request){									//Android captive portal. Maybe not needed. Might be handled by notFound handler.
		request->send(SPIFFS, "/index.html");
	}).setFilter(ON_AP_FILTER);*/ 
	//on("/secret.json",handleFileReadAdmin);
												
	/*on("/",[&](){												/ * Главная страница. * /
		handleFileRead(uri());
		taskPower.resume();
	});		
	
	
	//on("/fwlink", [this](){if (!handleFileRead("/index.html"))	this->send(404, "text/plain", "FileNotFound");});  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.	
	
	const char * headerkeys[] = {"User-Agent","Cookie"/ *,"x-SETNET"* /} ;
	size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
	//ask server to track these headers
	collectHeaders(headerkeys, headerkeyssize );*/	
	serveStatic("/secret.json", SPIFFS, "/").setDefaultFile("secret.json").setAuthentication(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str());
		
#ifdef HTML_PROGMEM
	on("/",[](AsyncWebServerRequest * reguest){	reguest->send_P(200,F("text/html"),index_html);});								/* Главная страница. */	 
	on("/global.css",[](AsyncWebServerRequest * reguest){	reguest->send_P(200,F("text/css"),global_css);});					/* Стили */
	on("/battery.png",handleBatteryPng);
	on("/scales.png",handleScalesPng);
	rewrite("/sn", "/settings.html");
	serveStatic("/", SPIFFS, "/");
#else
	serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
#endif
	onNotFound([](AsyncWebServerRequest *request){
		request->send(404);
	});
}

bool TapeRequestHandler::canHandle(AsyncWebServerRequest *request){
	if (request->host().equalsIgnoreCase("/tp")){
		/*if (SlaveScales.doTape()){
			Scale.tare();
			return true;
		}*/
	}
	return false;
}

void BrowserServerClass::send_wwwauth_configuration_html(AsyncWebServerRequest *request) {
	if (!checkAdminAuth(request))
		return request->requestAuthentication();
	if (request->args() > 0){  // Save Settings
		if (request->hasArg("wwwuser")){
			_httpAuth.wwwUsername = request->arg("wwwuser");
			_httpAuth.wwwPassword = request->arg("wwwpass");
		}		
		_saveHTTPAuth();
	}
	request->send(SPIFFS, request->url());
}

bool BrowserServerClass::_saveHTTPAuth() {
	
	DynamicJsonBuffer jsonBuffer(256);
	//StaticJsonBuffer<256> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["user"] = _httpAuth.wwwUsername;
	json["pass"] = _httpAuth.wwwPassword;

	//TODO add AP data to html
	File configFile = SPIFFS.open(SECRET_FILE, "w");
	if (!configFile) {
		configFile.close();
		return false;
	}

	json.printTo(configFile);
	configFile.flush();
	configFile.close();
	return true;
}

bool BrowserServerClass::_downloadHTTPAuth() {
	_httpAuth.wwwUsername = "sa";
	_httpAuth.wwwPassword = "343434";
	File configFile = SPIFFS.open(SECRET_FILE, "r");
	if (!configFile) {
		configFile.close();
		return false;
	}
	size_t size = configFile.size();

	std::unique_ptr<char[]> buf(new char[size]);
	
	configFile.readBytes(buf.get(), size);
	configFile.close();
	DynamicJsonBuffer jsonBuffer(256);
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		return false;
	}	
	_httpAuth.wwwUsername = json["user"].as<String>();
	_httpAuth.wwwPassword = json["pass"].as<String>();
	return true;
}

bool BrowserServerClass::checkAdminAuth(AsyncWebServerRequest * r) {	
	return r->authenticate(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str());
}

/*
void BrowserServerClass::restart_esp() {
	String message = "<meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1'/>";
	message += "<meta http-equiv='refresh' content='10; URL=/admin.html'>Please Wait....Configuring and Restarting.";
	send(200, "text/html", message);
	SPIFFS.end(); // SPIFFS.end();
	delay(1000);
	ESP.restart();
}*/

bool BrowserServerClass::isAuthentified(AsyncWebServerRequest * request){
	if (!request->authenticate(CORE->getNameAdmin(), CORE->getPassAdmin())){
		if (!checkAdminAuth(request)){
			return false;
		}
	}
	return true;
}

void handleFileReadAuth(AsyncWebServerRequest * request){
	if (!browserServer.isAuthentified(request)){
		return request->requestAuthentication();
	}
	request->send(SPIFFS, request->url());
}

#ifdef HTML_PROGMEM

void handleBatteryPng(AsyncWebServerRequest * request){
	//#ifdef HTML_PROGMEM
		AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", battery_png, battery_png_len);
		request->send(response);
	//#else
		//request->send(SPIFFS, request->url());
	//#endif
}

void handleScalesPng(AsyncWebServerRequest * request){
	//#ifdef HTML_PROGMEM
		AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", scales_png, scales_png_len);
		request->send(response);
	//#else
		//request->send(SPIFFS, request->url());
	//#endif
}

#endif

void handleSettings(AsyncWebServerRequest * request){
	if (!browserServer.isAuthentified(request))
		return request->requestAuthentication();
	#ifdef HTML_PROGMEM
		AsyncResponseStream *response = request->beginResponseStream("application/json");
		DynamicJsonBuffer jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		JsonObject& scale = root.createNestedObject(SCALE_JSON);
		scale["id_auto"] = CoreMemory.eeprom.settings.autoIp;
		scale["bat_max"] = CoreMemory.eeprom.settings.bat_max;
		//scale["id_pe"] = CoreMemory.eeprom.settings.power_time_enable;
		scale["id_assid"] = CoreMemory.eeprom.settings.apSSID;
		scale["id_n_admin"] = CoreMemory.eeprom.settings.scaleName;
		scale["id_p_admin"] = CoreMemory.eeprom.settings.scalePass;
		scale["id_lan_ip"] = CoreMemory.eeprom.settings.scaleLanIp;
		scale["id_gateway"] = CoreMemory.eeprom.settings.scaleGateway;
		scale["id_subnet"] = CoreMemory.eeprom.settings.scaleSubnet;
		scale["id_ssid"] = String(CoreMemory.eeprom.settings.wSSID);
		scale["id_key"] = String(CoreMemory.eeprom.settings.wKey);
		
		JsonObject& server = root.createNestedObject(SERVER_JSON);
		server["id_host"] = String(CoreMemory.eeprom.settings.hostUrl);
		server["id_pin"] = CoreMemory.eeprom.settings.hostPin;
		
		root.printTo(*response);
		request->send(response);
	#else
		request->send(SPIFFS, request->url());
	#endif
}

void handleScaleProp(AsyncWebServerRequest * request){
	if (!browserServer.isAuthentified(request))
		return request->requestAuthentication();
	AsyncJsonResponse * response = new AsyncJsonResponse();
	JsonObject& root = response->getRoot();
	root["id_date"] = getDateTime();
	root["id_local_host"] = WiFi.hostname();
	root["id_ap_ssid"] = String(WiFi.softAPSSID());
	root["id_ap_ip"] = toStringIp(WiFi.softAPIP());
	root["id_ip"] = toStringIp(WiFi.localIP());
	root["sl_id"] = String(Scale.getSeal());
	root["chip_v"] = String(ESP.getCpuFreqMHz());
	response->setLength();
	request->send(response);
	/*String values = "";
	values += "id_date|" + getDateTime() + "|div\n";
	values += "id_local_host|"+String(MY_HOST_NAME)+"/|div\n";
	values += "id_ap_ssid|" + String(SOFT_AP_SSID) + "|div\n";
	values += "id_ap_ip|" + toStringIp(WiFi.softAPIP()) + "|div\n";
	values += "id_ip|" + toStringIp(WiFi.localIP()) + "|div\n";
	values += "sl_id|" + String(Scale.getSeal()) + "|div\n";
	
	request->send(200, "text/plain", values);*/
}

/*
#if! HTML_PROGMEM
void handleAccessPoint(AsyncWebServerRequest * request){
	if (!browserServer.isAuthentified(request))
		return request->requestAuthentication();
	request->send(200, TEXT_HTML, netIndex);	
}
#endif*/

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
	if(type == WS_EVT_CONNECT){	
		client->ping();	
	} else if(type == WS_EVT_DISCONNECT){
	} else if(type == WS_EVT_ERROR){
	} else if(type == WS_EVT_PONG){
	} else if(type == WS_EVT_DATA){
		String msg = "";
		for(size_t i=0; i < len; i++) {
			msg += (char) data[i];
		}
		DynamicJsonBuffer jsonBuffer;
		JsonObject &root = jsonBuffer.parseObject(msg);
		if (!root.success()) {
			return;
		}
		const char *command = root["cmd"];			/* Получить показания датчика*/
		JsonObject& json = jsonBuffer.createObject();
		json["cmd"] = command;
		if (strcmp(command, "wt") == 0){
			JsonObject& master = json.createNestedObject("ms");
			JsonObject& slave = json.createNestedObject("sl");
			
			char b[10];
			float f = Scale.getTest() + SlaveScales.getWeigt();
			Scale.formatValue(f,b);
			String str = SlaveScales.isConnected()?String(b):String("slave???");
			json["w"] = str;
			
			master["w"]= String(Scale.getBuffer());
			master["c"]= BATTERY.getCharge();
			master["s"]= Scale.getStableWeight();
			
			slave["w"]= SlaveScales.s_weight;
			slave["c"]= SlaveScales.getCharge();
			slave["s"]= SlaveScales.getStable();
		}else if (strcmp(command, "tp") == 0){
			Scale.tare();
			SlaveScales.doTape();
		}else {
			return;
		}
		size_t lengh = json.measureLength();
		AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(lengh);
		if (buffer) {
			json.printTo((char *)buffer->get(), lengh + 1);
			if (client) {
				client->text(buffer);
			}
		}
		
		/*if (msg.equals("/wt")){						
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			JsonObject& master = json.createNestedObject("ms");
			JsonObject& slave = json.createNestedObject("sl");
			
			char b[10];
			float f = Scale.getTest() + SlaveScales.getWeigt();
			Scale.formatValue(f,b);
			String str = SlaveScales.isConnected()?String(b):String("slave???");
			json["w"] = str;
			
			master["w"]= String(Scale.getBuffer());
			master["c"]= BATTERY.getCharge();
			master["s"]= Scale.getStableWeight();
			
			slave["w"]= SlaveScales.s_weight;
			slave["c"]= SlaveScales.getCharge();
			slave["s"]= SlaveScales.getStable();
			
			size_t len = json.measureLength();
			AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
			if (buffer) {
				json.printTo((char *)buffer->get(), len + 1);
				if (client) {
					client->text(buffer);
				}
			}
			//String str = SlaveScales.isConnected()?String(SlaveScales.getWeigt()):String("Slave no conn");
			//client->text(String("{\"w\":\""+ str +"\",\"c\":"+String(SlaveScales.getCharge())+",\"s\":"+String(Scale.getStableWeight())+"}"));
		}*/
	}
}

