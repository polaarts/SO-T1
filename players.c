#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "observer.h"

#define myfifo "/tmp/myfifo"

void generar_votos(int pids[], int size, int votos[]) {
    printf("generando\n");
    // Generar votos aleatorios para cada PID
    for (int i = 0; i < size; i++) {
        votos[i] = (rand() % size) + 1;  // Genera un voto entre 1 y el tamaño del array
        printf("%i\n", votos[i]);    
    }
}

void send_votes(int votes[], int size) {
    printf("enviando\n");
    int fd;

    // Eliminar el FIFO si ya existe
    unlink(myfifo);

    // Crear el FIFO
    mkfifo(myfifo, 0666);

    // Abrir el FIFO para escribir
    printf("Abriendo FIFO para escribir\n");
    fd = open(myfifo, O_WRONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }
    printf("FIFO abierto\n");

    // Escribir los votos en el FIFO
    write(fd, votes, size * sizeof(int));
    printf("fifo escrita\n");
    close(fd);
}

void sig_terminated(){
    /*
    Debe ejecutar la eliminación del respectivo 
    hijo dado el retorno del observer
     */
}

int main(int argc, char *argv[]) {
    int ctd_players = 6;
    int pids[ctd_players];
    int votos[ctd_players];

    // Crear los jugadores
    for (int i = 0; i < ctd_players; i++) {
        pid_t pid = fork();
        if (pid > 0) {
            pids[i] = pid;  // Guardar el PID del hijo
        } else if (pid == 0) {
            printf("i%i: proceso hijo %d\n", i, getpid());
            exit(0);
        } else {
            perror("fork failed");
            exit(1);
        }
    }

    // El padre espera que todos los hijos terminen
    for (int i = 0; i < ctd_players; i++) {
        wait(NULL);
    }

    // Imprimir los PIDs de los procesos hijos
    printf("PLAYERS. PIDs de los procesos hijos:\n");
    for (int j = 0; j < ctd_players; j++) {
        printf("Proceso hijo %d terminado\n", pids[j]);
    }

    // Iniciar el observador antes de enviar los votos
    observer();

    // Generar y enviar los votos
    generar_votos(pids, ctd_players, votos);

    send_votes(votos, ctd_players);

    return 0;
}
