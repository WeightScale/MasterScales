// SlaveScalesClass.h

#ifndef _SLAVESCALES_h
#define _SLAVESCALES_h

#include <AsyncWebSocket.h>
#include <ESP8266HTTPClient.h>
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class SlaveScalesClass : public AsyncWebSocket{
	protected:
		uint32 _clientId;
		AsyncWebSocketClient * _client;
		float _weight;
		
		unsigned int _charge;
		bool _stableWeight;
		bool _doTape;
		bool _connected = false;
		long _time_connect;
		String _url;
	
	public:
		SlaveScalesClass(const String& url);
		~SlaveScalesClass();
		String s_weight;
		void events(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
		void init();
		void setClient(AsyncWebSocketClient * c){_client = c;};
		AsyncWebSocketClient * getClient(){return _client;};
		void doTape(){
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			JsonObject& tp = json.createNestedObject("tp");
			size_t len = json.measureLength();
			AsyncWebSocketMessageBuffer * buffer = makeBuffer(len);
			if (buffer) {
				json.printTo((char *)buffer->get(), len + 1);
				if (_client) {
					_client->text(buffer);
				}
			}
		}		
		void powerDown(){
			if (_client){
				_client->text("/po");
			}	
		}
		bool doValueUpdate(AsyncWebServerRequest * request);				
		uint8_t getCharge(){return _charge;};
		
		float getWeight(){return _weight;};
		void setWeight(float w){_weight = w;};
	
		void setStable(bool s){_stableWeight = s;};
		bool getStable(){return _stableWeight;};
		
		void setDoTape(float t){_doTape = t;};
		bool isTape(){return _doTape;};
		bool isConnected(){
			if((_time_connect + 5000)> millis())
				return true;	
			return false;
		};
		String getUrl(){return _url;};
		//bool doTape();
};


extern SlaveScalesClass SlaveScales;
//extern HTTPClient http;

#endif

