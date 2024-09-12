#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define myfifo "/tmp/myfifo"

// Función para determinar el jugador a eliminar
int termination(int votes[], int size) {
    key_t key = ftok("shmfile", 65);
    
    // shmget returns an identifier in shmid
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);

    // shmat to attach to shared memory
    char* str = (char*)shmat(shmid, (void*)0, 0);

    int maxVotos = 0;
    int frecuencia[100] = {0};  // Suponiendo que los valores de votos están entre 0 y 99.
    int maxVotados[100];  // Lista de candidatos empatados.
    int totalMaxVotados = 0;

    // Contamos la frecuencia de cada voto.
    for (int i = 0; i < size; i++) {
        frecuencia[votes[i]]++;
        if (frecuencia[votes[i]] > maxVotos) {
            maxVotos = frecuencia[votes[i]];
        }
    }

    // Identificamos los valores con la mayor cantidad de votos.
    for (int i = 0; i < 100; i++) {
        if (frecuencia[i] == maxVotos) {
            maxVotados[totalMaxVotados++] = i;
        }
    }

    // Si hay empate, retornamos uno de los empatados aleatoriamente.
    srand(time(0));
    int indiceAleatorio = rand() % totalMaxVotados;

    // Convertir el max_votes en una cadena y almacenarlo en memoria compartida
    sprintf(str, "%d", maxVotados[indiceAleatorio]);

    // Desasociar la memoria compartida del proceso
    shmdt(str);

    int decision = maxVotados[indiceAleatorio];
    return decision;
}

// Función para recibir los votos a través del FIFO
void receive_votes(int votes[], int size){   
    int fd;
    fd = open(myfifo, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    // Leer los votos
    if (read(fd, votes, size * sizeof(int)) == -1) {
        perror("read failed");
    }

    close(fd);
}

// Función observer que decide el jugador a eliminar
void observer(int size){
   int votes[20]; // Asumimos un máximo de 20 jugadores

//    printf("OBSERVADOR: %d\n", getpid()); // PID del observador
    
   // Llamar a receive_votes para recibir los votos
   receive_votes(votes, size);  // Recibir los votos

   // Ahora puedes llamar a la función termination para eliminar un proceso
   int pid_index = termination(votes, size);
}
