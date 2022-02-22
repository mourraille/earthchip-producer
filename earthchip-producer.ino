#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#include <Adafruit_BMP085_U.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

#define wifi_ssid "ssid"
#define wifi_password "xxxxxxxxxxxx"

#define mqtt_port 1883
#define mqtt_server "xx.xx.xx.xx"

#define SECONDS 1000
#define MINUTES 1000 * 60


WiFiClient espClient;
PubSubClient client(espClient);
int currentMP = 0;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Earthchip-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("INIT", "HELLOWORLD");
      // ... and resubscribe
      // client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(D5, OUTPUT);
  digitalWrite(D5, HIGH);
  Serial.begin(115200);
  if (!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  double soilmoisturepercent = map(analogRead(A0), 874, 450, 0, 100);
  char arr[5];
  dtostrf(soilmoisturepercent, 4, 3, arr);
  client.publish("SOIL", arr);
  float temperature;
  bmp.getTemperature(&temperature);
  temperature = temperature - 1;
  char strtemp[10];
  dtostrf(temperature, 4, 2, strtemp);
  client.publish("TEMP", strtemp);
  delay(2 * SECONDS);
  Serial.println(soilmoisturepercent);
}
