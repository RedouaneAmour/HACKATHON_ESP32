/**
   PostHTTPClient.ino

    Created on: 21.11.2016

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <string>
#include <stdio.h>
#include<stdlib.h>
using namespace std;

/* this can be run with an emulated server on host:
        cd esp8266-core-root-dir
        cd tests/host
        make ../../libraries/ESP8266WebServer/examples/PostServer/PostServer
        bin/PostServer/PostServer
   then put your PC's IP address in SERVER_IP below, port 9080 (instead of default 80):
*/
//#define SERVER_IP "10.0.1.7:9080" // PC address with emulation on host
#define SERVER_IP "172.20.10.2:8282"

#ifndef STASSID
#define STASSID "iPhone de Chadi"
#define STAPSK  "ChadiA98"
#endif

int pin_boutton = D7 ;
int pin_sensor = A0 ;
const int B=4275;                 // B value of the thermistor
const int R0 = 100000;            // R0 = 100k

void setup() {

  Serial.begin(115200);
  pinMode(pin_boutton, INPUT) ;
  pinMode(pin_sensor,INPUT) ;

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {

// acquisition de la température 
int a = analogRead(pin_sensor );
float R = 1023.0/((float)a)-1.0;
R = 100000.0*R;
float temperature = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;//convert to temperature via datasheet ;
//float temperature_numerique = (1023/((analogRead(pin_sensor)) - 1.0));
//float temperature_en_celsius= (float) (1.0/(log(100000*temperature_numerique/100000)/4275+1/298.15)-273.15); // convert to temperature via datasheet
Serial.print("temperature = ");
Serial.print(temperature);
Serial.println("°C") ;
String cstr = String(temperature,2) ;

if (temperature > 34.0) {


// wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://" SERVER_IP "/~/mn-cse/mn-name/MY_SENSOR_ESP8266/DATA"); //HTTP
    http.addHeader("X-M2M-Origin","admin:admin");
    http.addHeader("Content-Type", "application/xml;ty=4");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST(String("<m2m:cin xmlns:m2m=\"http://www.onem2m.org/xml/protocols\"> <cnf>message</cnf> <con> &lt;obj&gt; &lt;str name=&quot;appId&quot; val=&quot;MY_SENSOR_ESP8266&quot;/&gt;  &lt;str name=&quot;category&quot; val=&quot;temperature&quot;/&gt;  &lt;int name=&quot;data&quot; val=&quot;") + cstr + String("&quot;/&gt;  &lt;int name=&quot;unit&quot; val=&quot;celsius&quot;/&gt;  &lt;/obj&gt;  </con> </m2m:cin>"));
  
    
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);
}
else Serial.println("tout va bien");
delay(5000);
}
