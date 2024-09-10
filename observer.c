#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define myfifo "/tmp/myfifo"

int termination(int votes[]){

    /*
    Debe retornar el valor del índice del array de PIDs que debe
    ser eliminado.

    input: votes[]
    output: pid
    */

    int pid_index_eliminated;

    // Contar los números iguales

    // Manejar el empate (random)


    return pid_index_eliminated;
}

void receive_votes(int votes[], int size){   
    int fd;
    mkfifo(myfifo, 0666);
    fd = open(myfifo, O_RDONLY);

    read(fd, votes, size * sizeof(int));

    printf("RECIBIDO");
    close(fd);

}

// Mandar signal del pid eliminador

int observer(int argc, char *argv[]){
    /*
    Acá se debe incluir la lógica que de WAIT, EXIT
    para controlar el flujo del juego
    */
   int votes[4];

   printf("OBSERVADOR: %d\n", getpid()); // PID del observador
   
   receive_votes(votes, sizeof(votes));
}