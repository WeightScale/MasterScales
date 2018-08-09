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
//using namespace ArduinoJson;

#define SETTINGS_FILE "/settings.json"
#define HOST_URL "sdb.net.ua"
#define TIMEOUT_HTTP 3000
#define STABLE_NUM_MAX 10
#define MAX_EVENTS 100
//#define MAX_CHG 1013//980	//делитель U2=U*(R2/(R1+R2)) 0.25
//#define MIN_CHG 880			//ADC = (Vin * 1024)/Vref  Vref = 1V
#define MIN_CHG 670			//ADC = (Vin * 1024)/Vref  Vref = 1V

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

const char netIndex[] PROGMEM = R"(<html lang='en'><meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1'/><body><form method='POST'><input name='ssid'><br/><input type='password' name='key'><br/><input type='submit' value='СОХРАНИТЬ'></form></body></html>)";

//settings.html
const char settings_html[] PROGMEM = R"(<!DOCTYPE html><html lang='en'> <head> <meta charset="UTF-8"><meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1.0, user-scalable=no'/> <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate"/> <meta http-equiv="Pragma" content="no-cache"/> <title>Настройки</title> <link rel="stylesheet" type="text/css" href="global.css"> <style>input:focus{background: #FA6;outline: none;}table{width:100%;}input,select{width:100%;text-align:right;font-size:18px;}input[type=submit]{width:auto;}input[type=checkbox]{width:auto;}</style> <script>function setOnBlur(ip){setTimeout(function(){if (document.activeElement===ip){ip.onblur=function(){if(ip.value.length===0 || !CheckIP(ip)){setTimeout(function(){ip.focus()},0);}else ip.onblur=null;}}},0)}function CheckIP(ip){ipParts=ip.value.split("."); if(ipParts.length===4){for(i=0;i<4;i++){TheNum=parseInt(ipParts[i]); if(TheNum>=0 && TheNum <=255){}else break;}if(i===4) return true;}return false;}function sendDateTime(){var fd=new FormData(); var date=new Date(); var d=date.toLocaleDateString(); d+="-"+date.toLocaleTimeString(); fd.append('data',d.replace(/[^\x20-\x7E]+/g,'')); var r=new XMLHttpRequest(); r.onreadystatechange=function(){if (r.readyState===4 && r.status===200){if(r.responseText !==null){document.getElementById('id_date').innerHTML="<div>Обновлено<br/>"+r.responseText+"</div>";}}}; r.open("POST","settings.html?"+new Date().getTime(),true); r.send(fd);}function GetValue(){var r=new XMLHttpRequest(); r.overrideMimeType('application/json'); r.onreadystatechange=function(){if(r.readyState===4){if(r.status===200){var j=JSON.parse(r.responseText); for(en in j){try{document.getElementById(en).innerHTML=j[en];}catch (e){}}}}}; r.open('GET','/sv',true); r.send(null);}function GetSettings(){var r=new XMLHttpRequest(); r.overrideMimeType('application/json'); r.onreadystatechange=function(){if (r.readyState===4){if(r.status===200){try{var j=JSON.parse(r.responseText); var s=j.scale; for(e in s){try{if(document.getElementById(e).type==='checkbox'){document.getElementById(e).checked=s[e];}else document.getElementById(e).value=s[e];}catch (e){}}var sr=j.server; for(en in sr){try{document.getElementById(en).value=sr[en];}catch (e){}}enableAuthFields(document.getElementById('id_auto'));}catch(e){alert("ОШИБКА "+e.toString());}}else{alert("ДАННЫЕ НАСТРОЕК НЕ НАЙДЕНЫ!!!");}document.body.style.visibility='visible'; GetValue();}}; r.open('GET','/settings.json', true); r.send(null);}window.onload=function(){GetSettings();}; function openSDB(){var url='https://'+document.getElementById('id_host').value+'/scale.php?code='+document.getElementById('id_pin').value; var win=window.open(url,'_blank'); win.focus();}function enableAuthFields(check){if(check.checked){document.getElementById('id_table_net').style.display='none';}else{document.getElementById('id_table_net').style.display='';}}function formNet(i){var f=document.getElementById(i); var fd=new FormData(f); var r=new XMLHttpRequest(); r.onreadystatechange=function(){if(r.readyState===4){if(r.status===200){var rec=confirm('Пересоеденится с новыми настройками'); if(rec){r.onreadystatechange=null; r.open('GET','/rc',true); r.send(null);}}else if(r.status===400){alert('Ошибка при сохранении настроек');}}}; r.open('POST','/settings.html',true); r.send(fd);}</script> </head><body style='visibility: hidden'><a href='/' class='btn btn--s btn--blue'>&lt;</a>&nbsp;&nbsp;<strong>Настройки</strong><hr><fieldset><details><summary>Конфигурация сети</summary><br><h5 align='left'><b>Точка доступа WiFi роутера</b></h5> <form id='form_id' action='javascript:formNet("form_id")'> Получать IP:<input type='checkbox' id='id_auto' name='auto' onclick='enableAuthFields(this);'> <div id='id_ip'></div><table id='id_table_net' > <tr> <td>IP:</td><td><input id='id_lan_ip' type='text' name='lan_ip' onfocus='setOnBlur(this)'></td></tr><tr> <td>ШЛЮЗ:</td><td><input id='id_gateway' type='text' name='gateway' onfocus='setOnBlur(this)'></td></tr><tr> <td>МАСКА:</td><td><input id='id_subnet' type='text' name='subnet' onfocus='setOnBlur(this)'></td></tr></table> <table> <tr> <td>СЕТЬ:</td><td><input id='id_ssid' name='ssid' placeholder='имя сети'></td></tr><tr> <td>КЛЮЧ:</td><td><input id='id_key' type='password' name='key' placeholder='пароль'></td></tr><tr> <td></td><td><input type='submit' value='СОХРАНИТЬ'/></td></tr></table> </form></details></fieldset><br/><fieldset style='width: auto'><details><summary>Общии настройки</summary><br><form action='javascript:sendDateTime()'><h5 align='left'><b>Установка дата время</b></h5> <table> <tr> <td><h5 id='id_date'>дата время</h5></td><td><input type='submit' name='data' value='УСТАНОВИТЬ'/></td></tr></table> </form><hr> <form method='POST'><h5 align='left'><b>Время выключения</b></h5> <table> <tr> <td><input type='checkbox' id='id_pe' name='pe'></td><td> <select id='id_pt' name='pt' title="Время выключения"> <option value='600000'>10мин</option> <option value='1200000'>20мин</option> <option value='1800000'>30мин</option> <option value='2400000'>40мин</option> </select> </td><td><input type='submit' value='УСТАНОВИТЬ'/></td></tr></table> </form><hr> <form method='POST'><h5>Настройки база данных интернет</h5> <table> <tr> <td>СЕРВЕР:</td><td ><input id='id_host' name='host' placeholder='сервер'></td></tr><tr> <td>ПИН:</td><td><input id='id_pin' name='pin' placeholder='пин весов'></td></tr><tr> <td><a href='javascript:openSDB();'>открыть</a></td><td><input id='id_submit_code' type='submit' value='СОХРАНИТЬ'/></td></tr></table> </form><hr> <form method='POST'><h5>Доступ к настройкам</h5> <table> <tr> <td>ИМЯ:</td><td><input id='id_n_admin' name='n_admin' placeholder='имя админ'></td></tr><tr> <td>ПАРОЛЬ:</td><td><input type='password' id='id_p_admin' name='p_admin' placeholder='пароль админ'></td></tr><tr> <td></td><td><input type='submit' value='СОХРАНИТЬ'/></td></tr></table> </form></details></fieldset><br/><fieldset><details><summary>Информация</summary><br><table> <tr> <td><h5>Имя хоста:</h5></td><td align='right'><h5 id='id_local_host'></h5></td></tr></table><hr><h5 align='left'><b>Точка доступа весов</b></h5> <table> <tr> <td id='id_ap_ssid'></td><td align='right' id='id_ap_ip'></td></tr></table><hr> <table> <tr> <td>Пломба:</td><td align='right'><div id='sl_id'></div></td></tr></table><hr> <a href='/calibr.html'>калибровка</a></details></fieldset><hr><footer align='center'>2018 © Powered by www.scale.in.ua</footer></body></html>)";

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
} settings_t;

class CoreClass : public AsyncWebHandler{
	private:
	settings_t _settings;
	
	String _username;
	String _password;
	bool _authenticated;
	
	bool saveAuth();
	bool loadAuth();		
	bool _downloadSettings();
			

	public:			
		CoreClass(const String& username, const String& password);
		~CoreClass();
		void begin();
		bool saveSettings();
		String& getNameAdmin(){return _settings.scaleName;};
		String& getPassAdmin(){return _settings.scalePass;};
		String& getSSID(){return _settings.scaleWlanSSID;};
		String& getLanIp(){return _settings.scaleLanIp;};
		String& getGateway(){return _settings.scaleGateway;};
		void setSSID(const String& ssid){_settings.scaleWlanSSID = ssid;};
		void setPASS(const String& pass){_settings.scaleWlanKey = pass;};	
		String& getPASS(){return _settings.scaleWlanKey;};
		bool saveEvent(const String&, const String&);
		//void setBatMax(int m){_settings.bat_max = m;};
		String getIp();
		bool eventToServer(const String&, const String&, const String&);
		/*#if! HTML_PROGMEM
			void saveValueSettingsHttp(AsyncWebServerRequest*);
		#endif*/			
		void handleSetAccessPoint(AsyncWebServerRequest*);	
		String getHash(const int, const String&, const String&, const String&);
		int getPin(){return _settings.hostPin;};
				
		
		bool isAuto(){return _settings.autoIp;};		
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


//void powerOff();
void reconnectWifi(AsyncWebServerRequest*);
extern CoreClass * CORE;
extern BatteryClass BATTERY;
//extern Task POWER;

#endif //_CORE_h







