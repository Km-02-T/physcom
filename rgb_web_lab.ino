#include <WiFiS3.h>
#include <PubSubClient.h>

// 1. ตั้งค่า Wi-Fi ของคุณ
const char* ssid = "wifi_name";
const char* password = "password";

// 2. ตั้งค่า MQTT Broker
const char* mqtt_server = "broker.hivemq.com"; 
const int mqtt_port = 1883; 
const char* topic = "physcom/68070000";

// 3. กำหนดขา PWM สำหรับหลอด RGB
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp = "";
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageTemp);

  int firstComma = messageTemp.indexOf(',');
  int secondComma = messageTemp.indexOf(',', firstComma + 1);

  if (firstComma != -1 && secondComma != -1) {
    int r = messageTemp.substring(0, firstComma).toInt();
    int g = messageTemp.substring(firstComma + 1, secondComma).toInt();
    int b = messageTemp.substring(secondComma + 1).toInt();

    Serial.print("Target RGB -> Red: "); Serial.print(r);
    Serial.print(" | Green: "); Serial.print(g);
    Serial.print(" | Blue: "); Serial.println(b);

    // ----------------------------------------------------
    // จุดสำคัญสำหรับ Common Anode: ต้องเอา 255 ไปลบค่าที่รับมา
    // ----------------------------------------------------
    analogWrite(redPin, 255 - r);
    analogWrite(greenPin, 255 - g);
    analogWrite(bluePin, 255 - b);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ArduinoUnoR4-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // อย่าลืมตั้ง Baud Rate ใน Serial Monitor ให้เป็น 115200 เพื่อดูข้อความ
  Serial.begin(115200);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  // ปิดไฟตอนเริ่มต้น สำหรับ Common Anode ค่า 255 คือ ปิดไฟ (OFF)
  analogWrite(redPin, 255);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 255);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}