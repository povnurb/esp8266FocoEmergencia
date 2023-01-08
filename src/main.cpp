//realizar para el esp8266
//que cuando entre si esta el foco prendido aparesca prendido en el html
//quitar lo que no se usa para que no ocupe espacio en el esp8266
//ponerle un DNS
#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>  //ESP-FTP-Server-Lib by Peter Buchegger
#include <ESPAsyncWebServer.h>  //version 3.0.0 ESPAsyncWebServer-esphome by ESPHome Team
#define RELAY 0
void initWiFiAP();
void InitWiFi();
void initServer(); //iniciamos la funcion 
void NotFound(AsyncWebServerRequest *request);
void RedesRequest(AsyncWebServerRequest *request);
void ConectarWiFi(AsyncWebServerRequest *request);
void ENCENDER(AsyncWebServerRequest *request);
void APAGAR(AsyncWebServerRequest *request);
String header;
String SSID="", PASS="";
const String lugar = "sala";
bool wifiIsConnected=false;
bool indicadorLed;
long tiempoDeConexion;
AsyncWebServer server(80);
DNSServer dnsServer;
class myHandler : public AsyncWebHandler { //myHandler hereda todo de AsyncWebHandler
  public:
    myHandler() {} //constructor
    virtual ~myHandler() {} //destructor
    bool canHandle(AsyncWebServerRequest *request){
      Serial.println("Entramos al canHandle");
      return true; //para cualquier tipo de peticion
    }
    void handleRequest(AsyncWebServerRequest *request) {
      Serial.println("Entramos al handle");
      request->send(SPIFFS, "/index.html"); //redireccionar a la pagina principal
    }
};
void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  Serial.println();
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  InitWiFi();
}
void loop(){
  if(WiFi.status() != WL_CONNECTED){  //se hace para que se mantenga a la escucha de las peticiones
  //cuando esta desconectado, ya que en cuanto se pida una peticion nos enviara directamente
  //a la pagina principal
    dnsServer.processNextRequest();
  }
}
void NotFound(AsyncWebServerRequest *request){
  request->send(SPIFFS, "/indexNotFound.html");
  Serial.println("pagina no encontrada");
}
void ENCENDER(AsyncWebServerRequest *request){
  request->send(SPIFFS, "/RELAY=ON.html"); //a donde me tiene que llevar
  digitalWrite(RELAY, LOW);
  indicadorLed= true; //indica al programa que esta encendido el LED
  Serial.println("se enciente el LED");
}
void APAGAR(AsyncWebServerRequest *request){
  request->send(SPIFFS, "/RELAY=OFF.html"); //a donde me tiene que llevar
  digitalWrite(RELAY, HIGH);
  indicadorLed= false; //indica al programa que esta encendido el LED
  Serial.println("entramos a apagar, y se apagamos el LED");
}
void initWiFiAP(){
  WiFi.mode(WIFI_AP);
  while(!WiFi.softAP(lugar.c_str())){
    Serial.println(".");
    delay(100);  
  }
  Serial.print("Direccion IP AP: ");
  Serial.println(WiFi.softAPIP());
  initServer();
}
void InitWiFi(){
  WiFi.mode(WIFI_STA);      
  // Inicializamos el WiFi con nuestras credenciales.
  Serial.print("Conectando a ");
  Serial.print(SSID);
  WiFi.begin(SSID.c_str(), PASS.c_str());
  tiempoDeConexion = millis();
  while(WiFi.status() != WL_CONNECTED){     // Se quedata en este bucle hasta que el estado del WiFi sea diferente a desconectado.
    Serial.print(".");
    delay(100);
    if(tiempoDeConexion+7000 < millis()){
      break;
    }
  }
  if(WiFi.status() == WL_CONNECTED){        // Si el estado del WiFi es conectado entra al If
    Serial.println();
    Serial.println();
    Serial.println("Conexion exitosa!!!");
    Serial.println("");
    Serial.print("Tu IP STA: ");
    Serial.println(WiFi.localIP());
  }
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("");
    Serial.println("No se logro conexion");
    initWiFiAP();
  }
}
void initServer(){
  Serial.println("Iniciando el servidor index");
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.on("/RELAY=ON", HTTP_GET, ENCENDER);
  server.on("/RELAY=OFF", HTTP_GET, APAGAR);
  server.onNotFound(NotFound); //pagina no encontrada
  dnsServer.start(53, "*", WiFi.softAPIP()); //escucha por el puerto 53 y todas las peticiones "*"
  //las redirecciona a WiFi.softAPIP() (otra ip una ip de nuestro esp32 en modo Acces Point)
  server.addHandler(new myHandler()).setFilter(ON_AP_FILTER);
  //la instruccion anterios .setFilter(ON_AP_FILTER) pregunta si la ip viene del acces point
  //myHandler() es una clase
	server.begin();
  Serial.println("Servidor iniciado");
}

