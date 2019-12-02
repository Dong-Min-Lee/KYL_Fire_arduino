#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <FirebaseArduino.h>
 
#define PIN_DHT 4
#define FIREBASE_HOST "ifs-systems-32945.firebaseio.com"
#define FIREBASE_AUTH "wfsjgFbLS1s1ENjZGFLpOHDhkNTwThJGS5xeK5hx"

 
const char* ssid = "JU_C";
const char* password = "gojangan2011";
String host = "http://192.168.27.214:8080";
 
const long interval = 5000;
unsigned long previousMillis = 0;

const long finterval = 1000;
unsigned long fpreviousMillis = 0;
int cnt = 0;

WiFiServer server(80);
WiFiClient client;
HTTPClient http;
DHT DHTsensor(PIN_DHT, DHT11);
 
void setup() {
  DHTsensor.begin();
  
  Serial.begin(115200);

  pinMode(5,INPUT);
  pinMode(A0,INPUT);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server started");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
 
void loop() {

    String fired = "SR2";
    float humidity = DHTsensor.readHumidity();
    float temp = DHTsensor.readTemperature();
    int gas = analogRead(A0);
    int flame = digitalRead(5);
    int fire =0;
    if(gas>=10 and flame ==0 and temp >=20){
      fire = 1;
     
  
    }
    else {
      fire = 0;

    }


  // 센서값 DB 전송 부분

  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

  
    String phpHost = host+"/ifs/insert?fired="+ fired +"&temp="+String(temp)+"&gas="+String(gas)+"&flame="+String(flame)+"&fire="+String(fire);
    Serial.print("Connect to ");
    Serial.println(phpHost);
    
    http.begin(client, phpHost);
    http.setTimeout(1000);
    int httpCode = http.GET();
    
   Serial.println(httpCode);
    if(httpCode > 0) {
      Serial.printf("GET code : %d\n\n", httpCode);
    } 
    else {
      Serial.printf("GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();



    // 푸시알림전송 부분    
    if(cnt==0){
      if(fire ==1){
        
         String msgHost = host+"/ifs/pushmsg.do?fired=" + fired + "&fire="+String(fire);
         Serial.print("Connect to ");
         Serial.println(msgHost);

          http.begin(client,msgHost);
          http.setTimeout(1000);
         int msgCode = http.GET();
        if(msgCode > 0) {
         Serial.printf("GET code : %d\n\n", msgCode);
      } 
      else {
        Serial.printf("GET failed, error: %s\n", http.errorToString(msgCode).c_str());
      }
    cnt++;
    http.end();
    }
   
    } if(fire==0){
      cnt=0;

    }
}

if(currentMillis - fpreviousMillis >= finterval) {
    fpreviousMillis = currentMillis;


      //파이어베이스 전송 부분
  if(fire == 0){
  Firebase.setString("icom2f/SR2/fire", "0");
  Serial.println(fire);
  Serial.println("센서변경성공");
  }

  if(fire == 1){
  Firebase.setString("icom2f/SR2/fire", "1");
  Serial.println(fire);
  Serial.println("센서변경성공2");
  }
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
    }

    
  
}
