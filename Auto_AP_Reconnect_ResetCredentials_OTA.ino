/*
   2021 - Marce Ferra
   Base de firmware para ESP32 con las siguientes funciones:
    - AutoConnect de Hieromon https://hieromon.github.io/AutoConnect/
    - OTA - Mix de los ejemplos de Hieromon
    - Reset de credenciales almacenadas por AutoConnect y disparada por un pulsador
    - Reconectar automaticamente ante pérdida de señal WiFi (sin necesidad de reset)


    Si esta información te resulta útil e interesante, invitame un cafecito!!!
    https://cafecito.app/marce_ferra

    Desde fuera de Argentina en:
    https://www.buymeacoffee.com/marceferra

     O podés colaborar comprando algunos de los objetos creados en los tutoriales del blog:
     https://listado.mercadolibre.com.ar/_Envio_MercadoEnvios_CustId_13497891

    Gracias!!!
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "HTTPUpdateServer.h"
#define HOSTIDENTIFY  "esp32"
#define mDNSUpdate(c)  do {} while(0)
using WebServerClass = WebServer;
using HTTPUpdateServerClass = HTTPUpdateServer;

#include <WiFiClient.h>
#include <AutoConnect.h>

// Hostname del mDNS
static const char* host = HOSTIDENTIFY "-webupdate";
#define HTTP_PORT 80

//Instancia compatida de ESP8266WebServer por AutoConnect y UpdateServer
WebServerClass  httpServer(HTTP_PORT);

#define USERNAME "user"   // Usuario para OTA
#define PASSWORD "pass"   // Contraseña para OTA

HTTPUpdateServerClass httpUpdater;
AutoConnectAux  update("/update", "Update");

AutoConnect     Portal(httpServer);
AutoConnectAux  hello;

#define salida 2 //LED onboard
#define pulsador 13
#define led_ROJO 12

void rootPage() {

  // Pagina web principal!
  String pagina = "<!DOCTYPE html>"
                  "<html>"
                  "<head>"
                  "<meta charset='utf-8' />"
                  "<title>Marcelo Web ESP32</title>"
                  "</head>"
                  "<body>"
                  "<center>"
                  "<h1>Marcelo Web ESP32</h1>"
                  "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                  "<img src='data:image/png;base64, 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mLPo3l1u6p54BNyNyPY50wJU2gpOZFRI1IQyRrdHQPdAQTyw87Io857QspZFRM4cDl7QlcdX1QHkalchHoq4CQdBAKRuM/SQX7zouYXBOIVP4KKtrpPI1r60TaClzYSLUfPofKrNkC2I7/fRA9e4M4KNoYecG63SvVVKDecdQb9LLGwVxIUt7oZbACOnByIoAIVeu1g2YOKJUDJVSAXHeTwvTMSieoFoDgZrWqDsA5lVHspjgyFA3xLAc8x1Cl+1V1iudiBSPaKwVEKt2K6phh7wqiWINFOR8+pPS8qgkrJfGP8Ajv/aAAwDAQACAAMAAAAQ8tNNNNNNNNNNNNNY888+HAAAAAAAAAAAd/8APPPlPvvvvvvvvvvlP/PPPg/ffevvI8cccVf/ADzz5Dzzy37zn3339T/zzz4Dzzz37z488/xT/wA88+A888928zzzzz3/APPPPgPPPPf/ADzzzzxT/wA88+A088598822Xj/r888+E1889+8/jO88Ur888+QLT89+8l9888Ur888+A8Lx9++X8888Ur888+A88ps29e8888Ur888+HMMOtvOuMMMM/r8880vLDDDDDDDDDDTj888888888888888888888oAA88888Aoc88888888AU8YIc8As888M08888AU8QMc8AI08k4c8888Mc8cs88c8w8sQ888888888888888888888//xAAgEQACAgEFAQEBAAAAAAAAAAAAARExYRAgITBxQEFg/9oACAEDAQE/ECJMno9Ho9Ho9Ho9Ho9Ho9HoiTtkHHYAAADDj6CyamC2FNSllNGtxtqkoh1FsKaJ4PYAmnXLYU0aWMBgMAkp1y2FNGnG1VTmT6ypR8MrQpEJK1aWG7o5EIStuJJgQcPWwpopJGNLEU6pbCIOHpYU0dQlChdksnKPwZYUGhNiW+6WTlH7ChcUfXJolsqblyN4IkGU21TTLaIXIzGYzGYzGYzGYzGYzGYzCmuP6/8A/8QAKBEAAgAFAwQCAwEBAAAAAAAAAAERMWFx8CCRsRAh0eFBUYGhwfFg/9oACAECAQE/EB70j/Poyv0ZX6Mr9GV+jK/Rlfoyv0ZX6Mr9GV+jK/Rlfoyv0NiYe0Z+tEyyO7KaTmyh+zAzAzAzAzAzAzAzAzAzAzu6km5sn3crRNsjHYiyLIsiyLIsiyLIsiyLFnYn3crR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' alt='Logo' />"

                  "<p><a href='/on'><button style='height:50px;width:100px'>ON</button></a></p>"
                  "<p><a href='/off'><button style='height:50px;width:100px'>OFF</button></a></p>"
                  "<p><a href='/flash'><button style='height:50px;width:100px'>Flash!</button></a></p>"
                  "<p><a href='/_ac'>Opciones</a></p>"
                  "</center>"
                  "</body>"
                  "</html>";

  httpServer.send(200, "text/html", pagina);

}

void web_ON() {
  Serial.println("GPIO on");
  digitalWrite(salida, HIGH);
  rootPage();
}

void web_OFF() {
  Serial.println("GPIO off");
  digitalWrite(salida, LOW);
  rootPage();
}

void web_FLASH() {
  Serial.println("Fiestaaaaaa!!!");
  rootPage();
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
  digitalWrite(salida, HIGH);
  delay(50);
  digitalWrite(salida, LOW);
  delay(50);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  pinMode(salida, OUTPUT);
  pinMode(led_ROJO, OUTPUT);
  digitalWrite(salida, LOW);

  pinMode(pulsador, INPUT_PULLUP);

  httpServer.on("/", rootPage);
  httpServer.on("/on", web_ON);
  httpServer.on("/off", web_OFF);
  httpServer.on("/flash", web_FLASH);

  WiFi.onEvent(Wifi_disconnected, SYSTEM_EVENT_STA_DISCONNECTED);

  // /update handler
  httpUpdater.setup(&httpServer, USERNAME, PASSWORD);

  Portal.join({ update });

  if (Portal.begin()) {
    if (MDNS.begin(host)) {
      MDNS.addService("http", "tcp", HTTP_PORT);
      Serial.printf("OTA activado! Ingresar a http://%s.local/update\n", host);
      Serial.println("IP: " + WiFi.localIP().toString() );
    }
    else
      Serial.println("MDNS con error");
  }
}

void loop() {
  Portal.handleClient();

  // Si se apreta el pulsador se borran las credenciales de AutoConnect y la de WiFi.Begin() - Luego se hace un reset...
  if (digitalRead(pulsador) == 0) {
    digitalWrite(led_ROJO, !digitalRead(pulsador));
    //LED rojo de la muerte
    while (!digitalRead(pulsador)); //Antirebote
    deleteAllCredentials(); //Borra toda las credenciales del AutoConnect de Hieromon
    WiFi.disconnect(false, true); //WiFi.disconnect(bool wifioff, bool eraseap) - hace la magia con borrar la ultima credencial
    ESP.restart(); //Reinicia el ESP
  }
}

void deleteAllCredentials(void) {
  AutoConnectCredential credential;
  station_config_t config;
  uint8_t ent = credential.entries();

  Serial.printf("credentials.entries = %d\n", ent);

  while (ent--) {
    credential.load((int8_t)0, &config);
    credential.del((const char*)&config.ssid[0]);
  }
}

void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Desconectado del WiFi!!!");
  Serial.print("Motivo: ");
  Serial.println(info.disconnected.reason);
  Serial.println("Reconectando...");
  WiFi.begin();
}
