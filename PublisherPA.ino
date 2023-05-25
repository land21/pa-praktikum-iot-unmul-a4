#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Mengatur SSID dan Password Jaringan yang di pakai
const char* ssid = "MITHA";
const char* password = "mithew13";

// server Mqtt broker
const char* mqtt_server = "broker.hivemq.com";


#define FS_PIN D1 // Sensor Api
#define DHTPIN D4 // Sensor Suhu
#define DHTTYPE DHT11 //Mengatur TYPE DHT (Karena ada 2 jenis yaitu DHT11 & DHT22)
#define TIMEDHT 1000 

unsigned long timerDHT = 0;

DHT dht(DHTPIN, DHTTYPE); 

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int FS = 0;
int FSold = 0;
int value = 0;
float h;
float t;

// Fungsi koneksi wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.println("Mencoba Menghubungkan ke ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  randomSeed(micros());

  Serial.println("WiFi Terhubung");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// fungsi untuk menghubungkan ke broker
void reconnect() {
  // Loop sampai terkoneksi dengan broker
  while (!client.connected()) {
    Serial.println("Mencoba untuk menghubungkan dengan MQTT...");
    // membuat random client ID
    String clientId = "ESP8266Client- ";
    clientId += String(random(0xffff), HEX);
    // mencoba untuk connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung");
      client.subscribe("iotunmulunik");
    } else {
      Serial.println("Gagal Terhubung");
      Serial.println("Mencoba dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(FS_PIN,INPUT); // inisialisasi Sensor Api sebagai input
  Serial.begin(115200);
  dht.begin(); // memulai dht
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) { // jika tidak terkoneksi dengan MQTT maka akan melakukan fungsi reconnect
    reconnect();
  }
  client.loop();

   if ((millis() - timerDHT) > TIMEDHT) { // jika detik di kurangi timer lebih dari TIMEDHT
    // Update pada timer menggunakan millis
    timerDHT = millis();
    // Membaca kelembapan
    h = dht.readHumidity();
    // Membaca Suhu
    t = dht.readTemperature();
    
    float f = dht.readTemperature(false);

    // mengecek dht jika gagal dalam membaca suhu dan kelembapan
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("DHT sensor gagal membaca!"));
      return;
    }

  }
  
  // membuat nilai inputan DHT menjadi integer pada kelembapan dan Suhu
  int hum = (int)h;
  int temp = (int)t;

  Serial.print("Suhu : ");
  Serial.println(temp);
  Serial.print("Kelembaban : ");
  Serial.println(hum);

  delay(200);
  snprintf (msg, MSG_BUFFER_SIZE, "%s", itoa(temp,msg,10));
  client.publish("iot_unmul/iot_a_4/Suhu", msg); // publish data Suhu
  snprintf (msg, MSG_BUFFER_SIZE, "%s", itoa(hum,msg,10));
  client.publish("iot_unmul/iot_a_4/Kelembapan", msg); // publish data Kelembapan

  int FS = digitalRead(FS_PIN); // membaca sensor Api
  // jika nilai sensor api masih sama maka tidak akan ada publish
  if (FS == FSold){ 
    FS = FSold;
  } else {
    FSold = FS;
    delay(2000);
    snprintf (msg, MSG_BUFFER_SIZE, "%s", itoa(FS,msg,10));
    Serial.print("Api : ");
    Serial.println(FS);
    client.publish("iot_unmul/iot_a_4/Api", msg); } // publish data Api
}