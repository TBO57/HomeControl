/* ------------------------------------------------ Librerías ------------------------------------------------ */

#include <WiFi.h>                     // Conexión red WiFi
#include <Preferences.h>              // Almacenar varibles en memoria flash
#include <ESPmDNS.h>                  // Agregar una mDNS a la URL del proycto "http//HomeControl.local"
#include <SPIFFS.h>                   // Almacenar archivos memoria flash ESP32
#include <HTTPClient.h>               // Envio de peticiones tipo POST

/* Librerías WebServer */
#include <AsyncTCP.h>                 // Funciones Assincronas para el WebServer
#include <ESPAsyncWebServer.h>        // Creación de un WebServer
#include <WebSocketsServer.h>         // Funciones de WebSockets para envio y recepcion de datos
#include <ArduinoJson.h>              // Uso de datos tipo JSON

/* Librerías IR */
#include <Arduino.h>
#include <IRsend.h>                   // Decodificación y envio de señales IR
#include <IRremoteESP8266.h>          // Uso de tecninologias IR con ESP32 y ESP8266

#include <DHT.h>                      // Sensor DHT11


/* ------------------------------------------------ Constantes y Variables ------------------------------------------------ */

const char* ssid = "JMROSALIN";       // Nombre de la Red Wifi
const char* password = "Camlu1831";   // Contraseña de la Red Wifi

const int MAX_CLIENTS = 5;            // Número máximo de clientes WebSocket
IPAddress connectedIP[MAX_CLIENTS];   // Variable global para guardar IPs conectadas
bool isConnected[MAX_CLIENTS];        // Variable global para saber si un cliente está conectado


/* ------------------------------------------------ API Google APP Scripts para envio de datos a Google Sheets ------------------------------------------------ */
const char* scriptUrl = "https://script.google.com/macros/s/AKfycbz_I72eyparILFbkHZCf44kVZIQTuI88gF1SDMuLi7a3FWfhaks-x5FFra2MkgUTtzHTA/exec";
/* ------------------------------------------------ API Google APP Scripts para envio de datos a Google Sheetss ------------------------------------------------ */


unsigned long anteriorMillis = 0;     // Tiempo de envio de lectura de temperatura
const long intervalo = 2500;          // Intervalo de 2 segundos y medio

bool aireEncendido = false;

int setTempOn = 34;                   // Temperatura Default de Encendido del AC
int setTempOff = 27;                  // Temperatura Default de Apagado del AC

Preferences preferences;


/* ------------------------------------------------ Pins / Inicializaciones ------------------------------------------------ */

#define IR_LED_PIN 4                  // Pin Led Emisor IR
IRsend irSender(IR_LED_PIN);

#define relayPin 14                   // Pin del Relay

#define FlagGPin 26                   // Bandera de servidor encendido

#define DHTPIN 27                     // Pin del sensor Temperatura
#define DHTTYPE DHT11                 // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);
WebSocketsServer webSocket(81);


/*--------------------------------------------- Codigos del Aire acondicionado ------------------------------------------*/
uint16_t POWER_ON[299] = {4382, 4404,  526, 1648,  502, 552,  524, 1648,  502, 1646,  504, 550,  524, 550,  526, 1646,  504, 550,  524, 550,  524, 1646,  506, 548,  528, 550,  524, 1644,  504, 1648,  502, 550,  526, 1646,  506, 550,  524, 548,  526, 1646,  504, 1646,  504, 1654,  500, 1644,  506, 1648,  502, 1648,  506, 1644,  504, 1648,  504, 550,  524, 548,  528, 550,  524, 550,  526, 550,  526, 548,  526, 548,  528, 548,  526, 548,  528, 548,  526, 548,  524, 550,  528, 550,  524, 548,  526, 1646,  508, 1644,  506, 1644,  506, 1646,  506, 1644,  504, 1646,  504, 1644,  508, 1644,  506, 5218,  4384, 4406,  526, 1642,  508, 570,  508, 1642,  508, 1644,  506, 568,  504, 572,  508, 1640,  508, 570,  508, 568,  532, 1616,  506, 572,  506, 568,  508, 1644,  502, 1626,  526, 572,  506, 1622,  528, 568,  508, 566,  508, 1622,  534, 1614,  536, 1616,  530, 1620,  532, 1618,  534, 1616,  532, 1620,  528, 1622,  530, 546,  524, 552,  524, 548,  524, 550,  526, 550,  524, 550,  524, 550,  526, 550,  524, 552,  524, 552,  522, 554,  522, 552,  524, 552,  520, 552,  526, 1644,  506, 1644,  506, 1648,  502, 1646,  506, 1646,  504, 1646,  504, 1644,  506, 1646,  504, 5242,  4360, 4430,  500, 1652,  500, 1648,  502, 576,  500, 1652,  498, 574,  498, 1652,  498, 576,  498, 1654,  496, 576,  496, 1656,  494, 1656,  496, 580,  494, 580,  496, 1680,  444, 630,  468, 606,  466, 610,  466, 610,  464, 608,  466, 610,  464, 608,  466, 608,  466, 608,  466, 608,  466, 608,  468, 608,  466, 610,  466, 1684,  464, 610,  466, 608,  470, 606,  466, 610,  464, 608,  468, 608,  468, 608,  464, 608,  466, 608,  466, 608,  466, 612,  464, 610,  464, 612,  464, 1686,  462, 612,  462, 612,  464, 1686,  464, 610,  466, 612,  462, 1712,  440};
#define POWER_OFF 0xB27BE0                                          // Código para apagar el AC
/*--------------------------------------------- Codigos del Aire acondicionado ------------------------------------------*/


void setup() {

  irSender.begin();                   // Comienza la comunicación con el emisor IR
  dht.begin();                        // Comienza la comunicación con el sensor DHT11
  Serial.begin(115200, SERIAL_8N1);

  pinMode(FlagGPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);        // Inicialmente apagar el relé

  /* ------------------------------------------------ Inicializar SPIFFS ------------------------------------------------ */

  if (!SPIFFS.begin(true)) {
    Serial.println("Error al inicializar SPIFFS");
    return;
  }


  /* ------------------------------------------------ Conexión a la red Wi-Fi ------------------------------------------------ */

  Serial.print("Conectando a la red WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("Conexión establecida WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());


  /* ------------------------------------------------ Inicializar mDNS ------------------------------------------------ */

  if (!MDNS.begin("homecontrol")) {             // Nombre de la URL
    Serial.println("Error al iniciar mDNS");
    return;
  }
  MDNS.addService("http", "tcp", 80);


  /* ------------------------------------------------ Inicializar NVS ------------------------------------------------ */

  preferences.begin("my-app", false);

  setTempOn = preferences.getInt("setTempOn", 34);          // 34 es el valor predeterminado si no se encuentra
  setTempOff = preferences.getInt("setTempOff", 27);        // 27 es el valor predeterminado si no se encuentra


  Serial.printf("Temperatura de encendido cargada: %d\n", setTempOn);
  Serial.printf("Temperatura de apagado cargada: %d\n", setTempOff);


  /* ------------------------------------------------ Ruta para la raiz / Pagina Web ------------------------------------------------ */

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });


  /* ------------------------------------------------ Inicnio del servidor ------------------------------------------------ */

  server.begin();

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(FlagGPin, HIGH);
  }

  webSocket.begin();                                  // WebSocket event handler
  webSocket.onEvent(webSocketEvent);                  // WebSocket event handler
}


void loop() {

  unsigned long actualMillis = millis();

  if (actualMillis - anteriorMillis >= intervalo) {
    anteriorMillis = actualMillis;

    float humedad = dht.readHumidity();               // leer la temperatura
    float temperatura = dht.readTemperature();        // leer la humedad

    webSocket.broadcastTXT("hum-" + String(humedad));
    webSocket.broadcastTXT("tmp-" + String(temperatura));

    // Verificar si la temperatura es mayor o igual a TEMPERATURA DE ENCENDIDO = encender el aire acondicionado
    if (temperatura >= setTempOn && !aireEncendido) {
      for (int i = 0; i < 2; i++) {
        irSender.sendRaw(POWER_ON, 299, 38);
        delay(350); // Esperar un poco entre cada repetición
      }
      Serial.println("Encendiendo el aire acondicionado...");
      aireEncendido = true;
    }

    // Verificar si la temperatura es menor TEMPERATURA DE APAGADO = apagar el aire acondicionado
    if (temperatura < setTempOff) {
      for (int i = 0; i < 2; i++) {
        irSender.sendCOOLIX(POWER_OFF, 24);
        delay(350); // Esperar un poco entre cada repetición
      }
      Serial.println("Apagando el aire acondicionado...");
      aireEncendido = false;
    }

  }

  webSocket.loop();                                   // Mantener la comunicación del WebSocket activa
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  IPAddress ip = webSocket.remoteIP(num);                                                                   // Obtención de IP
  String ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);           // Guardado de IP en variable String

  switch (type) {
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        String ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
        connectedIP[num] = ip;                                                                              // Guardado de IP de usario actual conectado
        isConnected[num] = true;                                                                            // Guardado de estado de IP a conexión

        /* ------------------------------------------------ Envio de datos de CONEXIÓN por medio de formato POST ------------------------------------------------ */
        Serial.printf("[%u] Conectado: %s\n", num, ipStr.c_str());
        String dataConexion = "{\"type\":\"conexion\",\"IP\":\"" + ipStr + "\",\"Estado\":\"Conectado\"}";  // Archivo JSON con información de usuario CONECTADO

        HTTPClient http;
        http.begin(scriptUrl);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(dataConexion);
        if (httpResponseCode > 0) {
          Serial.printf("Datos de conexión enviados al servidor, código de respuesta: %d\n", httpResponseCode);
        } else {
          Serial.printf("Error al enviar datos de conexión al servidor, código de respuesta: %d\n", httpResponseCode);
        }
        http.end();
        /* ------------------------------------------------ Envio de datos de CONEXIÓN por medio de formato POST ------------------------------------------------ */

        webSocket.broadcastTXT("ph1-" + String(setTempOn));       // Actualización del Valor de ENCENDIDO en el Placeholder
        webSocket.broadcastTXT("ph2-" + String(setTempOff));      // Actualización del Valor de APAGADO en el Placeholder

        break;
      }

    case WStype_DISCONNECTED: {
        IPAddress ip = connectedIP[num];;                                                                           // Revision de IP de usuario que se desconeto
        String ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
          isConnected[num] = false;                                                                                   // Guardado de estado de IP a desconexión

        /* ------------------------------------------------ Envio de datos de DESCONEXIÓN por medio de formato POST ------------------------------------------------ */
        Serial.printf("[%u] Desconectado: %s\n", num, ipStr.c_str());
        String dataDesconexion = "{\"type\":\"conexion\",\"IP\":\"" + ipStr + "\",\"Estado\":\"Desconectado\"}";    // Archivo JSON con información de usuario DESCONECTADO

        HTTPClient http;
        http.begin(scriptUrl);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(dataDesconexion);
        if (httpResponseCode > 0) {
          Serial.printf("Datos de desconexión enviados al servidor, código de respuesta: %d\n", httpResponseCode);
        } else {
          Serial.printf("Error al enviar datos de desconexión al servidor, código de respuesta: %d\n", httpResponseCode);
        }
        http.end();
        /* ------------------------------------------------ Envio de datos de DESCONEXIÓN por medio de formato POST ------------------------------------------------ */
        break;
      }

    case WStype_TEXT: {
        Serial.printf("[%u] Mensaje recibido: %s\n", num, payload);
        String msg = String((char *)payload);

        /* ------------------------------------------------ Envio de datos de ENCENDIDO del Foco por medio de formato POST ------------------------------------------------ */
        if (msg.equalsIgnoreCase("ledon")) {                                                                        // Se recibe mensaje desde el WebServer para ENCENDER el Foco
          digitalWrite(relayPin, LOW);                                                                              // Se ENCIENDE el Foco


          String dataFocoOn = "{\"type\":\"foco\",\"IP\":\"" + ipStr + "\",\"Estado\":\"Encendido\"}";              // Archivo JSON con información de Foco ENCENDIDO

          HTTPClient http;
          http.begin(scriptUrl);
          http.addHeader("Content-Type", "application/json");
          int httpResponseCode = http.POST(dataFocoOn);
          if (httpResponseCode > 0) {
            Serial.printf("Datos de encendido de foco enviados al servidor, código de respuesta: %d\n", httpResponseCode);
          } else {
            Serial.printf("Error al enviar datos de encendido de foco al servidor, código de respuesta: %d\n", httpResponseCode);
          }
          http.end();
          /* ------------------------------------------------ Envio de datos de ENCENDIDO del Foco por medio de formato POST ------------------------------------------------ */

          /* ------------------------------------------------ Envio de datos de APAGADO del Foco por medio de formato POST ------------------------------------------------ */
        } else if (msg.equalsIgnoreCase("ledoff")) {                                                                // Se recibe mensaje desde el WebServer para APAGAR el Foco
          digitalWrite(relayPin, HIGH);                                                                             // Se APAGA el Foco


          String dataFocoOff = "{\"type\":\"foco\",\"IP\":\"" + ipStr + "\",\"Estado\":\"Apagado\"}";               // Archivo JSON con información de Foco APAGADO

          HTTPClient http;
          http.begin(scriptUrl);
          http.addHeader("Content-Type", "application/json");
          int httpResponseCode = http.POST(dataFocoOff);
          if (httpResponseCode > 0) {
            Serial.printf("Datos de apagado de foco enviados al servidor, código de respuesta: %d\n", httpResponseCode);
          } else {
            Serial.printf("Error al enviar datos de apagado de foco al servidor, código de respuesta: %d\n", httpResponseCode);
          }
          http.end();
          /* ------------------------------------------------ Envio de datos de APAGADO del Foco por medio de formato POST ------------------------------------------------ */


          /* ------------------------------------------------ Envio de datos de CONFIGURACIÓN del AC por medio de formato POST ------------------------------------------------ */
        } else {
          StaticJsonDocument<200> doc;
          DeserializationError error = deserializeJson(doc, msg);
          if (!error) {
            if (doc["type"] == "set_temperatures") {                                // Se recibe un archivo JSON  desde el WebServer para cambiar ENCENDIDO/APAGADO del AC
              setTempOn = doc["valueOn"];                                           // Valor de ENCENDIDO
              webSocket.broadcastTXT("ph1-" + String(setTempOn));                   // Actualización del Valor de ENCENDIDO en el Placeholder
              setTempOff = doc["valueOff"];                                         // Valor de APAGADO
              webSocket.broadcastTXT("ph2-" + String(setTempOff));                  // Actualización del Valor de APAGADO en el Placeholder

              Serial.printf("Nueva temperatura de encendido: %d\n", setTempOn);
              Serial.printf("Nueva temperatura de apagado: %d\n", setTempOff);

              Serial.println(" ");
              irSender.sendCOOLIX(POWER_OFF, 24);
              Serial.println("Apagando el aire acondicionado...");
              Serial.println(" ");

              aireEncendido = false;

              preferences.putInt("setTempOn", setTempOn);                           // Guardar las nuevas temperaturas de ENCENDIDO en NVS
              preferences.putInt("setTempOff", setTempOff);                         // Guardar las nuevas temperaturas de APAGADO en NVS

              /* ------------------------------------------------ Archivo JSON con información de CONFIGURACIÓN del AC ------------------------------------------------ */
              String dataAire = "{\"type\":\"aire\",\"IP\":\"" + ipStr + "\",\"Encendido\":" + String(setTempOn) + ",\"Apagado\":" + String(setTempOff) + "}";
              /* ------------------------------------------------ Archivo JSON con información de CONFIGURACIÓN del AC ------------------------------------------------ */

              HTTPClient http;
              http.begin(scriptUrl);
              http.addHeader("Content-Type", "application/json");
              int httpResponseCode = http.POST(dataAire);
              if (httpResponseCode > 0) {
                Serial.printf("Datos deconfiguración del aire acondicionado enviados al servidor, código de respuesta: %d\n", httpResponseCode);
              } else {
                Serial.printf("Error al enviar datos de configuración del aire acondicionado al servidor, código de respuesta: %d\n", httpResponseCode);
              }
              http.end();
            }
          } else {
            Serial.println("Error al parsear mensaje JSON");
          }
        }
        /* ------------------------------------------------ Envio de datos de CONFIGURACIÓN del AC por medio de formato POST ------------------------------------------------ */

        break;
      }
  }
}
