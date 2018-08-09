// Updater.h

#ifndef _HTTPUPDATER_h
#define _HTTPUPDATER_h
#include "BrowserServer.h"

const char successResponse[] /*PROGMEM*/ = R"(<META http-equiv='refresh' content='15;URL=/'>Обновление успешно! Перегрузка...)";

const char serverIndex[] /*PROGMEM*/ = R"(<html><body><form method='POST' action='' enctype='multipart/form-data'>
										<input type='file' name='update'>
										<input type='submit' value='Update'>
										</form></body></html>)";

class HttpUpdaterClass: public AsyncWebHandler{
	public:
		//HttpUpdaterClass();
		HttpUpdaterClass(const String& username=String(), const String& password=String());
		
		/*void setup(BrowserServerClass *server, const char * username, const char * password){
			setup(server, "/update", username, password);
		}

		void setup(BrowserServerClass *server, const char * path, const char * username, const char * password);*/
		
		BrowserServerClass *getServer(){ return _server;};
		//char * getUserName(){return _username;};
		//char * getPassword(){return _password;};
		void setAuthenticated(bool a){_authenticated = a;};
		bool getAuthenticated(){return _authenticated;};
		void handleHttpStartUpdate(AsyncWebServerRequest*);
		virtual bool canHandle(AsyncWebServerRequest *request) override final;
		virtual void handleRequest(AsyncWebServerRequest *request) override final;
		virtual void handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) override final;	
		virtual bool isRequestHandlerTrivial() override final {return false;}

	protected:
		

	private:		
		BrowserServerClass *_server;
		String _username;
		String _password;
		//char * _username;
		//char * _password;
		bool _authenticated;
		
		
};

//extern HttpUpdaterClass httpUpdater;

//void handleUpdatePage(AsyncWebServerRequest*);
//void handleStartUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
//void handleEndUpdate(AsyncWebServerRequest*);
void setUpdaterError();


#endif //_HTTPUPDATER_h

