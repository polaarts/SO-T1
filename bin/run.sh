#!/bin/bash

# Iniciar el observer en segundo plano y redirigir su salida a un archivo de log
./bin/observer > observer.log 2>&1 &

# Guardar el PID del observer
OBSERVER_PID=$!

echo "Observer iniciado con PID $OBSERVER_PID"

# Dar un breve tiempo para que el observer se inicie
sleep 1

# Iniciar el programa principal
./bin/players

# Después de que el programa principal termine, detener el observer
kill $OBSERVER_PID

echo "Observer detenido"
