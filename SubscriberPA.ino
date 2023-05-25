#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Mengatur SSID dan Password Jaringan yang di pakai
const char* ssid = "Universitas Mulawarman";
const char* password = ""; 

// server Mqtt broker
const char* mqtt_server = "broker.hivemq.com";

// Buzzer & LED pin
#define BUZZER_PIN D5
#define LED_PIN D1

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int Hum;
int Temp;
int Api;
int Max = 30;


// Fungsi koneksi wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Mencoba Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("WiFi Terhubung");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menerima data
void callback(char* topic, byte* payload, unsigned int length) {

   if(strcmp(topic, "iot_unmul/iot_a_4/Kelembapan") == 0)// jika topic yang di terima adalah kelembapan
   {
        String hum = ""; 
        for (int i = 0; i < length; i++) {
          hum += (char)payload[i];
        }
        Hum  = hum.toInt(); // nilai kelembapan akan di tempatkan pada variabel Hum
        Serial.print("Kelembapan [");
        Serial.print(Hum);
        Serial.println("] ");
   }

   else if(strcmp(topic, "iot_unmul/iot_a_4/Suhu") == 0) // jika topic yang di terima adalah Suhu
   {
        
        String temp = ""; 
        for (int i = 0; i < length; i++) {
          temp += (char)payload[i];
        }
        Temp  = temp.toInt(); // nilai Suhu akan di tempatkan pada variabel Temp
        Serial.print("Suhu [");
        Serial.print(Temp);
        Serial.println("] ");
   }
   
   else if(strcmp(topic, "iot_unmul/iot_a_4/Api") == 0) // jika topic yang di terima adalah Api
   {
        String fire = ""; 
        for (int i = 0; i < length; i++) {
          fire += (char)payload[i];
        }
        Api = fire.toInt(); // nilai keadaan Api akan di tempatkan pada variabel Api
        Serial.print("Api [");
        Serial.print(Api);
        Serial.println("] ");
   }
   
   else if(strcmp(topic, "iot_unmul/iot_a_4/SuhuMax") == 0) // jika topic yang di terima adalah menentukan Maksimal Suhu 
   {
        String maks = ""; 
        for (int i = 0; i < length; i++) {
          maks += (char)payload[i];
        }
        Max  = maks.toInt(); // nilai maksimal Suhu akan di tempatkan pada variabel Max
   }
    
}

// fungsi untuk mengubungkan ke broker
void reconnect() {
  // Loop sampai terkoneksi dengan broker
  while (!client.connected()) {
    Serial.println("Mencoba untuk menghubungkan dengan MQTT...");
    // membuat random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX); 
    // mencoba untuk connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung");
      client.subscribe("iot_unmul/iot_a_4/#");
    } else {
      Serial.println("Gagal Terhubung ");
      Serial.println("Mencoba dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);  // Inisialisasi pin BUZZER
  pinMode(LED_PIN, OUTPUT);     // Inisialisasi pin LED
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) { // jika tidak terkoneksi dengan MQTT maka akan melakukan fungsi reconnect
    reconnect();
  }
  client.loop();
  
  if (Temp > Max) // jika temperatur lebih dari nilai maksimal
    {
      // melakukan blink pada LED dengan delay 500
      digitalWrite(LED_PIN, HIGH); 
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    }
    else if (Temp <= Max) // jika temperatur kurang dari sama dengan maksimal
    {
      digitalWrite(LED_PIN, LOW);  // LED akan mati
    }
  
  if (Api == 1) // jika api terdeteksi
    {
      digitalWrite(LED_PIN, HIGH); // LED akan menyala
      tone(BUZZER_PIN,1000); // Buzzer akan berbunyi dengan frekuensi 1000
    }
  else if (Api == 0) // jika api tidak terdeteksi
    {
      digitalWrite(LED_PIN, LOW); // LED Mati
      noTone(BUZZER_PIN); // Buzzer mati
    }
}