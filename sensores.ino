//Tabla lectura vs luz

const int MEDIDAS_TABLA_LUZ = 4;
int tabla_lectura_luz[4][2] = {{50, 0}, {225, 600}, {823, 950}, {985, 1024}}; 


int media_movil(void){
  int medida = tomar_medida();

  int luz_real;
  int i = 0;

  if(medida < tabla_lectura_luz[0][0]){
    luz_real = tabla_lectura_luz[0][1];
  }
  else if(medida > tabla_lectura_luz[MEDIDAS_TABLA_LUZ - 1][0]){
    luz_real = tabla_lectura_luz[MEDIDAS_TABLA_LUZ - 1][1];
  }
  
  else{
    while(tabla_lectura_luz[i][0] < medida) i++;

    int distancia_ldr = tabla_lectura_luz[i+1][0] - tabla_lectura_luz[i][0];
    int distancia_luz = tabla_lectura_luz[i+1][1] - tabla_lectura_luz[i][1];

    float proporcion = (float) distancia_luz / distancia_ldr;

    luz_real = medida*proporcion;
  }
  
  return luz_real;
}

int tomar_medida(void){
  int tiempo_inicial = millis();
  int tiempo_final = millis();
  int num_medidas = 0;

  int medidas_luz[NUM_MEDIDAS];

  while(tiempo_final - tiempo_inicial < 10*NUM_MEDIDAS){
    medidas_luz[num_medidas] = lee_sensor(SENSORLUZ);
    num_medidas++;

    delay(100);
    tiempo_final = millis();
  }
  
  int media = calcular_media(medidas_luz);

  return media;
}

int calcular_media(int medidas_luz[NUM_MEDIDAS]){
  int total_luz = 0;

  for(int j = 0; j < NUM_MEDIDAS; j++){
    total_luz += medidas_luz[j];
  }

  int media_luz = total_luz / NUM_MEDIDAS;

  return media_luz;
}


int lee_sensor(int tipo){
  int lectura = analogRead(tipo);
  return lectura;
}

void muestra_sensores(int lectura_luz){
  float lux;
  char texto[50];

  sprintf(texto, "Lectura fotoresistencia: %d", lectura_luz);
  Serial.println(texto);
}
