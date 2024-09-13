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

// Definición de la estructura para memoria compartida
typedef struct {
    int player_to_eliminate;
    int result_ready; // 0: no listo, 1: listo
} shared_data_t;

// Función para determinar el jugador a eliminar
int termination(int votes[], int size) {
    int maxVotos = 0;
    int frecuencia[100] = {0};
    int maxVotados[100];
    int totalMaxVotados = 0;

    // Contamos la frecuencia de cada voto
    for (int i = 0; i < size; i++) {
        frecuencia[votes[i]]++;
        if (frecuencia[votes[i]] > maxVotos) {
            maxVotos = frecuencia[votes[i]];
        }
    }

    // Identificamos los jugadores con la mayor cantidad de votos
    for (int i = 0; i < 100; i++) {
        if (frecuencia[i] == maxVotos) {
            maxVotados[totalMaxVotados++] = i;
        }
    }

    // Si hay empate, seleccionamos uno aleatoriamente
    srand(time(0));
    int indiceAleatorio = rand() % totalMaxVotados;

    int decision = maxVotados[indiceAleatorio];
    return decision;
}

// Función para recibir los votos a través del FIFO
void receive_votes(int votes[], int* size){   
    int fd;
    fd = open(myfifo, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    // Leer el tamaño primero
    if (read(fd, size, sizeof(int)) == -1) {
        perror("read size failed");
        exit(1);
    }

    // Leer los votos
    if (read(fd, votes, (*size) * sizeof(int)) == -1) {
        perror("read votes failed");
        exit(1);
    }

    close(fd);
}

// Función principal del observer
int main() {
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(shared_data_t), 0666 | IPC_CREAT);
    shared_data_t* shared_data = (shared_data_t*)shmat(shmid, NULL, 0);

    // Aseguramos que el FIFO exista
    unlink(myfifo);
    if (mkfifo(myfifo, 0666) == -1) {
        perror("mkfifo failed");
    }

    while (1) {
        // Esperar hasta que result_ready sea 0
        while (shared_data->result_ready == 1) {
            usleep(10000); // Espera de 10 ms
        }

        int votes[20];
        int size;

        // Recibir votos
        receive_votes(votes, &size);

        // Procesar votos
        int jugador_eliminar = termination(votes, size);

        // Escribir resultado en memoria compartida
        shared_data->player_to_eliminate = jugador_eliminar;
        shared_data->result_ready = 1;
    }

    // Limpieza
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
