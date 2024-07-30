#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h> 

#define RE 4
#define DE 4

SoftwareSerial mySerial(14, 2);

//bytes for retrieveing temp,hum and ec
const byte request[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x03, 0xb0, 0x0b};
byte values[11];
float postTemp = 0;
float postHum = 0;
float postEC = 0;
int flag = 0;

const char* ssid = "";
const char* pass = "";

//---- HiveMQ Cloud Broker settings
const char topic[]  = "sensor/readings";
const char* mqttServer = "";
const char* mqttUserName = "";
const char* mqttPassword = "";
int port = 8883;

BearSSL::WiFiClientSecure espClient;
PubSubClient client(espClient);

static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

unsigned long lastMsg = 0;
int value = 0;

void setup() {

  initialize();
  delay(3000);

  // Wait for serial to initialize.
  while(!Serial) { }

  BearSSL::X509List *serverTrustedCA = new BearSSL::X509List(ca_cert);
  espClient.setTrustAnchors(serverTrustedCA);
  connectToWiFi();
  setClock();
  client.setServer(mqttServer,port);
  connectMQTT();
}

void loop() 
{
  getMeasurements();
  float temp = (values[3]<<8 | values[4])/100.00;
  float hum = (values[5] | values[6])/10;
  float EC = (values[7] | values[8])/10;
  
  postTemp = temp;
  postHum = hum;
  postEC = EC;

  Serial.print(" Temperature: ");
  Serial.print(temp);
  Serial.print(" ºC ");
  delay(3000);

  Serial.print(" EC: ");
  Serial.print(EC);
  Serial.print("  ");
  delay(3000);

  Serial.print(" Humidity: ");
  Serial.print(hum);
  Serial.print(" % ");
  delay(3000);

  if(!client.connected())
  {
    connectMQTT();
  }
  
    if(flag == 0)
    {
      flag++;
    }
    else
    {
      sendMQTT();
      Serial.println("Going to sleep now");
      WiFi.forceSleepBegin();
      delay(600000); //10m delay
      WiFi.forceSleepWake();
      Serial.println("Waking up");
    }
  
}

 void initialize()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
 
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  // put RS-485 into receive mode
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
}

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("WiFi connection Successful ");

  Serial.print(WiFi.localIP());
}


void connectMQTT()
{

  while(!client.connected())
  {
    Serial.println("Attempting to MQTT Connection!");

    String clientID = "ESP8266";

    if(client.connect(clientID.c_str(), mqttUserName, mqttPassword))
    {
      Serial.println("Connected!");
    }
    else
    {
      Serial.println("Failed! Client state = ");
      Serial.print(client.state());
      Serial.println("Retring in 5 seconds!");
      delay(5000);
    }
  }

}

void sendMQTT()
{

  StaticJsonDocument<200> doc;
    doc["temperature"] = postTemp;
    doc["humidity"] = postHum;
    doc["ec"] = postEC;

    String jsonStr;
    serializeJson(doc, jsonStr);
    
  Serial.print("Sending message: ");
  Serial.println(jsonStr);
  client.publish(topic, jsonStr.c_str());
  delay(10);
}

void setClock()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");

  Serial.println(asctime(&timeinfo));
}

void getMeasurements(){
  mySerial.flush();
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  //Serial.print("Before write ");
//  Serial.print(mySerial.write(request, sizeof(request)));
   if (mySerial.write(request, sizeof(request)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i<sizeof(values); i++){
      values[i] = mySerial.read();
      //Serial.print(values[i], HEX);
      //Serial.print(" ");
     }
   }else{
    Serial.println("Fais here!");
   }
   Serial.println(" ");
  //dummy reading for cleaning
  while(mySerial.available() > 0) mySerial.read();
  mySerial.flush();
}


// void connectMQTT()
// {
//   if(!mqttClient.connect(broker,port)){
//     Serial.print("MQTT connection failed! Error code = ");

//     Serial.println(mqttClient.connectError());

//     while (1);
//   } else{
//     Serial.println("You're connected to the MQTT broker!");

//     Serial.println();
//   }
// }

// void sendMQTT()
// {
//   client.poll();
//   unsigned long currentMillis = millis();

//   if (currentMillis - previousMillis >= interval) {

//     // save the last time a message was sent
//     previousMillis = currentMillis;

//     StaticJsonDocument<200> doc;
//     doc["temperature"] = 25;
//     doc["humidity"] = 34;
//     doc["ec"] = 12;

//     String jsonStr;
//     serializeJson(doc, jsonStr);

//     Serial.print("Sending message to topic: ");
//     Serial.println(topic);
//     Serial.println(jsonStr);

//     mqttClient.beginMessage(topic);
//     mqttClient.print(jsonStr);
//     mqttClient.endMessage();
//   }
// }