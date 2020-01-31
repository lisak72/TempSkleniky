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
//20190408001
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include <TimeLib.h> //TimeLib library is needed https://github.com/PaulStoffregen/Time
#include <NtpClientLib.h> //Include NtpClient library header  https://github.com/gmag11/NtpClient

//<SETUP>
const byte DallasPin=19;
const byte LED=13; 
const bool WebServerOn=1;
const bool WebServer1On=0;
const String deviceName="01_Skleniky leva strana (zapadni)";
//<SENSOR1>
const bool Sensor1=1;
DeviceAddress internal1Adr={0x28,0xff,0xe,0xf8,0x40,0x18,0x3,0xce};
const String Sensor1Place="Teplota chodba leva: ";
//</SENSOR1>
//<SENSOR2>
const bool Sensor2=1;
DeviceAddress external2Adr={0x28,0xff,0x13,0xfa,0x40,0x18,0x3,0x59};
const String Sensor2Place="Teplota koje zimovani (sever): ";
//</SENSOR2>
//<SENSOR3>
const bool Sensor3=1;
DeviceAddress external3Adr={0x28,0xff,0xf2,0xf3,0x40,0x18,0x3,0xdd};
const String Sensor3Place="<br>Teplota venkovni sever: ";
const float calibration3=-0.31;
//</SENSOR3>
//<SENSOR4>
const bool Sensor4=1;
DeviceAddress external4Adr={0x28,0xaa,0xaf,0xcb,0x1d,0x13,0x2,0x23};
const String Sensor4Place="Teplota koje krajni jih: ";
const float calibration4=0.0;
//</SENSOR4>
int counter;
const float tempLimit=5.0;
float t1=-99.0, t2=-99.0,t3=-99.0,t4=-99.0;
String bcgcolor="red";
//<WIFICLIENT>
const bool CLIENT=1;
const char *ssid = "BUArealBridge";
const char *password = "************";
//</WIFICLIENT>
//<WIFISERVER>
const bool AP=0;
const char* APssid="TermAP";
const char* APpassword="************";
IPAddress APIP(192,168,60,1);
IPAddress APGW(192,168,60,1);
const IPAddress IPMask(255,255,255,0);
//</WIFISERVER>
//<WEBSERVER>
String tempWeb="not connected";
String tempWebFin="not connected";
String head01="<!DOCTYPE html>\n <html>\n <head>\n <meta http-equiv=\"refresh\" content=\"15\" />\n <meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\" /> \n <title>"+deviceName+"</title>\n </head>\n <body style=background-color:";
const String tail03="</h1> \n </body>\n </html>\n";
//</WEBSERVER>
const bool Debug=0;
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
Serial.begin(115200);
pinMode(LED, OUTPUT);
delay(1000); 
digitalWrite(LED, LOW);
NTP.begin ("147.231.248.1", 1, true);

//WiFi.mode(WIFI_AP_STA);
WiFi.mode(WIFI_STA);

if(CLIENT){
  WiFi.begin(ssid, password);
    while(WiFi.status()!= WL_CONNECTED)
      {
    delay(500);
    Serial.print(".");
    blinkLed();
    counter++;
    if(counter>50) ESP.restart(); 
      }
  counter=0;
}

if(AP){
  WiFi.softAP(APssid,APpassword);
  WiFi.softAPConfig(APIP, APGW, IPMask);
  Serial.println(WiFi.softAPIP());
  }

digitalWrite(LED, HIGH); //IF WL CONNECTED
 Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
Sensors.begin(); //start comm with Dallas sensor

if(WebServerOn){  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html",head01+bcgcolor+">\n" +tempWebFin +tail03);
    //request->send(200, "text/plain", temp01);
  }); 
  server.begin();
}

if(WebServer1On){  
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
  tempWeb="<h1> \n"+NTP.getTimeDateString()+" Signal: "+WiFi.RSSI()+"    Dev: "+deviceName;
  delay(10);
  if(Sensor1){
      t1=nactiTeplotuAddr(internal1Adr);
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
  if(Sensor4){
    delay(10);
    t4=nactiTeplotuAddr(external4Adr);
    t4+=calibration4;
    tempWeb+="\n <br>"+Sensor4Place;
    tempWeb+="     ";
    tempWeb+=String(t4,2)+" &deg;C"; 
    }
  if(Sensor3){
    delay(10);
    t3=nactiTeplotuAddr(external3Adr);
    t3+=calibration3;
    tempWeb+="\n <br>"+Sensor3Place;
    tempWeb+="     ";
    tempWeb+=String(t3,2)+" &deg;C"; 
    }

  readingTempCorrect(t1);
  readingTempCorrect(t2);
  readingTempCorrect(t3);
  readingTempCorrect(t4);
  
  setBackgroundColor(t1,t2,t4);
  if(Debug){
    DallasOW.reset_search();
    tempWeb+="</h1> \n <br>"+searchAddr()+"\n <br>"+searchAddr()+"\n <br>"+searchAddr()+"\n <br>"+searchAddr()+"\n <br>"+searchAddr();
  }
tempWebFin=tempWeb;
delay(5000);
  }
