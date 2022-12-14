//esp32 WROOM arduino
//https://techtutorialsx.com/2018/01/07/esp32-arduino-http-server-over-soft-ap/
// client https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
//       https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/examples/ESP_AsyncFSBrowser/ESP_AsyncFSBrowser.ino
//       https://github.com/me-no-dev/AsyncTCP
//       https://techtutorialsx.com/2018/05/05/esp32-arduino-temperature-humidity-and-co2-concentration-web-server/
// dallas more sensors https://randomnerdtutorials.com/esp32-with-multiple-ds18b20-temperature-sensors/
//hw  https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
// hw ESP32 Wroover module, standard configuration
// ntp client from https://github.com/gmag11/NtpClient
// cidla kabel hneda +5V; zelena GND; zluta DATA
// Jiri Liska, liska.tbn@gmail.com, Trebon, Czech Rep.
// new hardware
//20200131001
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include <TimeLib.h> //TimeLib library is needed https://github.com/PaulStoffregen/Time !!version 2.5.0
//is strictly necessary to use Version 2.5.0, in new versions is there error in ip address taking from 
//esp 32 boards definition 
#include <NtpClientLib.h> //Include NtpClient library header  https://github.com/gmag11/NtpClient
#include "passwords.h"
//#include <NTPClient.h>

//<SETUP>
const byte DallasPin=19;
const byte DallasPowerPin=32;
const byte LED=13; 
const bool WebServerOn=1;
const bool WebServer1On=0;
const String deviceName="02_Skleniky prava strana (vychodni)";
//<SENSOR1>
const bool Sensor1=1;
//DeviceAddress internal1Adr={0x28,0xaa,0x96,0x83,0x1d,0x13,0x2,0xa8};
DeviceAddress internal1Adr={0x28,0xaa,0x43,0x84,0x1d,0x13,0x2,0xa2};
const String Sensor1Place="Teplota chodba prava: ";
//</SENSOR1>
//<SENSOR2>
const bool Sensor2=1;
//DeviceAddress external2Adr={0x28,0xaa,0x43,0x84,0x1d,0x13,0x2,0xa2};
DeviceAddress external2Adr={0x28,0xaa,0x96,0x83,0x1d,0x13,0x2,0xa8};
const String Sensor2Place="Teplota koje krajni sever: ";
//</SENSOR2>
//<SENSOR3>
const bool Sensor3=1;
DeviceAddress external3Adr={0x28,0xff,0x36,0xa0,0x42,0x18,0x1,0x43};
//0x30,0x1f,0xfb,0x3f,0x40,0x3c,0xfc,0x3f
const String Sensor3Place="Teplota koje krajni jih: ";
const float calibration3=0.0;
//</SENSOR3>
int counter;
const float tempLimit=5.0;
float t1=-99.0, t2=-99.0,t3=-99.0,t4=-99.0;
String bcgcolor="red";
//<WIFICLIENT>
bool CLIENT=1;
const PROGMEM char *ntpServer = "pool.ntp.org";
boolean syncEventTriggered = false; // NTP True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // NTP Last triggered event
//const char *ssid = "BUArealBridge";  //defined in passwords.h
//const char *password = "************";
//</WIFICLIENT>
//<WIFISERVER>
bool AP=0;
//const char* APssid="TermAP";  //defined in passwords.h
//const char* APpassword="************";
IPAddress APIP(192,168,60,1);
IPAddress APGW(192,168,60,1);
const IPAddress IPMask(255,255,255,0);
//</WIFISERVER>
//<WEBSERVER>
String tempWeb="not connected";
String tempWebFin="not connected";
String head01="<!DOCTYPE html>\n <html>\n <head>\n <meta http-equiv=\"refresh\" content=\"15\" />\n <meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\" /> \n <title>"+deviceName+"</title>\n </head>\n <body style=background-color:";
//String head01="<!DOCTYPE html>\n <html>\n <head>\n <meta http-equiv=\"refresh\" content=\"15\" />\n <title>"+deviceName+"</title>\n </head>\n <body style=background-color:";
const String tail03="</h1> \n </body>\n </html>\n";
const bool Debug=0; //for dallas addr
const bool debug=0;
//</WEBSERVER>
//</SETUP>

OneWire DallasOW(DallasPin);  //create instance Onewire
DallasTemperature Sensors(&DallasOW); //create instance Dallas (use OneWire)

AsyncWebServer server(80);
WiFiServer server1(80); 

float nactiTeplotu(int index){
  Sensors.requestTemperatures();
  return(Sensors.getTempCByIndex(index));
}

float nactiTeplotuAddr(DeviceAddress adr){
  Sensors.requestTemperatures();
  return(Sensors.getTempC(adr));
}

String searchAddr()
 {
  String addrs="<br>";
  byte addr[8];
  DallasOW.search(addr);
  for(int i=0; i<8;i++){
    addrs+=String(addr[i], HEX);
    addrs+=":";
  }
  return addrs;
 }

void blinkLed(){
  digitalWrite(LED, HIGH);
  delay(20);
  digitalWrite(LED, LOW);
  delay(50);
 }

void setBackgroundColor(float measTemp){
  if(measTemp<tempLimit) bcgcolor="red";
  else bcgcolor="green";
 }

 void setBackgroundColor(float measTemp1, float measTemp2){
  if((measTemp1<tempLimit)|| (measTemp2<tempLimit)) bcgcolor="red";
  else bcgcolor="green";
 }

 void setBackgroundColor(float measTemp1, float measTemp2, float measTemp3){
  if((measTemp1<tempLimit)|| (measTemp2<tempLimit)|| (measTemp3<tempLimit)) bcgcolor="red";
  else bcgcolor="green";
 }

 void readingTempCorrect(float tt){
  if(tt<(-120.0)){
    counter++;
    if(counter>5) ESP.restart(); 
  }
 }
 
void setup(){
if(Debug){
    AP=1; 
    CLIENT=0;
    head01="<!DOCTYPE html>\n <html>\n <head>\n \n <title>"+deviceName+"</title>\n </head>\n <body style=background-color:";
  }
if(debug) Serial.begin(115200);
pinMode(LED, OUTPUT);
pinMode(DallasPowerPin, OUTPUT); 
digitalWrite(LED, LOW);
//Dallas reset
pinMode(DallasPowerPin, OUTPUT);
pinMode(DallasPin, OUTPUT);
digitalWrite(DallasPowerPin, LOW);
digitalWrite(DallasPin, LOW);
delay(1000);
digitalWrite(DallasPowerPin, HIGH);
pinMode(DallasPin, INPUT);
//end of Dallas reset


//WiFi.mode(WIFI_AP_STA);
WiFi.mode(WIFI_STA);

if(CLIENT){
  WiFi.begin(ssid, password);
  if(debug) Serial.println("WiFi STA connecting");
    while(WiFi.status()!= WL_CONNECTED)
      {
    delay(500);
    if(debug) Serial.print(".");
    blinkLed();
    counter++;
    if(counter>50) ESP.restart(); 
      }
  counter=0;
}

if(AP){
  WiFi.softAP(APssid,APpassword);
  WiFi.softAPConfig(APIP, APGW, IPMask);
  if(debug) Serial.println(WiFi.softAPIP());
  }

digitalWrite(LED, HIGH); //IF WL CONNECTED
 if(debug) Serial.println("");
  if(debug) Serial.println("WiFi connected");
  if(debug) Serial.println("IP address: ");
  if(debug) Serial.println(WiFi.localIP());
  
Sensors.begin(); //start comm with Dallas sensor

if(WebServerOn){  
  if(debug) Serial.println("http server begin...");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html",head01+bcgcolor+">\n" +tempWebFin +tail03);
    //request->send(200, "text/plain", temp01);
  }); 
  server.begin();
}
if(debug) Serial.println("NTP begin...");
NTP.begin (ntpServer,1,true,0);

if(WebServer1On){  
  if(debug) Serial.println("http server1 begin...");
  server1.begin();
}

} //end of setup part

 
//String temp01, temp02; 
void loop(){
  if(CLIENT && WiFi.status()!= WL_CONNECTED) {
    digitalWrite(LED, LOW);
    WiFi.reconnect();
    delay(20000);
    if(WiFi.status()!= WL_CONNECTED) ESP.restart();
  }
  else digitalWrite(LED, HIGH);
  t1=nactiTeplotuAddr(internal1Adr);
  tempWeb="<h1> \n"+NTP.getTimeDateString()+" Signal: "+WiFi.RSSI()+"    Dev: "+deviceName;
  delay(10);
  if(Sensor1){
      tempWeb+="\n <br>"+Sensor1Place;
      tempWeb+="     ";
      tempWeb+=String(t1,2)+" &deg;C"; 
    }
 if(Sensor2){
    delay(10);
    t2=nactiTeplotuAddr(external2Adr);
    tempWeb+="\n <br>"+Sensor2Place;
    tempWeb+="     ";
    tempWeb+=String(t2,2)+" &deg;C";
    }
 if(Sensor3){
    delay(10);
    t3=nactiTeplotuAddr(external3Adr);
    t3-=calibration3;
    tempWeb+="\n <br>"+Sensor3Place;
    tempWeb+="     ";
    tempWeb+=String(t3,2)+" &deg;C"; 
    }
  readingTempCorrect(t1);//check if temp is in normal interval
  readingTempCorrect(t2);
  readingTempCorrect(t3); 
  setBackgroundColor(t1,t2,t3);
  if(Debug){
    DallasOW.reset_search();
    tempWeb+="</h1> \n <br>"+searchAddr()+"\n <br>"+searchAddr()+"\n <br>"+searchAddr()+"\n <br>"+searchAddr()+"\n <br>"+searchAddr();
  }
tempWebFin=tempWeb;
delay(5000);
  }
