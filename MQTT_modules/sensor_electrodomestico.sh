
CONSUMO=$((1 + $RANDOM % 4))

mosquitto_pub -h localhost -t usuario/sensor/electrodomestico -m "$CONSUMO"
