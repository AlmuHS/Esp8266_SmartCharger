
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Time.h>
#include <TimeAlarms.h>

const int SENSORLUZ = A0;
const int NUM_MEDIDAS = 10;

enum lista_estados{
  DESCONECTADO = 1,
  COCHE_APARCADO = 2,
  CARGANDO_TIEMPO = 3,
  CARGANDO_USUARIO = 4,
  COCHE_FUERA = 5
};


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       ""
#define WLAN_PASS       ""

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "192.168.1.122"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "usuario"
#define AIO_KEY         "usuario"


// Create an ESP8266 WiFiClient class to connect to the MQTT server.
extern WiFiClient client;

//Create a ntp cliente via udp
WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP, "hora.roa.es", 7200);

extern Adafruit_MQTT_Client mqtt;

extern Adafruit_MQTT_Subscribe hora_inicio_carga;
extern Adafruit_MQTT_Subscribe hora_fin_carga;
extern Adafruit_MQTT_Subscribe orden_carga;
extern Adafruit_MQTT_Subscribe pregunta_potencia;

extern Adafruit_MQTT_Publish aviso_carga;
extern Adafruit_MQTT_Publish coche_llegasale;
extern Adafruit_MQTT_Publish coche_aparcado;
extern Adafruit_MQTT_Publish potencia_instantanea;
extern Adafruit_MQTT_Publish potencia_cargador;
extern Adafruit_MQTT_Publish tiempo_carga;
extern Adafruit_MQTT_Publish potencia_acum;

struct programa_carga{
  int hora_inicio = 17;
  int min_inicio = 5;

  int hora_fin = 20;
  int min_fin = 5;
  
  int potencia = 4000;
};

programa_carga programa;

class Estado{
  int estado_inicial;
  int estado_actual;

  private:
    void desconectado(void);
    void coche_fuera(int luz);
    void cargando_tiempo(void);
    void cargando_usuario(void);
    void coche_aparcado(void);

  public:

  Estado(int estado_inicial = COCHE_FUERA);

  void set_estado_actual(int estado_actual);
  int get_estado_actual(void);
  void avanzar_estado(void);
  
};

Estado maquina_estados;

int tiempo_inicio_carga;
int potencia_acumulada = 0;

void empezar_carga(void){
  if(maquina_estados.get_estado_actual() == COCHE_APARCADO){
    maquina_estados.set_estado_actual(CARGANDO_TIEMPO);
    publicar_evento(aviso_carga, "comienza carga programada por franja horaria"); 

    tiempo_inicio_carga = millis();
    potencia_acumulada = 0;
  }
}

void terminar_carga(void){
  if(maquina_estados.get_estado_actual() == CARGANDO_TIEMPO){
    maquina_estados.set_estado_actual(COCHE_APARCADO);
    
    publicar_evento(aviso_carga, "termina carga por final de franja programada");
  }
}

void setup() { 
  Serial.begin(115200);

  pinMode(SENSORLUZ, INPUT);

  conectarWiFi();
  timeClient.begin();

  int hour, minute, seconds;
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  seconds = timeClient.getSeconds();
  
  setTime(hour,minute,seconds,1,1,11);

  hora_inicio_carga.setCallback(hora_inicio_callback);
  hora_fin_carga.setCallback(hora_fin_callback);
  orden_carga.setCallback(orden_callback);
  pregunta_potencia.setCallback(potencia_callback);
  
  // Setup MQTT subscription for feed.
  mqtt.subscribe(&hora_inicio_carga);
  mqtt.subscribe(&hora_fin_carga);
  mqtt.subscribe(&orden_carga);
  mqtt.subscribe(&pregunta_potencia);
 
}


void loop() {
  MQTT_connect();
  
  Alarm.delay(100);
  mqtt.processPackets(10000);

  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  
  mostrar_hora_actual();
  
  int luz = lee_sensor(SENSORLUZ);
  muestra_sensores(luz);

  maquina_estados.avanzar_estado();
  
  
  publicar_evento(potencia_cargador, "4");

}

void descomponer_hora_minuto(char hora_minuto_str[6], int hora_minuto[2]){
  int hora = (hora_minuto_str[0] - 48) * 10;
      hora += (hora_minuto_str[1] - 48);

  int minuto = (hora_minuto_str[3] - 48) * 10;
      minuto += (hora_minuto_str[4] - 48);
  
  hora_minuto[0] = hora;
  hora_minuto[1] = minuto;
}

void mostrar_hora_actual(void){
  timeClient.update();
  char hourminutes[15] = "";

  int hour, minute;
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  
  snprintf(hourminutes, 15, "%02d:%02d", hour, minute);
  Serial.println(hourminutes);
}
