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
    */
    int pid_index_eliminated = -1;

    // Aquí deberías agregar la lógica para contar los votos y manejar el empate

    return pid_index_eliminated;
}

void receive_votes(int votes[], int size){   
    int fd;

    printf("inicio RECEIVEEEE\n");

    // Abrir el FIFO para leer
    printf("Abriendo FIFO para leer\n");
    fd = open(myfifo, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }
    printf("FIFO abierto para lectura\n");

    // Leer los votos
    if (read(fd, votes, size * sizeof(int)) == -1) {
        perror("read failed");
    }

    printf("RECIBIDO: ");
    for (int i = 0; i < size; i++) {
        printf("%d ", votes[i]);
    }
    printf("\n");

    close(fd);
}

void observer(){
    /*
    Acá se debe incluir la lógica de WAIT, EXIT
    para controlar el flujo del juego
    */
   int votes[6]; // Asegúrate de que el tamaño coincida con ctd_players

   printf("OBSERVADOR: %d\n", getpid()); // PID del observador
    
   // Llamar a receive_votes para recibir los votos
   receive_votes(votes, 10);  // Pasa el tamaño correcto del array

   // Ahora puedes llamar a la función termination para eliminar un proceso
   int pid_index = termination(votes);
   printf("Eliminando proceso con índice: %d\n", pid_index);
}
