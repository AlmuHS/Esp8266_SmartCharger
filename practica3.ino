
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

int estado = COCHE_FUERA;


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

const int INICIA_CARGA = 1;
const int TERMINA_CARGA = 2;

struct programa_carga{
  int hora_inicio = 17;
  int min_inicio = 5;

  int hora_fin = 20;
  int min_fin = 5;
  
  int potencia = 4000;
};

programa_carga programa;
int orden = 0;

int tiempo_inicio_carga;
int potencia_acumulada = 0;

void empezar_carga(void){
  if(estado == COCHE_APARCADO){
    estado = CARGANDO_TIEMPO;
    publicar_evento(aviso_carga, "comienza carga programada por franja horaria"); 

    tiempo_inicio_carga = millis();
    potencia_acumulada = 0;
  }
}

void terminar_carga(void){
  if(estado == CARGANDO_TIEMPO){
    estado = COCHE_APARCADO;
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
  
  if(estado == COCHE_FUERA){
    if(luz > 823){
      estado = COCHE_APARCADO;
      publicar_evento(coche_llegasale, "llega coche");
    }
    else{
      publicar_evento(coche_aparcado, "coche fuera");
    }
  }
  
  else if(estado == COCHE_APARCADO){
    //Inicia la carga
    if(orden == INICIA_CARGA){
      estado = CARGANDO_USUARIO;
      publicar_evento(aviso_carga, "comienza carga por orden del usuario");
     
      tiempo_inicio_carga = millis();
      potencia_acumulada = 0;
      orden = 0;
    }
    //Consideramos que el coche ha salido cuando la luz es demasiado baja
    else if(luz < 600){
      estado = COCHE_FUERA;
      publicar_evento(coche_llegasale, "sale_coche");
    }
    else{
      publicar_evento(coche_aparcado, "coche aparcado y desconectado");
    }
  }

  
  else if(estado == CARGANDO_USUARIO){
    if(orden == TERMINA_CARGA){
      estado = COCHE_APARCADO;
      publicar_evento(aviso_carga, "termina carga por orden del usuario");
      orden = 0;
    }
    else if(potencia_acumulada >= programa.potencia){
      estado = COCHE_APARCADO;
      publicar_evento(aviso_carga, "termina carga por llegar a potencia maxima");
    }
  }
  else if(estado == CARGANDO_TIEMPO){
    if(potencia_acumulada >= programa.potencia){
      estado = COCHE_APARCADO;
      publicar_evento(aviso_carga, "termina carga por llegar a potencia maxima");
    }
  }
  
  if(estado != CARGANDO_USUARIO && estado != CARGANDO_TIEMPO){
    publicar_evento(potencia_instantanea, "0");
  }
  else{
      int tiempo_carga = (millis() - tiempo_inicio_carga)/(1000*60);
      potencia_acumulada += 100;
      char mensaje[100];
      float potencia_kwh = (float) potencia_acumulada / 1000.0;

      char potencia_str[10];

      snprintf(mensaje, 100, "coche cargando. Tiempo de carga: %d minutos. Potencia acumulada: %f", tiempo_carga, potencia_kwh);
      snprintf(potencia_str, 10, "%d", potencia_acumulada);

      
      publicar_evento(coche_aparcado, mensaje);
      publicar_evento(potencia_acum, potencia_str);

      publicar_evento(potencia_instantanea, "4");
    }
  
  
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
