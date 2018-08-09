#include <ArduinoJson.h>
#include <functional>
#include "SlaveScales.h"

using namespace std::placeholders;
SlaveScalesClass SlaveScales("/ss");
//SlaveScalesClass SlaveScales;
//HTTPClient http;

SlaveScalesClass::SlaveScalesClass(const String& url) : AsyncWebSocket(url){
	onEvent(std::bind(&SlaveScalesClass::events, this, _1, _2, _3, _4, _5, _6));
}

SlaveScalesClass::~SlaveScalesClass(){}
	
void SlaveScalesClass::init(){
	
	//onEvent(events);
}
	
void SlaveScalesClass::events(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){	
	if(type == WS_EVT_CONNECT){
		SlaveScales.setClient(client);
		//client->ping();
		//_connected = true;
	} else if(type == WS_EVT_DISCONNECT){
		//_connected = false;
	} else if(type == WS_EVT_ERROR){
	} else if(type == WS_EVT_PONG){
	} else if(type == WS_EVT_DATA){
		_time_connect = millis();
		DynamicJsonBuffer jsonBuffer(len);
		JsonObject& json = jsonBuffer.parseObject(data);
		if (json.containsKey("slave")) {
			JsonObject& slave = json["slave"]; 
			s_weight = slave["w"].as<String>();
			_weight = s_weight.toFloat();
			//_weight = slave["w"].as<float>();
			_charge = slave["c"];
			_stableWeight = json["slave"]["s"].as<bool>();
			//_charge = json["slave"]["c"];
			//setWeight(slave["w"].as<float>());
			//setStable(slave["s"].as<bool>());
		}else if(json.containsKey("ip")){
			_url = json["ip"].as<String>();
		}
	}
}

bool SlaveScalesClass::doValueUpdate(AsyncWebServerRequest * request){	
	if (request->args() > 0) {
		if (request->hasArg("update")){
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			JsonObject& update = json.createNestedObject("up");
			update["ac"]= request->arg("weightAccuracy").toInt();
			update["ws"]= request->arg("weightStep").toInt();
			update["wa"]= request->arg("weightAverage").toInt();
			update["wf"]= request->arg("weightFilter").toInt();
			update["wm"]= request->arg("weightMax").toInt();
			size_t len = json.measureLength();
			AsyncWebSocketMessageBuffer * buffer = makeBuffer(len);
			if (buffer) {
				json.printTo((char *)buffer->get(), len + 1);
				if (_client) {
					_client->text(buffer);
					return true;
				}
			}			
		}		
	}
	return false;	
}

/*
bool SlaveScalesClass::doTape(){
	/ *if (_server == NULL){
		return false;	
	}
	
	_server->client(_clientId)->text("/tp");* /
	
	//_client->text("/tp");
	float f = false;
	String host = "http://192.168.4.100";
	//host += _client->remoteIP().toString();
	http.begin( host , 80, "/tp");
	http.addHeader("Content-Type", "application/x-www-form-urlencoded");
	http.setTimeout(10000);	
	//int httpCode = http.POST("wt="+String(Scale.getUnits()));
	int httpCode = http.GET();
	
	if(httpCode == 204) {
		f = true;
	}
	http.end();
	return f;
}*/



