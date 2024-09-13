#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define myfifo "/tmp/myfifo"

// Definición de la estructura para memoria compartida
typedef struct {
    int player_to_eliminate;
    int result_ready; // 0: no listo, 1: listo
} shared_data_t;

// Códigos de color para la terminal
#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[1;31m"
#define GREEN_COLOR "\033[1;32m"
#define YELLOW_COLOR "\033[1;33m"
#define CYAN_COLOR "\033[1;36m"
#define MAGENTA_COLOR "\033[1;35m"

// Función que el jugador ejecutará antes de ser eliminado
void ejecutar_amurrarse() {
    execl("./bin/amurrarse", "amurrarse", NULL);
    perror("Error al ejecutar 'amurrarse'");
    exit(1);  // Si exec falla, el proceso debe terminar
}

// Handler para la señal que indica al jugador que ejecute 'amurrarse'
void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        ejecutar_amurrarse();
    } else if (sig == SIGTERM) {
        // El ganador recibirá SIGTERM y saldrá sin ejecutar 'amurrarse'
        printf(GREEN_COLOR "¡¡Jugador %d ha terminado el juego como ganador!!\n" RESET_COLOR, getpid());
        exit(0);
    }
}

// Mantener al jugador activo hasta recibir la señal para ejecutar 'amurrarse'
void jugador_activo() {
    signal(SIGUSR1, signal_handler);
    signal(SIGTERM, signal_handler);  // Manejar SIGTERM para el ganador
    while (1) {
        pause();
    }
}

// Función que elimina al jugador cuyo índice es pasado como argumento
void eliminar_jugador(int pids[], int* ctd_players, int indice) {
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
    sleep(2); // Asegura que el proceso tenga tiempo de ejecutar 'amurrarse'

    // Eliminar el proceso después de que ejecutó 'amurrarse'
    if (kill(pid_from_shared_memory, SIGKILL) == -1) {
        perror("Error al intentar eliminar el proceso");
        exit(1);
    }

    // Imprimir el mensaje después de eliminar el proceso
    printf(RED_COLOR "\nSe eliminó al jugador con PID %d\n" RESET_COLOR, pid_from_shared_memory);

    // Eliminar el PID del array pids y ajustar el array de jugadores
    for (int i = indice; i < *ctd_players - 1; i++) {
        pids[i] = pids[i + 1];
    }
    (*ctd_players)--;
}


// Función que genera votos aleatorios
void generar_votos(int pids[], int size, int votos[]) {
    printf(YELLOW_COLOR "Generando votos...\n" RESET_COLOR);
    printf("...........................................................\n");

    // Generar votos en un rango válido
    for (int i = 0; i < size; i++) {
        votos[i] = rand() % size; // Índices de jugadores de 0 a size-1
        printf("Jugador con PID %d votó por el jugador %d\n", pids[i], votos[i]);
    }

    printf("...........................................................\n");
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

    printf("\n¡Votos enviados al observer!\n\n");
    close(fd);
}

// Función principal
int main(int argc, char *argv[]) {
    int ctd_players;

    // Configurar memoria compartida
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(shared_data_t), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    shared_data_t* shared_data = (shared_data_t*)shmat(shmid, NULL, 0);
    if (shared_data == (void*) -1) {
        perror("shmat failed");
        exit(1);
    }
    shared_data->result_ready = 0;

    // Pedir al usuario el número de jugadores
    printf(CYAN_COLOR "Ingresa el número de jugadores: " RESET_COLOR);
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
    printf(MAGENTA_COLOR "==================== JUGADORES INICIADOS ====================\n" RESET_COLOR);
    for (int j = 0; j < ctd_players; j++) {
        printf("Jugador %d con PID %d\n", j, pids[j]);
    }
    printf(MAGENTA_COLOR "============================================================\n\n" RESET_COLOR);

    // Bucle de eliminación hasta que quede solo un jugador
    while (ctd_players > 1) {
        // Generar un tiempo de espera aleatorio entre 1 y 5 segundos antes de votar
        int tiempo_espera = (rand() % 5) + 1;
        printf(YELLOW_COLOR "Esperando %d segundos antes de la ronda de votación...\n" RESET_COLOR, tiempo_espera);
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
        eliminar_jugador(pids, &ctd_players, jugador_eliminar);

        // Mostrar la lista actualizada de jugadores
        printf("\n");
        printf(MAGENTA_COLOR "================== LISTA ACTUALIZADA DE JUGADORES ==================\n" RESET_COLOR);
        for (int i = 0; i < ctd_players; i++) {
            printf("Jugador %d con PID %d\n", i, pids[i]);
        }
        printf(MAGENTA_COLOR "====================================================================\n\n" RESET_COLOR);
    }

    // Proclamar al ganador
    if (ctd_players == 1) {

        // Enviar señal al proceso ganador para que termine sin 'amurrarse'
        if (kill(pids[0], SIGTERM) == -1) {
            perror("Error al intentar enviar la señal al proceso ganador");
            exit(1);
        }

        // Esperar un momento para que el proceso ganador termine
        sleep(1);

        // Verificar si el proceso sigue existiendo
        if (kill(pids[0], 0) == 0) {
            // Si el proceso sigue vivo, forzar su terminación
            if (kill(pids[0], SIGKILL) == -1) {
                perror("Error al intentar eliminar el proceso ganador");
                exit(1);
            }
            printf("\nEl proceso ganador ha sido eliminado.\n");
        } else {
            printf("El proceso ganador ha terminado correctamente.\n");
        }
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
