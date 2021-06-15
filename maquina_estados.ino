extern Adafruit_MQTT_Publish aviso_carga;
extern Adafruit_MQTT_Publish coche_llegasale;
extern Adafruit_MQTT_Publish coche_aparca;
extern Adafruit_MQTT_Publish potencia_instantanea;
extern Adafruit_MQTT_Publish potencia_cargador;
extern Adafruit_MQTT_Publish tiempo_carga;
extern Adafruit_MQTT_Publish potencia_acum;

int potencia_acumulada = 0;

programa_carga programa;

const int MAX_LUZ = 980;
const int MIN_LUZ = 860;

enum tipo_orden{
  ESPERA = 0,
  INICIA_CARGA = 1,
  TERMINA_CARGA = 2
};

tipo_orden orden = ESPERA;

/* IMPLEMENTACION DE LA MAQUINA DE ESTADOS */
/* ******************************************* */

Estado::Estado(lista_estados estado_inicial){
  this->estado_inicial = estado_inicial;
  this->estado_actual = estado_inicial;
}

// Implementación del estado "indeterminado". Este estado se utiliza como punto de partida, para descubrir la situación inicial
void Estado::indeterminado(void){
  int luz = lee_sensor(SENSORLUZ);
  if(luz > MAX_LUZ){
    this->estado_actual = COCHE_APARCADO;
    publicar_evento(coche_llegasale, "coche aparcado");
  }
  else{
    this->estado_actual = COCHE_FUERA;
    publicar_evento(coche_aparca, "coche fuera");
  }
}

// Implementación del estado "coche fuera"
void Estado::coche_fuera(void){
  int luz = lee_sensor(SENSORLUZ);

  // Si el nivel de luz es alto, asumimos que el coche ha llegado
  if(luz > MAX_LUZ){
      this->estado_actual = COCHE_APARCADO;
      publicar_evento(coche_llegasale, "llega coche");
  }
  else{
    publicar_evento(coche_aparca, "coche fuera");
  }
}

// Implementación del estado "coche aparcado"
void Estado::coche_aparcado(void){
  int hora_actual = timeClient.getHours();
  int min_actual = timeClient.getMinutes();

  int luz = lee_sensor(SENSORLUZ);
  
  // Si el usuario da la orden de iniciar la carga, dado que el coche está presente, comenzará dicha carga
  if(orden == INICIA_CARGA){
    this->estado_actual = CARGANDO_USUARIO;
    publicar_evento(aviso_carga, "comienza carga por orden del usuario");

    // Anotamos la hora de inicio de la carga
    tiempo_inicio_carga = millis();

    // Establecemos que, al inicio, la potencia acumulada será 0
    potencia_acumulada = 0;

    // Reseteamos la orden, para que no se vuelva a ejecutar
    orden = ESPERA;
  }

  // Si el coche sigue presente, y estamos dentro del rango horario de carga programada, inicia la carga
  else if((hora_actual == programa.hora_inicio) && (min_actual == programa.min_inicio) && (luz >= MIN_LUZ)){
    this->empezar_carga_programada(); 
  }
  // Si la luz es baja, consideramos que el coche se ha ido
  else if(luz < MIN_LUZ){
    this->estado_actual = COCHE_FUERA;
    publicar_evento(coche_llegasale, "sale_coche");
  }
  else{
    publicar_evento(coche_aparca, "coche aparcado y desconectado");
  }
}

// Implementa la carga del coche
void Estado::cargar(void){
  int luz = lee_sensor(SENSORLUZ);

  // Obtiene el tiempo de carga, restando el tiempo actual (ms desde que arrancó el sistema) al tiempo de inicio de la carga
  int tiempo_carga = (millis() - tiempo_inicio_carga)/(1000*60);

  // Aumentamos la potencia acumulada por el cargador
  potencia_acumulada += 100;
  char mensaje[100];
  float potencia_kwh = (float) potencia_acumulada / 1000.0;

  // Informamos sobre el tiempo de carga y la potencia acumulada
  char potencia_str[10];
  snprintf(mensaje, 100, "coche cargando. Tiempo de carga: %d minutos. Potencia acumulada: %f", tiempo_carga, potencia_kwh);
  snprintf(potencia_str, 10, "%d", potencia_acumulada);

  publicar_evento(coche_aparca, mensaje);
  publicar_evento(potencia_acum, potencia_str);

  publicar_evento(potencia_instantanea, "4");

  if(orden == TERMINA_CARGA){
      this->estado_actual = COCHE_APARCADO;
      publicar_evento(aviso_carga, "termina carga por orden del usuario");
      orden = ESPERA;
  }
  // Si la potencia acumulada ha llegado al nivel de potencia requerida en el programador, finalizamos la carga
  else if(potencia_acumulada >= programa.potencia){
    this->estado_actual = COCHE_APARCADO;
    publicar_evento(aviso_carga, "termina carga por llegar a potencia maxima");
  }
  else if(luz < MIN_LUZ){
    this->estado_actual = COCHE_FUERA;
    publicar_evento(coche_llegasale, "sale_coche");
  }
}

// Función auxiliar para iniciar la carga programada
void Estado::empezar_carga_programada(void){
  Serial.println("Activada alarma de inicio de carga programada");
  
  this->estado_actual = CARGANDO_PROGRAMADO;
  publicar_evento(aviso_carga, "comienza carga programada por franja horaria"); 

  this->tiempo_inicio_carga = millis();
  potencia_acumulada = 0;
}

// Implementación del estado de carga programada
void Estado::cargando_programado(void){
  int hora_actual = timeClient.getHours();
  int min_actual = timeClient.getMinutes();
  
  if(programa.hora_fin == hora_actual && programa.min_fin == min_actual){
    this->terminar_carga_programada();
  }
  else{
    this->cargar();
  }
}

// Función auxiliar para terminar la carga programada
void Estado::terminar_carga_programada(void){
  Serial.println("Activada alarma de fin de carga programada");
  
  this->estado_actual = COCHE_APARCADO;
    
  publicar_evento(aviso_carga, "termina carga por final de franja programada");
}

// Implementación del estado de carga solicitada por el usuario
void Estado::cargando_usuario(void){
  this->cargar();
}

// Implementación de la máquina de estados, controlada por lista de estados
void Estado::avanzar_estado(void){
  switch(this->estado_actual){
    case INDETERMINADO:
      this->indeterminado();

      break;
      
    case COCHE_APARCADO:
      this->coche_aparcado();

      break;

    case CARGANDO_PROGRAMADO:
      this->cargando_programado();

      break;

    case CARGANDO_USUARIO:
      this->cargando_usuario();

      break;

    case COCHE_FUERA:
      this->coche_fuera();

      break;
  }
  
  if(this->estado_actual != CARGANDO_USUARIO && this->estado_actual != CARGANDO_PROGRAMADO)
    publicar_evento(potencia_instantanea, "0");
      
  publicar_evento(potencia_cargador, "4");

}
