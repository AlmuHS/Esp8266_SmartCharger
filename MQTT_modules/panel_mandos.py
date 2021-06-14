import ssl
import sys


import paho.mqtt.client

def on_connect(client, userdata, flags, rc):
        print('connected (%s)' % client._client_id)
        client.subscribe(topic='usuario/cargador/estado_coche', qos=2)
        client.subscribe(topic='usuario/sensor/coche/carga', qos=2)
        client.subscribe(topic='usuario/cargador/hora_inicio', qos=2)
        client.subscribe(topic='usuario/cargador/hora_fin', qos=2)
        client.subscribe(topic='usuario/cargador/coche_aparcado/potencia_acum', qos=2)
        client.subscribe(topic='usuario/control/precios', qos=2)
        
def on_message(client, userdata, message):
        print('------------------------------')
        print('topic: %s' % message.topic)
        print('payload: %s' % message.payload)
        print('qos: %d' % message.qos)
    
def main():
        client = paho.mqtt.client.Client(client_id='usuario', clean_session=False)
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(host='localhost', port=1883)
        client.loop_forever()
                
if __name__ == '__main__':
	main()

sys.exit(0)
