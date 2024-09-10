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
    int pid_index_eliminated;

    // Aquí deberías agregar la lógica para contar los votos y manejar el empate

    return pid_index_eliminated;
}

void receive_votes(int votes[], int size){   
    int fd;

     // Eliminar el FIFO si ya existe
    unlink(myfifo);
    
    // Verificar si ya existe el FIFO
    if (mkfifo(myfifo, 0666) == -1) {
        perror("mkfifo failed");
    }

    // Abrir el FIFO para leer
    printf("Abriendo FIFO para leer\n");
    fd = open(myfifo, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

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

int observer(int argc, char *argv[]){
    /*
    Acá se debe incluir la lógica de WAIT, EXIT
    para controlar el flujo del juego
    */
   int votes[4];

   printf("OBSERVADOR: %d\n", getpid()); // PID del observador
   
   // Llamar a receive_votes para recibir los votos
   receive_votes(votes, 4);  // Asegúrate de pasar el tamaño correcto del array

   // Ahora puedes llamar a la función termination para eliminar un proceso
   int pid_index = termination(votes);
   printf("Eliminando proceso con índice: %d\n", pid_index);

   return 0;
}
