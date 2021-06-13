/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "192.168.1.122"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "usuario"
#define AIO_KEY         "usuario"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//************************************* Suscripciones *******************************************************
Adafruit_MQTT_Subscribe hora_inicio_carga = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/cargador/hora_inicio");
Adafruit_MQTT_Subscribe hora_fin_carga = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/cargador/hora_fin");
Adafruit_MQTT_Subscribe orden_carga = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/cargador/orden_carga"); //Inicio o fin de la carga
Adafruit_MQTT_Subscribe pregunta_potencia = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/cargador/potencia_necesita");

//************************************* Publicaciones *******************************************************
Adafruit_MQTT_Publish aviso_carga = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/cargador/carga_inicia_fin");
Adafruit_MQTT_Publish coche_llegasale = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/cargador/coche");
Adafruit_MQTT_Publish coche_aparca = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/cargador/estado_coche");
Adafruit_MQTT_Publish potencia_instantanea = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/cargador/potencia_instantanea");
Adafruit_MQTT_Publish potencia_cargador = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/cargador/potencia_cargador");
Adafruit_MQTT_Publish tiempo_carga = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/cargador/coche_aparcado/tiempo_carga");
Adafruit_MQTT_Publish potencia_acum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/cargador/coche_aparcado/potencia_acum");

void hora_inicio_callback(char* hora, short unsigned int longitud) {
  Serial.print("Carga programada inicia a hora ");
  Serial.println(hora);
  
  int hora_minuto[2];
  descomponer_hora_minuto(hora, hora_minuto);

  programa.hora_inicio = hora_minuto[0];
  programa.min_inicio = hora_minuto[1];
}

void hora_fin_callback(char* hora, short unsigned int longitud){
  Serial.print("Carga programada finaliza a hora ");
  Serial.println(hora);
  
  int hora_minuto[2];
  
  descomponer_hora_minuto(hora, hora_minuto);

  programa.hora_fin = hora_minuto[0];
  programa.min_fin = hora_minuto[1];
}

void potencia_callback(uint32_t potencia){
    Serial.println(potencia);
    programa.potencia = potencia;
}


void orden_callback(uint32_t _orden){
  orden = (tipo_orden) _orden;
  Serial.println(orden);
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

bool publicar_evento(Adafruit_MQTT_Publish evento, char* mensaje){
  bool publicado = false;
  
  Serial.print(F("\nSending val "));
  Serial.print(mensaje);
  Serial.print("...");
  if (! evento.publish(mensaje)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
    publicado = true;
  }

  return publicado;
}
