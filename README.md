### Compilación de procesos:

- `gcc -o bin/players src/players.c`
- `gcc -o bin/observer src/observer.c`
- `gcc -o bin/amurrarse src/amurrarse.c`

### Dar permisos de ejecución al script:

`chmod +x run_game.sh`

### Ejecución de los procesos:

`./bin/run.sh`

### CHECKLIST

- **SÍ** - El juego debe funcionar desde 2 hasta N procesos
  jugadores, los N jugadores deben estar
  emparentados. Adem ́as el valor de N jugadores se
  debe introducir por consola mediante un input.

- **SÍ** - Existe un observador el cual no tiene relación
  padre-hijo con los N jugadores.

- **SÍ** - El observador recibe el voto de los N jugadores a
  través de named pipes existiendo un correcto uso de
  la comunicación entre procesos. Adicionalmente,
  este realiza el conteo de votos y anuncia al jugador
  más votado con una correcta comunicación entre
  procesos.

- **SÍ** - En cada ronda la duración de la música es aleatoria
  y se maneja con iteraciones o tiempo real.

- **SÍ** - Se maneja bien el número de sillas que se reducen
  en cada ronda.

- **SÍ** - Cuando un jugador es elegido aleatoriamente para
  abandonar el juego, este debe debe reclamar. Para
  esto se debe utilizar exec().

- **SÍ** Se manejan correctamente casos de empate a la
  hora de elegir al jugador a eliminar.

- **SÍ** - Una vez queda un único proceso jugador, este se
  considera el ganador del juego y es anunciado por
  consola. La ejecución del código termina luego de
  esto.

- **SÍ** - La compilación arroja en consola resultados
  coherentes y entendibles, logrando apreciar así la
  ejecución del juego (Considerando que la
  sincronización de procesos está realizada
  correctamente y no existen inconsistencias entre los
  procesos en cada ronda).

- **SÍ** - Todos los procesos deben ser terminados al
  momento de ser eliminados, esto también incluye al
  ganador.
