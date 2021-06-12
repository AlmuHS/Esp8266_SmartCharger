

extern Adafruit_MQTT_Publish aviso_carga;
extern Adafruit_MQTT_Publish coche_llegasale;
extern Adafruit_MQTT_Publish coche_aparca;
extern Adafruit_MQTT_Publish potencia_instantanea;
extern Adafruit_MQTT_Publish potencia_cargador;
extern Adafruit_MQTT_Publish tiempo_carga;
extern Adafruit_MQTT_Publish potencia_acum;

extern int tiempo_inicio_carga;
extern int potencia_acumulada;

int luz;

const int MAX_LUZ = 823;
const int MIN_LUZ = 600;

const int INICIA_CARGA = 1;
const int TERMINA_CARGA = 2;

int orden = INICIA_CARGA;

Estado::Estado(int estado_inicial){
  this->estado_inicial = estado_inicial;
}

void Estado::desconectado(void){
  
}

void Estado::coche_fuera(int luz){
  if(luz > MAX_LUZ){
      this->estado_actual = COCHE_APARCADO;
      publicar_evento(coche_llegasale, "llega coche");
    }
    else{
      publicar_evento(coche_aparca, "coche fuera");
    }
}

void Estado::cargando_tiempo(void){
    //Inicia la carga
    if(orden == INICIA_CARGA){
      this->estado_actual = CARGANDO_USUARIO;
      publicar_evento(aviso_carga, "comienza carga por orden del usuario");
     
      tiempo_inicio_carga = millis();
      potencia_acumulada = 0;
      orden = 0;
    }
    //Consideramos que el coche ha salido cuando la luz es demasiado baja
    else if(luz < MIN_LUZ){
      this->estado_actual = COCHE_FUERA;
      publicar_evento(coche_llegasale, "sale_coche");
    }
    else{
      publicar_evento(coche_aparca, "coche aparcado y desconectado");
    }
}

void Estado::cargando_usuario(void){
  if(orden == TERMINA_CARGA){
      this->estado_actual = COCHE_APARCADO;
      publicar_evento(aviso_carga, "termina carga por orden del usuario");
      orden = 0;
    }
    else if(potencia_acumulada >= programa.potencia){
      this->estado_actual = COCHE_APARCADO;
      publicar_evento(aviso_carga, "termina carga por llegar a potencia maxima");
    }
}

void Estado::coche_aparcado(void){
  //Inicia la carga
  if(orden == INICIA_CARGA){
    this->estado_actual = CARGANDO_USUARIO;
    publicar_evento(aviso_carga, "comienza carga por orden del usuario");
   
    tiempo_inicio_carga = millis();
    potencia_acumulada = 0;
    orden = 0;
  }
  //Consideramos que el coche ha salido cuando la luz es demasiado baja
  else if(luz < 600){
    this->estado_actual = COCHE_FUERA;
    publicar_evento(coche_llegasale, "sale_coche");
  }
  else{
    publicar_evento(coche_aparca, "coche aparcado y desconectado");
  }
}

int Estado::get_estado_actual(void){
  return this->estado_actual;
}

void Estado::set_estado_actual(int estado_actual){
  this->estado_actual = estado_actual;
}

void Estado::avanzar_estado(void){
  luz = lee_sensor(SENSORLUZ);

  switch(this->estado_actual){
    case DESCONECTADO:
      this->desconectado();

      break;
      
    case COCHE_APARCADO:
      this->coche_aparcado();

      break;

    case CARGANDO_TIEMPO:
      this->cargando_tiempo();

      break;

    case CARGANDO_USUARIO:
      this->cargando_usuario();

      break;

    case COCHE_FUERA:
      this->coche_fuera(luz);

      break;
  }

  if(this->estado_actual != CARGANDO_USUARIO && this->estado_actual != CARGANDO_TIEMPO){
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

      
      publicar_evento(coche_aparca, mensaje);
      publicar_evento(potencia_acum, potencia_str);

      publicar_evento(potencia_instantanea, "4");
    }

}
