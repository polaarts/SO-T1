#include <unistd.h>
#include <signal.h>
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

// Función que el jugador ejecutará antes de ser eliminado
void ejecutar_amurrarse() {
    execl("./bin/amurrarse", "amurrarse", NULL);
    perror("Error al ejecutar 'amurrarse'");
    exit(1);
}

// Handler para la señal que indica al jugador que ejecute 'amurrarse'
void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        ejecutar_amurrarse();
    }
}

// Mantener al jugador activo hasta recibir la señal para ejecutar 'amurrarse'
void jugador_activo() {
    signal(SIGUSR1, signal_handler);
    while (1) {
        pause();
    }
}

// Función que elimina al jugador cuyo índice es pasado como argumento
void eliminar_jugador(int pids[], int* ctd_players, int indice) {
    printf("Se eliminó el jugador %d\n", pids[indice]);

    // Verificar que el índice esté dentro de los límites
    if (indice < 0 || indice >= *ctd_players) {
        printf("Error: índice inválido %d para eliminar el proceso.\n", indice);
        return;
    }

    int pid_from_shared_memory = pids[indice];

    // Enviar señal al proceso para que ejecute 'amurrarse'
    if (kill(pid_from_shared_memory, SIGUSR1) == -1) {
        perror("Error al intentar enviar la señal al proceso");
        exit(1);
    }

    // Esperar unos segundos para que el proceso termine de ejecutar 'amurrarse'
    sleep(2);

    // Eliminar el proceso después de que ejecutó 'amurrarse'
    if (kill(pid_from_shared_memory, SIGKILL) == -1) {
        perror("Error al intentar eliminar el proceso");
        exit(1);
    }

    // Eliminar el PID del array pids y ajustar el array de jugadores
    for (int i = indice; i < *ctd_players - 1; i++) {
        pids[i] = pids[i + 1];
    }
    (*ctd_players)--;
}

// Función que genera votos aleatorios
void generar_votos(int pids[], int size, int votos[]) {
    printf("Generando votos\n");
    printf(".................\n");

    // Inicializamos los votos a 0
    for (int i = 0; i < size; i++) {
        votos[i] = 0;
    }

    // Generar votos en un rango válido
    for (int i = 0; i < size; i++) {
        votos[i] = (rand() % size) + 1;
        printf("El jugador %d votó por el jugador nro. %d\n", pids[i], votos[i]);
    }

    printf("......\n");
}

// Función que envía los votos a través del FIFO
void send_votes(int votes[], int size) {
    int fd;

    fd = open(myfifo, O_WRONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    // Escribir el tamaño primero
    if (write(fd, &size, sizeof(int)) == -1) {
        perror("write size failed");
        exit(1);
    }

    // Escribir los votos
    if (write(fd, votes, size * sizeof(int)) == -1) {
        perror("write votes failed");
        exit(1);
    }

    printf(".................\n");
    printf("¡VOTOS ENVIADOS!\n");
    printf(".................\n");
    close(fd);
}

// Función principal
int main(int argc, char *argv[]) {
    int ctd_players;

    // Configurar memoria compartida
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(shared_data_t), 0666 | IPC_CREAT);
    shared_data_t* shared_data = (shared_data_t*)shmat(shmid, NULL, 0);
    shared_data->result_ready = 0;

    // Pedir al usuario el número de jugadores
    printf("Ingresa el número de jugadores: ");
    scanf("%d", &ctd_players);

    // Asignar memoria dinámicamente para los arreglos de PIDs y votos
    int *pids = malloc(ctd_players * sizeof(int));
    int *votos = malloc(ctd_players * sizeof(int));

    srand(time(NULL));  // Inicializar la semilla aleatoria

    // Crear los jugadores
    for (int i = 0; i < ctd_players; i++) {
        pid_t pid = fork();
        if (pid > 0) {
            pids[i] = pid;  // Guardar el PID del hijo
        } else if (pid == 0) {
            jugador_activo();  // Mantener al proceso hijo activo
            exit(0);
        } else {
            perror("fork failed");
            exit(1);
        }
    }

    // Imprimir los PIDs de los procesos hijos
    printf("\n");
    printf("PLAYERS:\n\n");
    for (int j = 0; j < ctd_players; j++) {
        printf("Jugador %d con PID %d\n", j, pids[j]);
    }
    printf("\n\n");

    // Bucle de eliminación hasta que quede solo un jugador
    while (ctd_players > 1) {
        // Generar un tiempo de espera aleatorio entre 1 y 5 segundos antes de votar
        int tiempo_espera = (rand() % 5) + 1;
        printf("---------------------------------------------------------------------\n");
        printf("Esperando %d segundos antes de la ronda de votación...\n", tiempo_espera);
        printf("---------------------------------------------------------------------\n");
        sleep(tiempo_espera);
        printf("\n");

        // Generar y enviar los votos después de la pausa
        generar_votos(pids, ctd_players, votos);
        send_votes(votos, ctd_players);

        // Esperar a que el observer procese los votos
        while (shared_data->result_ready == 0) {
            usleep(10000); // Espera de 10 ms
        }

        // Leer el índice del jugador a eliminar desde la memoria compartida
        int jugador_eliminar = shared_data->player_to_eliminate;

        // Restablecer la bandera
        shared_data->result_ready = 0;

        // Eliminar el jugador basado en el índice recibido
        eliminar_jugador(pids, &ctd_players, jugador_eliminar - 1);  // Ajuste de índice

        // Mostrar la lista actualizada de jugadores
        printf("\n");
        printf("---------------------------------------------------------------------\n");
        printf("Lista actualizada de jugadores:\n");
        printf("................................\n");
        for (int i = 0; i < ctd_players; i++) {
            printf("Jugador %d con PID %d\n", i+1, pids[i]);
        }
    }

    // Proclamar al ganador
    if (ctd_players == 1) {
        printf("El jugador con PID %d es el ganador!\n", pids[0]);
        printf("---------------------------------------------------------------------\n");

        // Enviar señal al proceso ganador para que ejecute 'amurrarse'
        printf("Eliminando al jugador ganador (PID %d)...\n", pids[0]);
        if (kill(pids[0], SIGKILL) == -1) {
            perror("Error al intentar enviar la señal al proceso ganador");
            exit(1);
        }

        // Esperar unos segundos para que el proceso termine de ejecutar 'amurrarse'
        sleep(2);

        // Eliminar el proceso ganador
        if (kill(pids[0], SIGKILL) == -1) {
            perror("Error al intentar eliminar el proceso ganador");
            exit(1);
        }

        printf("El proceso ganador ha sido eliminado.\n");
    }

    // Eliminar el FIFO
    unlink(myfifo);

    // Liberar la memoria dinámica
    free(pids);
    free(votos);

    // Desasociar y eliminar la memoria compartida
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
