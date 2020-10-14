// ██╗███╗   ██╗ ██████╗██╗     ██╗   ██╗██████╗ ███████╗███████╗
// ██║████╗  ██║██╔════╝██║     ██║   ██║██╔══██╗██╔════╝██╔════╝
// ██║██╔██╗ ██║██║     ██║     ██║   ██║██║  ██║█████╗  ███████╗
// ██║██║╚██╗██║██║     ██║     ██║   ██║██║  ██║██╔══╝  ╚════██║
// ██║██║ ╚████║╚██████╗███████╗╚██████╔╝██████╔╝███████╗███████║
// ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝╚══════╝

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <Ticker.h>

#include "credentials.h"

//  ██████╗ ██████╗ ███╗   ███╗███╗   ███╗██╗   ██╗███╗   ██╗██╗ ██████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
// ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██║   ██║████╗  ██║██║██╔════╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║     ██║   ██║██╔████╔██║██╔████╔██║██║   ██║██╔██╗ ██║██║██║     ███████║   ██║   ██║██║   ██║██╔██╗ ██║
// ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██║   ██║██║╚██╗██║██║██║     ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
// ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║╚██████╗██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
//  ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

//  ██████╗ ██╗   ██╗████████╗██████╗  ██████╗ ██╗   ██╗███╗   ██╗██████╗
// ██╔═══██╗██║   ██║╚══██╔══╝██╔══██╗██╔═══██╗██║   ██║████╗  ██║██╔══██╗
// ██║   ██║██║   ██║   ██║   ██████╔╝██║   ██║██║   ██║██╔██╗ ██║██║  ██║
// ██║   ██║██║   ██║   ██║   ██╔══██╗██║   ██║██║   ██║██║╚██╗██║██║  ██║
// ╚██████╔╝╚██████╔╝   ██║   ██████╔╝╚██████╔╝╚██████╔╝██║ ╚████║██████╔╝
//  ╚═════╝  ╚═════╝    ╚═╝   ╚═════╝  ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚═════╝

// These must be kept in sync: computer client code <=> arduino code <=> esp32 code

#define ESP32_CONNECTED 10
#define TEMPERATURE_HUMIDITY_NOW_REQUEST 11
#define ARDUINO_OPERATION_MODE_REQUEST 12

//  ██████╗ ██████╗ ███╗   ███╗███╗   ███╗██╗   ██╗███╗   ██╗██╗ ██████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
// ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██║   ██║████╗  ██║██║██╔════╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║     ██║   ██║██╔████╔██║██╔████╔██║██║   ██║██╔██╗ ██║██║██║     ███████║   ██║   ██║██║   ██║██╔██╗ ██║
// ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██║   ██║██║╚██╗██║██║██║     ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
// ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║╚██████╗██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
//  ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

// ██╗███╗   ██╗██████╗  ██████╗ ██╗   ██╗███╗   ██╗██████╗
// ██║████╗  ██║██╔══██╗██╔═══██╗██║   ██║████╗  ██║██╔══██╗
// ██║██╔██╗ ██║██████╔╝██║   ██║██║   ██║██╔██╗ ██║██║  ██║
// ██║██║╚██╗██║██╔══██╗██║   ██║██║   ██║██║╚██╗██║██║  ██║
// ██║██║ ╚████║██████╔╝╚██████╔╝╚██████╔╝██║ ╚████║██████╔╝
// ╚═╝╚═╝  ╚═══╝╚═════╝  ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚═════╝

// These must be kept in sync: computer client code <=> arduino code <=> esp32 code

#define ARDUINO_CONNECTED "arduino connected"

//  ██████╗ ██╗      ██████╗ ██████╗  █████╗ ██╗     ███████╗
// ██╔════╝ ██║     ██╔═══██╗██╔══██╗██╔══██╗██║     ██╔════╝
// ██║  ███╗██║     ██║   ██║██████╔╝███████║██║     ███████╗
// ██║   ██║██║     ██║   ██║██╔══██╗██╔══██║██║     ╚════██║
// ╚██████╔╝███████╗╚██████╔╝██████╔╝██║  ██║███████╗███████║
//  ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝

const char *ssid = ssid_credential;
const char *password = password_credential;

WebServer server(80);

const int led = 13;

bool connected_to_arduino = false;

// Do not use directly. Use mutex functions instead
bool arduino_locked = false;

Ticker ticker;

//  █████╗ ██████╗ ██████╗ ██╗   ██╗██╗███╗   ██╗ ██████╗
// ██╔══██╗██╔══██╗██╔══██╗██║   ██║██║████╗  ██║██╔═══██╗
// ███████║██████╔╝██║  ██║██║   ██║██║██╔██╗ ██║██║   ██║
// ██╔══██║██╔══██╗██║  ██║██║   ██║██║██║╚██╗██║██║   ██║
// ██║  ██║██║  ██║██████╔╝╚██████╔╝██║██║ ╚████║╚██████╔╝
// ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝

// ██╗      ██████╗  ██████╗██╗  ██╗    ███╗   ███╗██╗   ██╗████████╗███████╗██╗  ██╗
// ██║     ██╔═══██╗██╔════╝██║ ██╔╝    ████╗ ████║██║   ██║╚══██╔══╝██╔════╝╚██╗██╔╝
// ██║     ██║   ██║██║     █████╔╝     ██╔████╔██║██║   ██║   ██║   █████╗   ╚███╔╝
// ██║     ██║   ██║██║     ██╔═██╗     ██║╚██╔╝██║██║   ██║   ██║   ██╔══╝   ██╔██╗
// ███████╗╚██████╔╝╚██████╗██║  ██╗    ██║ ╚═╝ ██║╚██████╔╝   ██║   ███████╗██╔╝ ██╗
// ╚══════╝ ╚═════╝  ╚═════╝╚═╝  ╚═╝    ╚═╝     ╚═╝ ╚═════╝    ╚═╝   ╚══════╝╚═╝  ╚═╝

void get_arduino()
{
  while (arduino_locked)
  {
    delay(500);
  }
  arduino_locked = true;
}

void release_arduino()
{
  arduino_locked = false;
}

// ██╗  ██╗████████╗████████╗██████╗     ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗
// ██║  ██║╚══██╔══╝╚══██╔══╝██╔══██╗    ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗
// ███████║   ██║      ██║   ██████╔╝    ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝
// ██╔══██║   ██║      ██║   ██╔═══╝     ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗
// ██║  ██║   ██║      ██║   ██║         ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║
// ╚═╝  ╚═╝   ╚═╝      ╚═╝   ╚═╝         ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝

// ██████╗  ██████╗ ██╗   ██╗████████╗███████╗    ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ███████╗██████╗ ███████╗
// ██╔══██╗██╔═══██╗██║   ██║╚══██╔══╝██╔════╝    ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██╔════╝██╔══██╗██╔════╝
// ██████╔╝██║   ██║██║   ██║   ██║   █████╗      ███████║███████║██╔██╗ ██║██║  ██║██║     █████╗  ██████╔╝███████╗
// ██╔══██╗██║   ██║██║   ██║   ██║   ██╔══╝      ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██╔══╝  ██╔══██╗╚════██║
// ██║  ██║╚██████╔╝╚██████╔╝   ██║   ███████╗    ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗███████╗██║  ██║███████║
// ╚═╝  ╚═╝ ╚═════╝  ╚═════╝    ╚═╝   ╚══════╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝

void handleRoot()
{
  digitalWrite(led, 1);
  String message = "Olá do ESP32 !\n";
  message += connected_to_arduino ? "Arduino conectado !\n" : "Arduino não-conectado\n";
  server.send(200, "text/plain", message);
  digitalWrite(led, 0);
}

void handleNotFound()
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleMeasureAndSend()
{
  get_arduino();
  Serial.write(TEMPERATURE_HUMIDITY_NOW_REQUEST); // request data from arduino
  while (Serial.available() == 0)
  {
    // wait
  }
  float temperature = Serial.parseFloat();
  float humidity = Serial.parseFloat();
  release_arduino();

  String url = sheets_url_credential;
  url += "?token=";
  url += sheets_token_credential;
  url += "&temperature=";
  url += temperature;
  url += "&humidity=";
  url += humidity;
  HTTPClient http;
  const char *headers[1];
  headers[0] = "Location";
  http.collectHeaders(headers, 1);
  http.begin(url);
  int http_code = http.GET();

  if (http_code == HTTP_CODE_FOUND)
  { // actually means redirect
    String redirect_location = http.header("Location");
    http.end();
    http.begin(redirect_location);
    http_code = http.GET();
  }

  String result = "Temperatura = ";
  result += temperature;
  result += "C\n";
  result += "Umidade = ";
  result += humidity;
  result += "%\n";

  if (http_code > 0 && http_code == HTTP_CODE_OK)
  {
    result += "Envio ao google sheets OK. Resposta foi: " + http.getString() + "\n";
  }
  else
  {
    result += "ERRO no envio ao google sheets. Status do erro: ";
    result += http_code;
    result += "\n";
    result += "Mensagem do erro (proxima linha):\n";
    result += http.getString();
  }

  server.send(200, "text/plain", result);
}

void handleMeasure()
{
  get_arduino();
  Serial.write(TEMPERATURE_HUMIDITY_NOW_REQUEST); // request data from arduino
  while (Serial.available() == 0)
  {
    // wait
  }
  float temperature = Serial.parseFloat();
  float humidity = Serial.parseFloat();
  release_arduino();

  String result = "<html><head>";
  result += "<meta name=\"viewport\" content=\"width=device-width, height=device-height, initial-scale=1.0, minimum-scale=1.0\">";
  result += "</head><body>";
  result += "<h1>Temperatura = ";
  result += temperature;
  result += "C</h1>";
  result += "<h1>Umidade = ";
  result += humidity;
  result += "%</h1></body></html>";

  server.send(200, "text/html", result);
}

void handleArduinoOperationMode()
{
  get_arduino();
  Serial.write(ARDUINO_OPERATION_MODE_REQUEST);
  while (Serial.available() == 0)
  {
    // wait
  }
  byte mode = Serial.read();
  release_arduino();

  int mode_int = (int)mode;
  String mode_string = "Arduino está no modo ";
  mode_string += mode_int;

  server.send(200, "text/plain", mode_string);
}

// ████████╗██╗███╗   ███╗███████╗██████╗      █████╗  ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
// ╚══██╔══╝██║████╗ ████║██╔════╝██╔══██╗    ██╔══██╗██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
//    ██║   ██║██╔████╔██║█████╗  ██████╔╝    ███████║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
//    ██║   ██║██║╚██╔╝██║██╔══╝  ██╔══██╗    ██╔══██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
//    ██║   ██║██║ ╚═╝ ██║███████╗██║  ██║    ██║  ██║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
//    ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═╝    ╚═╝  ╚═╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝

void measureAndSend()
{
  get_arduino();
  Serial.write(TEMPERATURE_HUMIDITY_NOW_REQUEST); // request data from arduino
  while (Serial.available() == 0)
  {
    // wait
  }
  float temperature = Serial.parseFloat();
  float humidity = Serial.parseFloat();
  release_arduino();

  String url = sheets_url_credential;
  url += "?token=";
  url += sheets_token_credential;
  url += "&temperature=";
  url += temperature;
  url += "&humidity=";
  url += humidity;
  HTTPClient http;
  const char *headers[1];
  headers[0] = "Location";
  http.collectHeaders(headers, 1);
  http.begin(url);
  int http_code = http.GET();

  if (http_code == HTTP_CODE_FOUND)
  { // actually means redirect
    String redirect_location = http.header("Location");
    http.end();
    http.begin(redirect_location);
    http_code = http.GET();
  }

  String result = "Temperatura = ";
  result += temperature;
  result += "C\n";
  result += "Umidade = ";
  result += humidity;
  result += "%\n";

  if (http_code > 0 && http_code == HTTP_CODE_OK)
  {
    result += "Envio ao google sheets OK. Resposta foi: " + http.getString() + "\n";
  }
  else
  {
    result += "ERRO no envio ao google sheets. Status do erro: ";
    result += http_code;
    result += "\n";
    result += "Mensagem do erro (proxima linha):\n";
    result += http.getString();
  }

  // TODO: log errors somewhere, somehow. Lest they pass silently and can lead to confusion
  // (e.g. "it is not working, but why ?!!")
  // server.send(200, "text/plain", result);
}

// ███████╗███████╗████████╗██╗   ██╗██████╗
// ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ███████║███████╗   ██║   ╚██████╔╝██║
// ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝

void setup(void)
{
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);

  // we expect arduino to attempt to identify its mode. respond here
  get_arduino();
  while (Serial.available() == 0)
  {
  }
  String response = Serial.readStringUntil('\n');
  response.trim();
  if (response.equals(ARDUINO_CONNECTED))
  {
    Serial.write(ESP32_CONNECTED);
    connected_to_arduino = true;
  }
  else
  {
    Serial.println("unknown message from arduino");
    connected_to_arduino = false;
  }
  release_arduino();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  //Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    // Serial.print(".");
  }
  //Serial.println("");
  //Serial.print("Connected to ");
  //Serial.println(ssid);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32"))
  {
    //Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  if (connected_to_arduino)
  {
    server.on("/measure", handleMeasure);
    server.on("/measure-and-send", handleMeasureAndSend);
    server.on("/arduino-operation-mode", handleArduinoOperationMode);

    ticker.attach(3600, measureAndSend);
  }

  server.begin();
  //Serial.println("HTTP server started");
}

// ██╗      ██████╗  ██████╗ ██████╗
// ██║     ██╔═══██╗██╔═══██╗██╔══██╗
// ██║     ██║   ██║██║   ██║██████╔╝
// ██║     ██║   ██║██║   ██║██╔═══╝
// ███████╗╚██████╔╝╚██████╔╝██║
// ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝

void loop(void)
{
  server.handleClient();
}
