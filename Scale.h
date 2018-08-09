
#ifndef _SCALE_h
#define _SCALE_h

/*
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif*/
#include "HX711.h"
#include "Filter.h"
#include "BrowserServer.h"

#define PAGE_FILE			"/calibr.html"
#define CDATE_FILE			"/cdate.json"

#define WEIGHT_MAX_JSON		"mw_id"
#define OFFSET_JSON			"ofs"
#define AVERAGE_JSON		"av_id"
#define STEP_JSON			"st_id"
#define ACCURACY_JSON		"ac_id"
#define SCALE_JSON			"scale"
#define FILTER_JSON			"fl_id"
#define SEAL_JSON			"sl_id"
#define USER_JSON			"us_id"
#define PASS_JSON			"ps_id"

const char calibr_html[] PROGMEM = R"(<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1.0, user-scalable=no'/> <meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/> <meta http-equiv='Pragma' content='no-cache'/> <title>Калибровка</title> <link rel='shortcut icon' href='favicon.png' type='image/png'> <link rel='stylesheet' type='text/css' href='global.css'> <style>#wt_id{display:inline;background: #fff;font-size:28px;font-family:Arial,sans-serif;color:#618ad2;margin-left:auto;margin-right:auto;}table{width:100%;}input{width:100%;text-align:right;font-size:18px;height:auto;}input[type='submit']{width:auto;}select{width:100%;text-align-last:right;font-size:18px;height:auto;border:1px solid #ccc;}</style> <script>var weight;var w;var ip_sl,url; function ScalesSocket(h,p,fm,fe){var host=h; var protocol=p; var timerWeight; var timerSocket; var ws; var startWeightTimeout=function(){clearTimeout(timerWeight); timerWeight=setTimeout(function(){fe();},5000);}; this.getWeight=function(){ws.send('/wt'); startWeightTimeout();}; this.openSocket=function(){ws=new WebSocket(host,protocol); ws.onopen=this.getWeight; ws.onclose=function(e){clearTimeout(timerWeight); starSocketTimeout(); fe();}; ws.onerror=function(e){fe();}; ws.onmessage=function(e){fm(JSON.parse(e.data));}}; var starSocketTimeout=function(){clearTimeout(timerSocket); timerSocket=setTimeout(function(){this.prototype.openSocket();},5000);}}function go(){document.getElementById('wt_id').innerHTML='---';}function setMax(){var form=document.getElementById('form_c_id'); var formData=new FormData(form); formData.append('update',true); var request=new XMLHttpRequest(); request.onreadystatechange=function(){if(this.readyState===4 && this.status===200){if(this.responseText !==null){if(document.getElementById("form_zero")===null){document.getElementById("id_bs").value='ОБНОВИТЬ'; var f=document.createElement('fieldset'); f.id='form_zero'; f.innerHTML="<legend>Нулевой вес</legend><form action='javascript:setZero()'><p>Перед установкой убедитесь что весы не нагружены.</p><input type='submit' value='УСТАНОВИТЬ НОЛЬ'/></form><br><div id='wt_id'>---</div>";document.body.appendChild(f);setupWeight();}}}}; request.open('POST','calibr.html',true); request.send(formData);}function setZero(){var req=new XMLHttpRequest(); var formData=new FormData(); formData.append('zero',true); formData.append('weightCal','0'); req.onreadystatechange=function(){if(this.readyState===4 && this.status===200){if(this.responseText !==null){document.getElementById('form_zero').disabled=true; var f=document.createElement('fieldset'); f.id='form_weight'; f.innerHTML="<legend>Калиброваный вес</legend><form action='javascript:setWeight()'><p>Перед установкой весы нагружаются контрольным весом. Дать некоторое время для стабилизации.Значение вводится с точностью которое выбрано в пункте Точность измерения.</p><table><tr><td>ГИРЯ:</td><td><input id='gr_id' value='0' type='number' step='any' required placeholder='Калиброваная гиря'/></td></tr><tr><td>ГРУЗ:</td><td><input id='id_cal_wt' value='0' type='number' step='any' title='Введите значение веса установленого на весах' max='100000' required placeholder='Калиброваный вес'/></td></tr><tr><td>ОШИБКА:</td><td><div id='dif_gr_id'></div></td></tr><tr><td><input type='submit' value='УСТАНОВИТЬ'/></td><td><a href='javascript:calculate();'>подобрать</a></td></tr></table></form>";document.body.appendChild(f);}}}; req.open('POST','calibr.html',true); req.send(formData);}function setWeight(){var fd=new FormData(); var w=parseFloat(document.getElementById('id_cal_wt').value) + parseFloat(document.getElementById('gr_id').value); fd.append('weightCal',w.toString()); var request=new XMLHttpRequest(); request.onreadystatechange=function(){if(this.readyState===4){if(this.status===200){if(this.responseText !==null){if(document.getElementById('form_seal')===null){var f=document.createElement('fieldset'); f.id='form_seal'; f.innerHTML="<legend>Пломбировка</legend><form action='javascript:setSeal()'><left><p>Сохранение процесса калибровки. Данные калибровки сохраняются в память весов.</p><input type='submit' value='ОПЛОМБИРОВАТЬ'/></left></form>";document.body.appendChild(f);}}}else if(this.status===400){alert(this.responseText);}}}; request.open('POST','calibr.html',true); request.send(fd);}function setSeal(){var req=new XMLHttpRequest(); req.onreadystatechange=function(){if(this.readyState===4 && this.status===200){alert('Номер пломбы: '+this.responseText); window.location.replace('/');}}; req.open('GET','/sl',true); req.send(null);}function GetSettings(){var req=new XMLHttpRequest(); req.overrideMimeType('application/json'); req.onreadystatechange=function(){if(req.readyState===4){try{var json=JSON.parse(req.responseText); for(entry in json){try{if(document.getElementById(entry)!==null) document.getElementById(entry).value=json[entry];}catch(e){}}}catch(e){alert(e.toString());}document.body.style.visibility='visible';}}; req.open('GET','/cdate.json',true); req.send(null);}window.onload=function(){GetSettings();}; function saveValue(){var f=document.getElementById('form_c_id'); var fd=new FormData(f); fd.append('update',true); var req=new XMLHttpRequest(); req.onreadystatechange=function(){if(this.readyState===4 && this.status===200){if(this.responseText !==null){window.open('/','_self');}}}; req.onerror=function(){alert('Ошибка');}; req.open('POST','calibr.html',true); req.send(fd);}; function getWeight(){w=new ScalesSocket('ws://'+document.location.host+'/ws',['arduino'],handleWeight,function(){go();w.openSocket();}); w.openSocket();}function calculate(){var dif=weight-parseFloat(document.getElementById('id_cal_wt').value); dif=parseFloat(document.getElementById('gr_id').value)/dif; dif=parseFloat(document.getElementById('id_cal_wt').value)*dif; document.getElementById('id_cal_wt').value=dif.toFixed(document.getElementById('ac_id').value); setWeight();}function handleWeight(d){weight=d.ms.w; document.getElementById('wt_id').innerHTML=weight; try{document.getElementById('form_seal').disabled=(d.ms.s===false);}catch(e){}if(d.ms.s){document.getElementById('wt_id').setAttribute('style','background: #fff;');}else{document.getElementById('wt_id').setAttribute('style','background: #C4C4C4;');}try{var dif_gr=parseFloat(document.getElementById('id_cal_wt').value)+parseFloat(document.getElementById('gr_id').value); dif_gr -=weight; document.getElementById('dif_gr_id').innerHTML=dif_gr.toFixed(document.getElementById('ac_id').value);}catch (e){}w.getWeight();}function setupWeight(){getWeight();}</script></head><body style='visibility: hidden'><a href='/settings.html' class='btn btn--s btn--blue'>&lt;</a>&nbsp;&nbsp;<strong>Калибровка</strong><hr><fieldset id='form_max' style='visibility: visible'> <legend>Общии настройки</legend> <form id='form_c_id' action='javascript:setMax()'> <table> <tr> <td>Точность измерения</td><td> <select id='ac_id' name='weightAccuracy' title='Выбор точности с которым будет измерения'> <option name='0' value='0'>0</option> <option name='0.0' value='1'>0.0</option> <option name='0.00' value='2'>0.00</option> <option name='0.000' value='3'>0.000</option> </select> </td></tr><tr> <td>Шаг измерения</td><td> <select id='st_id' name='weightStep' title='Выбор шага измерения'> <option name='шаг 1' value='1'>1</option> <option name='шаг 2' value='2'>2</option> <option name='шаг 5' value='5'>5</option> <option name='шаг 10' value='10'>10</option> <option name='шаг 20' value='20'>20</option> <option name='шаг 50' value='50'>50</option> </select> </td></tr><tr> <td>Кол-во измерений</td><td> <select id='av_id' name='weightAverage' title='Выбор количества измерений АЦП'> <option name='одно' value='1'>ОДИН</option> <option name='два' value='2'>ДВА</option> </select> </td></tr><tr> <td>Фильтр</td><td> <select id='fl_id' name='weightFilter' title='Выбор значения фильтра. Чем меньше значение тем лутшее фильтрация но дольше измерение'> <option name='5%' value='5'>5 %</option> <option name='10%' value='10'>10 %</option> <option name='20%' value='20'>20 %</option> <option name='30%' value='30'>30 %</option> <option name='40%' value='40'>40 %</option> <option name='50%' value='50'>50 %</option> <option name='60%' value='60'>60 %</option> <option name='70%' value='70'>70 %</option> <option name='80%' value='80'>80 %</option> <option name='90%' value='90'>90 %</option> <option name='100%' value='100'>100 %</option> </select> </td></tr><tr> <td>НВП</td><td> <input title='Введите значение максимального веса' type='number' min='1' max='100000' id='mw_id' name='weightMax' placeholder='Найбольший предел'> </td></tr><tr> <td> <a href='javascript:saveValue();'>сохранить и выйти</a> </td><td> <input id='id_bs' type='submit' value='ДАЛЬШЕ >>'/> </td></tr></table> </form></fieldset><fieldset> <details><summary>Авторизация для калибровки</summary> <form method='POST'> <table> <tr> <td>ИМЯ:</td><td> <input id='us_id' name='user' placeholder='имя админ'> </td></tr><tr> <td>ПАРОЛЬ:</td><td><input type='password' id='ps_id' name='pass' placeholder='пароль админ'></td></tr><tr> <td></td><td> <input type='submit' value='СОХРАНИТЬ'/></td></tr></table> </form> </details></fieldset></body></html>)";

typedef struct {
	long offset;		/*  */
	unsigned char average;				/*  */
	unsigned char step;					/*  */
	int accuracy;					/*  */
	//unsigned char w_filter; /*! Значение для фильтра от 1-100 % */
	unsigned int max;					/*  */
	float scale;
	int seal;
	String user;
	String password;
}t_scales_value;

typedef struct{
	bool isSave;
	float value;
}t_save_value;

class BrowserServerClass;

class ScaleClass : public HX711 , public ExponentialFilter<long>{
	private:
		float _weight;
		char _buffer[10];
	protected:
		BrowserServerClass *_server;
		//char _buffer[10];
		bool _authenticated;
		bool stableWeight;
		float weightSlave;
		t_save_value saveWeight;
		t_scales_value _scales_value;
		float _round;						/* множитиль для округления */
		float _stable_step;					/* шаг для стабилизации */
		//float _referenceWeight;				/*  */
		//long _calibrateWeightValue;			/*  */
				
			
		bool _downloadValue();

	public:
		ScaleClass(byte, byte);
		~ScaleClass();
		void setup(BrowserServerClass *server/*, const char * username, const char * password*/);	
		bool saveDate();
		void saveValueCalibratedHttp(AsyncWebServerRequest *);
		void handleWeight(AsyncWebServerRequest*);
		void fetchWeight();
		void mathScale(float referenceW, long calibrateW);
		void mathRound();
		//void setScale(d_type scale = 1.f){_scales_value.scale = scale;};
		//d_type getScale(){return _scales_value.scale;};
		char * getBuffer(){return _buffer;};
		float getTest(){return _weight;};
		void setTest(float f){_weight = f;};	
		void setOffset(long offset = 0){_scales_value.offset = offset;};
		long getOffset(){return _scales_value.offset;};
		void init();	
		long readAverage();
		long getValue();
		void setAverage(unsigned char);
		unsigned char getAverage(){return _scales_value.average;};	
		void setSeal(int s){_scales_value.seal = s; };
		int getSeal(){ return _scales_value.seal;};	
		BrowserServerClass *getServer(){ return _server;};
		void detectStable(float);
		
		float getUnits();
		float getWeight();
		void tare();
		
		void formatValue(float value, char* string);
		float getStableStep(){return _stable_step;};
		
		float getRound(){return _round;};
		bool getStableWeight(){return stableWeight;};
		void setStableWeight(bool s){stableWeight = s;};
			
		float isSave(){return saveWeight.isSave;};
		float getSaveValue(){return saveWeight.value;};
		void setIsSave(bool s){saveWeight.isSave = s;};
		
		float forTest(uint32_t h);		
		void setWeightSlave(float w){weightSlave = w;};
		float getWeightSlave(){return weightSlave;};
		
		//void setMax(unsigned int m){_scales_value.max = m;};
		//void setStep(unsigned char s){_scales_value.step = s;};
		//void setAccuracy(int a){_scales_value.accuracy = a;};
		//void setCalibrateZeroValue(long z){_calibrateZeroValue = z;};
		//void setCalibrateWeightValue(long w){_calibrateWeightValue = w;};	
		//void setReferenceWeight(d_type r){_referenceWeight = r;};			
};

extern ScaleClass Scale;
void handleSeal(AsyncWebServerRequest*);
void handleSlave(AsyncWebServerRequest*);



#endif

