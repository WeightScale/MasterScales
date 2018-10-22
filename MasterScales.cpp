#include <ESP8266WiFi.h>
#include <functional>
#include "BrowserServer.h"
#include "ESP8266NetBIOS.h" 
#include "Core.h"
#include "Task.h"
#include "HttpUpdater.h"

using namespace std;
using namespace std::placeholders;
/*
 * This example serves a "hello world" on a WLAN and a SoftAP at the same time.
 * The SoftAP allow you to configure WLAN parameters at run time. They are not setup in the sketch but saved on EEPROM.
 * 
 * Connect your computer or cell phone to wifi network ESP_ap with password 12345678. A popup may appear and it allow you to go to WLAN config. If it does not then navigate to http://192.168.4.1/wifi and config it there.
 * Then wait for the module to connect to your wifi and take note of the WLAN IP it got. Then you can disconnect from ESP_ap and return to your regular WLAN.
 * 
 * Now the ESP8266 is in your network. You can reach it through http://192.168.x.x/ (the IP you took note of) or maybe at http://esp8266.local too.
 * 
 * This is a captive portal because through the softAP it will redirect any http request to http://192.168.4.1/
 */

void onStationModeConnected(const WiFiEventStationModeConnected& evt);
void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt);
//void takeBlink();
void takeBattery();
void takeWeight();
//void powerSwitchInterrupt();
void connectWifi();
void prinScanResult(int networksFound);
//
TaskController taskController = TaskController();		/*  */
//Task taskBlink(takeBlink, 500);							/*  */
Task taskBattery(takeBattery, 20000);					/* 20 Обновляем заряд батареи */
//Task taskPower(powerOff, 2400000);						/* 10 минут бездействия и выключаем */
Task taskConnectWiFi(connectWifi, 20000);				/* Пытаемся соедениться с точкой доступа каждые 60 секунд */
Task taskWeight(takeWeight,200);
WiFiEventHandler stationModeConnectedHandler;
WiFiEventHandler stationModeDisconnectedHandler;

unsigned int COUNT_FLASH = 500;
unsigned int COUNT_BLINK = 500;
#define USE_SERIAL Serial

void connectWifi();

void setup() {
	pinMode(EN_NCP, OUTPUT);
	digitalWrite(EN_NCP, HIGH);
	pinMode(LED, OUTPUT);	
	//Serial.begin(115200);
	/*while (digitalRead(PWR_SW) == HIGH){
		delay(100);
	};*/
	BLINK = new BlinkClass();
	SPIFFS.begin();
	browserServer.begin();
	Scale.setup(&browserServer);
	//delay(1000);	
	takeBattery();
  
	taskController.add(BLINK);
	taskController.add(&taskBattery);
	taskController.add(&taskWeight);
	
	//taskController.add(&taskPower);	

	stationModeConnectedHandler = WiFi.onStationModeConnected(&onStationModeConnected);	
	stationModeDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationModeDisconnected);
  
	//ESP.eraseConfig();
	WiFi.persistent(false);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	//WiFi.smartConfigDone();
	WiFi.mode(WIFI_AP_STA);
	WiFi.hostname(CORE->getHostname());
	WiFi.softAPConfig(apIP, apIP, netMsk);
	WiFi.softAP(CORE->getApSSID(), SOFT_AP_PASSWORD);
	delay(500); 
	taskController.add(&taskConnectWiFi);
	connectWifi();
	 
	//CORE.saveEvent("power", "ON");
}

/*********************************/
/* */
/*********************************/
/*
void takeBlink() {
	bool led = !digitalRead(LED);
	digitalWrite(LED, led);	
	taskBlink.setInterval(led ? COUNT_BLINK : COUNT_FLASH);
}*/

/**/
void takeBattery(){
	BATTERY.fetchCharge(1);
}

void takeWeight(){
	Scale.fetchWeight();
	//taskPower.updateCache();	
}

/*
void powerSwitchInterrupt(){
	if(digitalRead(PWR_SW)==HIGH){
		unsigned long t = millis();
		digitalWrite(LED, HIGH);
		while(digitalRead(PWR_SW)==HIGH){ // 
			delay(100);	
			if(t + 4000 < millis()){ // 
				digitalWrite(LED, HIGH);
				while(digitalRead(PWR_SW) == HIGH){delay(10);};// 
				powerOff();			
				break;
			}
			digitalWrite(LED, !digitalRead(LED));
		}
	}
}*/

void connectWifi() {
	taskConnectWiFi.pause();
	if (String(CORE->getSSID()).length() == 0) {
		WiFi.setAutoConnect(false);
		WiFi.setAutoReconnect(false);
		return;
	}
	if (WiFi.SSID().equals(CORE->getSSID())) {
		WiFi.begin();
		return;
	}
	WiFi.disconnect(false);
	/*!  */
	int n = WiFi.scanComplete();
	if (n == -2) {
		WiFi.scanNetworksAsync(prinScanResult, true);
	}else if (n > 0) {
		prinScanResult(n);
	}
}

void prinScanResult(int networksFound) {
	for (int i = 0; i < networksFound; ++i) {
		if (WiFi.SSID(i).equals(CORE->getSSID())) {
			WiFi.persistent(true);
			WiFi.begin(CORE->getSSID(), CORE->getPASS());
			return;
		}
	}
	WiFi.scanDelete();
	taskConnectWiFi.resume();
}

/*
void connectWifi() {
	WiFi.disconnect(false);
	/ *!  * /
	int n = WiFi.scanComplete();
	if(n == -2){
		n = WiFi.scanNetworks();
		if (n <= 0){
			goto scan;
		}
	}else if (n > 0){
		goto connect;
	}else{
		goto scan;
	}
	connect:
	//if (CORE->getSSID().length()==0){
	if (String(CORE->getSSID()).length()==0){	
		WiFi.setAutoConnect(false);
		WiFi.setAutoReconnect(false);
		taskConnectWiFi.pause();
		return;
	}
	for (int i = 0; i < n; ++i)	{
		//if(WiFi.SSID(i) == CORE->getSSID().c_str()){
		if(WiFi.SSID(i).equals(CORE->getSSID())){	
			int32_t chan_scan = WiFi.channel(i);
			WiFi.setAutoConnect(true);
			WiFi.setAutoReconnect(true);
			if (!CORE->isAuto()){
				if (lanIp.fromString(CORE->getLanIp()) && gateway.fromString(CORE->getGateway())){
					WiFi.config(lanIp,gateway, netMsk);									// Надо сделать настройки ip адреса
				}
			}
			WiFi.softAP(CORE->getApSSID(), SOFT_AP_PASSWORD, chan_scan); //Устанавливаем канал как роутера
			WiFi.begin ( CORE->getSSID(), CORE->getPASS(),chan_scan/ *,BSSID_scan* /);
			int status = WiFi.waitForConnectResult();
			if(status == WL_CONNECTED ){
				NBNS.begin(CORE->getHostname().c_str());
			}
			return;
		}
	}
	scan:
	WiFi.scanDelete();
	WiFi.scanNetworks(true);
}*/

void loop() {
	taskController.run();	
	//DNS
	dnsServer.processNextRequest();
	//HTTP
	/*if (SlaveScales.isTape()){
		float f;
		HTTPClient http;
		String host = "http://";
		host += SlaveScales.getClient()->remoteIP().toString();
		http.begin(host, 80, "/tp");
		http.setTimeout(3000);
		int httpCode = http.GET();
		if(httpCode == 204) {
			f = true;
		}else{
			f = false;
		}
		http.end();
		SlaveScales.setDoTape(false);
	}*/
	if (Scale.isSave()){
		CORE->saveEvent("weight", String(Scale.getSaveValue()));
		Scale.setIsSave(false);
	}
	
	//Scale.fetchWeight();
	//powerSwitchInterrupt();
}

void onStationModeConnected(const WiFiEventStationModeConnected& evt) {
	taskConnectWiFi.pause();
	WiFi.softAP(CORE->getApSSID(), SOFT_AP_PASSWORD, evt.channel); //Устанавливаем канал как роутера	
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	NBNS.begin(CORE->getHostname().c_str());
	BLINK->onRun(bind(&BlinkClass::blinkSTA,BLINK));
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt) {
	//Serial.println("DisconnectStation");
	//WiFi.scanDelete();
	//WiFi.scanNetworks(true);
	taskConnectWiFi.resume();
	BLINK->onRun(bind(&BlinkClass::blinkAP,BLINK));
}
