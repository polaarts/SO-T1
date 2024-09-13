#!/bin/bash

# Compilar amurrarse.c
gcc -o bin/amurrarse src/amurrarse.c

# Iniciar el observer en segundo plano y redirigir su salida a un archivo de log
./bin/observer > observer.log 2>&1 &

# Guardar el PID del observer
OBSERVER_PID=$!

# Iniciar el programa principal
./bin/players

# Después de que el programa principal termine, detener el observer
kill $OBSERVER_PID

