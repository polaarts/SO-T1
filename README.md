### Compilación de procesos:

`gcc src/players.c src/observer.c -o bin/out`

### Ejecución de los procesos:

- Procesos jugadores: `./bin/players`
- Proceso observador: `./bin/observer`

### CHECKLIST

- **SI** - El juego debe funcionar desde 2 hasta N procesos
  jugadores, los N jugadores deben estar
  emparentados. Adem ́as el valor de N jugadores se
  debe introducir por consola mediante un input.

- **SI** - Existe un observador el cual no tiene relación
  padre-hijo con los N jugadores.

- **FALTA** - El observador recibe el voto de los N jugadores a
  través de named pipes existiendo un correcto uso de
  la comunicación entre procesos. Adicionalmente,
  este realiza el conteo de votos y anuncia al jugador
  más votado con una correcta comunicación entre
  procesos.

- **NO** - En cada ronda la duración de la música es aleatoria
  y se maneja con iteraciones o tiempo real.

- **NO** - Se maneja bien el número de sillas que se reducen
  en cada ronda.

- **NO** - Cuando un jugador es elegido aleatoriamente para
  abandonar el juego, este debe debe reclamar. Para
  esto se debe utilizar exec().

- **NO** Se manejan correctamente casos de empate a la
  hora de elegir al jugador a eliminar.

- **NO** - Una vez queda un único proceso jugador, este se
  considera el ganador del juego y es anunciado por
  consola. La ejecuci ́on del c ́odigo termina luego de
  esto.

- **FALTA** - La compilación arroja en consola resultados
  coherentes y entendibles, logrando apreciar así la
  ejecución del juego (Considerando que la
  sincronización de procesos está realizada
  correctamente y no existen inconsistencias entre los
  procesos en cada ronda).

- **NO** - Todos los procesos deben ser terminados al
  momento de ser eliminados, esto también incluye al
  ganador.
