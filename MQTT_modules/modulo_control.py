import ssl
import sys


import paho.mqtt.client
import datetime
import time

potencia_instantanea = -1
carga_actual_vehiculo = -1
tiempo_carga = -1
tamano_bateria = 4

def leer_hora_mas_barata():
        hora_mas_barata = "22:25"
        precio = "0.25"

        with open("./hora") as fichero_hora:
                datos = fichero_hora.read()
                hora_precio = datos.split(";")
                
                hora_mas_barata = hora_precio[0]
                precio = hora_precio[1]

        return hora_mas_barata, precio

def sumar_hora(hora_inicial, minutos_anadir):
        formato = "%H:%M"
        h1 = datetime.datetime.strptime(hora_inicial, formato)
        minutos = datetime.timedelta(minutes=minutos_anadir)
        
        resultado = h1 + minutos
        resultado_format = str(resultado.strftime("%H:%M"))
        
        return resultado_format

def on_connect(client, userdata, flags, rc):
        print('connected (%s)' % client._client_id)
        client.subscribe(topic='usuario/control/iniciar_parar_carga', qos=2)
        client.subscribe(topic='usuario/cargador/coche_aparcado/potencia_acum', qos=2)
        client.subscribe(topic='usuario/cargador/potencia_instantanea', qos=2)
        client.subscribe(topic='usuario/cargador/potencia_cargador', qos=2)
        client.subscribe(topic='usuario/sensor/coche/capacidad', qos=2)
        
def on_message(client, userdata, message):
        print('------------------------------')
        print('topic: %s' % message.topic)
        print('payload: %s' % message.payload)
        print('qos: %d' % message.qos)

        if(message.topic == 'usuario/control/iniciar_parar_carga'):
                client.publish('usuario/cargador/orden_carga', message.payload)
                
        elif (message.topic == 'usuario/cargador/potencia_instantanea'):
                potencia_instantanea = int(message.payload)
                
        elif (message.topic == 'usuario/cargador/coche_aparcado/potencia_acum'):
                carga_actual_vehiculo = float(message.payload)
                
        elif (message.topic == 'usuario/sensor/coche/capacidad'):
                tamano_bateria = int(message.payload)
def main():
        hora_mas_barata, precio_min = leer_hora_mas_barata()

        print(f"La hora mas barata es " + hora_mas_barata)

        client = paho.mqtt.client.Client(client_id='usuario', clean_session=False)
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(host='127.0.0.1', port=1883)
        
        client.loop_start()
        
        carga_programada = False
        
        while True:
                if (carga_actual_vehiculo < tamano_bateria) and (tamano_bateria > 0) and (not carga_programada):
                        tiempo_carga = (tamano_bateria - carga_actual_vehiculo) / 4 #Tiempo de carga en segundos
                        minutos_carga = tiempo_carga / 60
                
                        hora_final = sumar_hora(hora_mas_barata, minutos_carga)
                
                        client.publish('usuario/cargador/hora_inicio', hora_mas_barata)
                        client.publish('usuario/cargador/hora_fin', hora_final)
                        client.publish('usuario/control/precios', precio_min)

                        print(f'programa carga con inicio {hora_mas_barata} y fin {hora_final}')
        
                        carga_programada = True
        
                time.sleep(1)
        
if __name__ == '__main__':
	main()

sys.exit(0)
