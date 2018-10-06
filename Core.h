// scales.h

#ifndef _CORE_h
#define _CORE_h

#include "TaskController.h"
#include "Task.h"

/*
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif*/
#include <ArduinoJson.h>
#include "Scale.h"
#include "CoreMemory.h"
//using namespace ArduinoJson;

#define SETTINGS_FILE "/settings.json"
#define HOST_URL "sdb.net.ua"
#define TIMEOUT_HTTP 3000
#define STABLE_NUM_MAX 10
#define MAX_EVENTS 100
//#define MAX_CHG 1013//980	//делитель U2=U*(R2/(R1+R2)) 0.25
//#define MIN_CHG 880			//ADC = (Vin * 1024)/Vref  Vref = 1V


#define EN_NCP  12							/* сигнал включения питания  */
#define PWR_SW  13							/* сигнал от кнопки питания */
#define LED  2								/* индикатор работы */

#define SCALE_JSON		"scale"
#define SERVER_JSON		"server"
//#define DATE_JSON		"date"
#define EVENTS_JSON		"events"

extern TaskController taskController;		/*  */
extern Task taskBlink;								/*  */
extern Task taskBattery;							/*  */
extern Task taskConnectWiFi;
//extern Task taskPower;
extern void connectWifi();





/*
typedef struct {	
	bool autoIp;
	//bool power_time_enable;
	String scaleName;
	String scalePass;
	String scaleLanIp;
	String scaleGateway;
	String scaleSubnet;
	String scaleWlanSSID;
	String scaleWlanKey;
	String hostUrl;
	int hostPin;
	int timeout;
	//int time_off;
	int bat_max;	
} settings_t;*/

class CoreClass : public AsyncWebHandler{
	private:
	settings_t * _settings;
	
	String _hostname;
	String _username;
	String _password;
	bool _authenticated;
	
	bool saveAuth();
	bool loadAuth();		
	//bool _downloadSettings();
			

	public:			
		CoreClass(const String& username, const String& password);
		~CoreClass();
		void begin();
		//bool saveSettings();
		char* getNameAdmin(){return _settings->scaleName;};
		char* getPassAdmin(){return _settings->scalePass;};
		char* getSSID(){return _settings->wSSID;};
		char* getLanIp(){return _settings->scaleLanIp;};
		char* getGateway(){return _settings->scaleGateway;};
		char* getApSSID(){return _settings->apSSID;};
		//void setSSID(char * ssid){_settings->wSSID = ssid;};
		//void setPASS(char * pass){_settings->wKey = pass;};	
		char* getPASS(){return _settings->wKey;};
		bool saveEvent(const String&, const String&);
		//void setBatMax(int m){_settings.bat_max = m;};
		String getIp();
		String& getHostname(){return _hostname;};
		bool eventToServer(const String&, const String&, const String&);
		/*#if! HTML_PROGMEM
			void saveValueSettingsHttp(AsyncWebServerRequest*);
		#endif*/			
		void handleSetAccessPoint(AsyncWebServerRequest*);	
		String getHash(const int, const String&, const String&, const String&);
		int getPin(){return _settings->hostPin;};
				
		
		bool isAuto(){return _settings->autoIp;};		
		virtual bool canHandle(AsyncWebServerRequest *request) override final;
		virtual void handleRequest(AsyncWebServerRequest *request) override final;
		virtual bool isRequestHandlerTrivial() override final {return false;}
		
		
};

class BatteryClass{	
	protected:
		unsigned int _charge;
		int _max;
		int _get_adc(byte times = 1);
	
	public:
		BatteryClass(){};
		~BatteryClass(){};
		int fetchCharge(int);
		bool callibrated();		
		void setCharge(unsigned int ch){_charge = ch;};
		unsigned int getCharge(){return _charge;};
		void setMax(int m){_max = m;};	
		int getMax(){return _max;};
};

class BlinkClass : public Task {
	public:
	unsigned int _flash = 500;
	//unsigned int _blink = 500;
	public:
	BlinkClass() : Task(500){
		pinMode(LED, OUTPUT);
		onRun(std::bind(&BlinkClass::blinkAP,this));
	};
	void blinkSTA(){
		static unsigned char clk;
		bool led = !digitalRead(LED);
		digitalWrite(LED, led);
		if (clk < 6){
			led ? _flash = 70 : _flash = 40;
			clk++;
			}else{
			_flash = 2000;
			digitalWrite(LED, HIGH);
			clk = 0;
		}
		setInterval(_flash);
	}
	void blinkAP(){
		bool led = !digitalRead(LED);
		digitalWrite(LED, led);
		setInterval(500);
	}
};


//void powerOff();
void reconnectWifi(AsyncWebServerRequest*);
extern CoreClass * CORE;
extern BatteryClass BATTERY;
extern BlinkClass * BLINK;

#endif //_CORE_h







