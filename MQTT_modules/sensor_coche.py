import ssl
import sys

import time
import paho.mqtt.client

carga_actual_vehiculo = -1
tamano_bateria = 40

def on_connect(client, userdata, flags, rc):
        print('connected (%s)' % client._client_id)
        client.subscribe(topic='usuario/cargador/coche_aparcado/potencia_acum', qos=2)
        
def on_message(client, userdata, message):
        print('------------------------------')
        print('topic: %s' % message.topic)
        print('payload: %s' % message.payload)
        print('qos: %d' % message.qos)

        if(message.topic == 'usuario/cargador/coche_aparcado/potencia_acum'):
                potencia_acumulada = int(message.payload)
                carga_actual_vehiculo = (tamano_bateria - potencia_acumulada) * 100 / tamano_bateria
        
    
def main():
        client = paho.mqtt.client.Client(client_id='usuario', clean_session=False)
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(host='localhost', port=1883)
        
        client.loop_start()
        
        while True:
                if carga_actual_vehiculo != -1:
                        client.publish('usuario/sensor/coche/carga', str(carga_actual_vehiculo))
                        
                client.publish('usuario/sensor/coche/capacidad', tamano_bateria)
                
                time.sleep(1)
                
if __name__ == '__main__':
	main()

sys.exit(0)
