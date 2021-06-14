import requests
import json
import statistics

TOKEN = "b88ea74d42178303a2a76c2a167e7d17007fc1131513cacb9e3c935dfa1c2e2e"

url = 'https://api.esios.ree.es/indicators/10230'
headers = {'Accept':'application/json; application/vnd.esios-api-v2+json','Content-Type':'application/json','Host':'api.esios.ree.es','Authorization':'Token token=' + TOKEN}


def div10(n): 
    return round(n/10, 2) 


def obtener_hora_mas_barata():
        response = requests.get(url, headers=headers)
        #print (response)
        if response.status_code  == 401: print ("no acreditado\n")
        if response.status_code == 200:print ("OK\n")
            
        json_data = json.loads(response.text)
        #print (json_data)
        valores = json_data['indicator']['values']

        precios = [x['value'] for x in valores]

        #guardamos los precios redondeados al segundo dígito y en centimos de euro
        #precioscent = map(div10, precios) 
        precioscent = [div10(x) for x in precios]
        for i, precio in enumerate(precioscent):
            print('{0:^10} {1:>10.2f}'.format(str(i).zfill(2), precio))

        valor_min = min(precioscent[0:10]) #Solo comprueba entre las 00 y las 10 AM
        valor_max = max(precioscent)
        valor_med = round(statistics.mean(precioscent),2)
        print("-----------------------------")
        print("Precio mínimo: %s" % str(valor_min))
        print("Precio máximo: %s" % str(valor_max))
        print("Precio medio: %s" % str(valor_med))


        hora_mas_barata = precioscent.index(valor_min)

        print("Hora mas barata: " + str(hora_mas_barata))

        return hora_mas_barata, valor_min

if __name__ == '__main__':
        hora_mas_barata, precio_min = obtener_hora_mas_barata()
        
        file_dest = "./hora"
        
        with open(file_dest, "w") as file:
                hora_precio = "{hora_mas_barata};{precio_min}"
        
                file.write(hora_precio)
