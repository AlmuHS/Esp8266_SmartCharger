#include <NTPClient.h>

//Create a ntp cliente via udp
WiFiUDP ntpUDP;

// Establece un cliente NTP para la hora española, aplicando un offset de 7200 minutos sobre la hora UTC
NTPClient timeClient(ntpUDP, "hora.roa.es", 7200);

// Descompone una hora dada en formato HH:MM en hora y minuto
void descomponer_hora_minuto(char hora_minuto_str[6], int hora_minuto[2]){
  int hora = (hora_minuto_str[0] - 48) * 10;
      hora += (hora_minuto_str[1] - 48);

  int minuto = (hora_minuto_str[3] - 48) * 10;
      minuto += (hora_minuto_str[4] - 48);
  
  hora_minuto[0] = hora;
  hora_minuto[1] = minuto;
}

// Obtiene la hora actual mediante una llamada NTP, y la muestra en formato HH:MM
void mostrar_hora_actual(void){
  timeClient.update();
  char hourminutes[15] = "";

  int hour_time, minute_time;
  hour_time = timeClient.getHours();
  minute_time = timeClient.getMinutes();
  
  snprintf(hourminutes, 15, "%02d:%02d", hour_time, minute_time);
  Serial.println(hourminutes);
}
