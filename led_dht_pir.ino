#include <WiFi.h>
#include "DHT.h"

// Constante para definir rede a ser conectada
const char* ssid = "RouterTest";
const char* password = "routertest";

// Define um IP estático
IPAddress local_IP(192, 168, 0, 150);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiServer server(80); // Port 80

#define LED 32            // Led interruptor no pino 32
#define DHTPIN 4          // Define o pino de dados do sensor DHT11
#define DHTTYPE DHT11     // Define o tipo de sensor
DHT dht(DHTPIN, DHTTYPE); // Habilita o sensor
#define LEDPIR 26         // Led do sensor de presença
#define DATAPIR 27        // Define o pino de dados do sensor PIR

int presenca = 0;
String estado = "";
int wait30 = 30000; // timer usado em caso de desconexões.

void setup() {
  Serial.begin(9600);

  // Define os modos dos pinos utlizados
  pinMode(LED, OUTPUT);
  pinMode(LEDPIR, OUTPUT);
  pinMode(DATAPIR, INPUT);

  // Inicializa o sensor DHT
  dht.begin();

  // Configura o IP estático
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Erro em configurar a rede.");
  }

  // Conecta a rede WI-FI
  Serial.println();
  Serial.print("Connecting with ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected with WiFi.");

  // Inicia o servidor web
  server.begin();
  Serial.println("Web Server started.");

  // Mostra o IP do microcontrolador na serial
  Serial.print("This is IP to connect to the WebServer: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  
  presenca = digitalRead(DATAPIR); // Le o valor do sensor PIR
  if (presenca == LOW)  // Sem movimento, mantem desligado o led
  {
    digitalWrite(LEDPIR, LOW);
  } else  // Caso seja detectado um movimento liga o led
  {
    digitalWrite(LEDPIR, HIGH);
  }

  // Se desconectado da rede, tenta reconectar a cada 30 segundos
  if ((WiFi.status() != WL_CONNECTED) && (millis() > wait30)) {
    Serial.println("Tentando reconectar a rede Wi-Fi...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    wait30 = millis() + 30000;
  }
  // Confere se o cliente se conectou
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.print("Novo Cliente: ");
  Serial.println(client.remoteIP());

  // Espera que o cliente envie alguns dados
  while (!client.available()) {
    delay(1);
  }

  // Le as informações enviadas pelo cliente.
  String req = client.readStringUntil('\r');
  Serial.println(req);

  // Leitura dos dados do sensor DHT
  // Humidade (%)
  float h = dht.readHumidity();
  String humi = String(h, 2);
  // Temperatura (Celsius)
  float t = dht.readTemperature();
  String temp = String(t, 2);

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Falha ao capturar dados do sensor DHT!"));
    return;
  }
  // Make the client's request.
  if (req.indexOf("ledon") != -1) {
    digitalWrite(LED, HIGH);
    estado = "LED Ligado";
  }
  if (req.indexOf("ledoff") != -1) {
    digitalWrite(LED, LOW);
    estado = "LED desligado";
  }


  //////////////////////////////////////////////
  // Página WEB. ////////////////////////////
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");

  client.print(estado);
  client.print(",");
  client.print(temp);
  client.print(",");
  client.print(humi);
  client.print(",");
  client.print(presenca);

  client.stop();
  delay(1);
}
