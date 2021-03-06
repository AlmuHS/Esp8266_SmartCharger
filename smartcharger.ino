#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

const int SENSORLUZ = A0;
const int NUM_MEDIDAS = 10;

enum lista_estados{
  INDETERMINADO = 1,
  COCHE_APARCADO = 2,
  CARGANDO_PROGRAMADO = 3,
  CARGANDO_USUARIO = 4,
  COCHE_FUERA = 5
};

extern Adafruit_MQTT_Client mqtt;

extern Adafruit_MQTT_Subscribe hora_inicio_carga;
extern Adafruit_MQTT_Subscribe hora_fin_carga;
extern Adafruit_MQTT_Subscribe orden_carga;
extern Adafruit_MQTT_Subscribe pregunta_potencia;

// Declaración de la clase para la máquina de estados
class Estado{
  private:
    lista_estados estado_inicial;
    lista_estados estado_actual;
    int tiempo_inicio_carga;

    void indeterminado(void);
    void coche_fuera(void);
    void cargando_programado(void);
    void cargando_usuario(void);
    void coche_aparcado(void);
    void cargar(void);
    void empezar_carga_programada(void);
    void terminar_carga_programada(void);

  public:
    Estado(lista_estados estado_inicial = INDETERMINADO);
    void avanzar_estado(void);
  
};

// Declaración de la estructura de programación de carga en el cargador
struct programa_carga{
  int hora_inicio = 17;
  int min_inicio = 5;

  int hora_fin = 20;
  int min_fin = 5;
  
  int potencia = 4000;
};

// Instancia de la máquina de estados
Estado maquina_estados(INDETERMINADO);

void setup() { 
  Serial.begin(115200);

  pinMode(SENSORLUZ, INPUT);

  conectarWiFi();

  // Conexión de cada objeto MQTT con la función a ejecutar ante cada tipo de mensaje
  hora_inicio_carga.setCallback(hora_inicio_callback);
  hora_fin_carga.setCallback(hora_fin_callback);
  orden_carga.setCallback(orden_callback);
  pregunta_potencia.setCallback(potencia_callback);
  
  // Suscripciones de los canales, asociados al objeto de cada tipo de mensaje 
  mqtt.subscribe(&hora_inicio_carga);
  mqtt.subscribe(&hora_fin_carga);
  mqtt.subscribe(&orden_carga);
  mqtt.subscribe(&pregunta_potencia);
 
}


void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);

  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  
  mostrar_hora_actual();
  
  int luz = lee_sensor(SENSORLUZ);
  muestra_sensores(luz);

  maquina_estados.avanzar_estado(); 

}
