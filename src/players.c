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

#include "observer.h"

#define myfifo "/tmp/myfifo"

// Función que el jugador ejecutará antes de ser eliminado
void ejecutar_amurrarse() {
    execl("./bin/amurrarse", "amurrarse", NULL);  // Ejecutar el archivo "amurrarse"
    perror("Error al ejecutar 'amurrarse'");
    exit(1);  // Si exec falla, el proceso debe terminar
}

// Handler para la señal que indica al jugador que ejecute 'amurrarse'
void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        ejecutar_amurrarse();
    }
}

// Mantener al jugador activo hasta recibir la señal para ejecutar 'amurrarse'
void jugador_activo() {
    signal(SIGUSR1, signal_handler);  // Registrar el handler para SIGUSR1
    while (1) {
        pause();  // Esperar señal
    }
}

// Función que elimina al jugador cuyo PID está almacenado en memoria compartida
void eliminar_jugador(int pids[], int* ctd_players) {
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    char* str = (char*)shmat(shmid, (void*)0, 0);

    // Convertir la cadena de vuelta a entero (índice del proceso a eliminar)
    int indice = atoi(str);

    printf("Se eliminó el jugador %d\n", pids[indice]);


    // Verificar que el índice esté dentro de los límites
    if (indice < 0 || indice >= *ctd_players) {
        printf("Error: índice inválido %d para eliminar el proceso.\n", indice);
        shmdt(str);
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
        pids[i] = pids[i + 1];  // Desplazar los elementos hacia la izquierda
    }
    (*ctd_players)--;  // Reducir el número de jugadores

    // Desasociar y eliminar la memoria compartida
    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);  // Eliminar la memoria compartida
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
        votos[i] = (rand() % size) + 1;  // Asegurarse de que los votos estén dentro del rango de jugadores
        printf("El jugador %d votó por el jugador nro. %d\n", pids[i], votos[i]);
    }

    // Contar la frecuencia de cada voto
    int frecuencia[size+1];  // El índice 0 no se usa, por eso size+1
    for (int i = 0; i <= size; i++) {
        frecuencia[i] = 0;
    }

    // Contar cuántos votos recibió cada jugador
    for (int i = 0; i < size; i++) {
        frecuencia[votos[i]]++;
    }

    // Determinar el mayor número de votos
    int max_votos = 0;
    int jugador_max_votos = 0;
    for (int i = 1; i <= size; i++) {
        if (frecuencia[i] > max_votos) {
            max_votos = frecuencia[i];
            jugador_max_votos = i;
        }
    }

    printf("......\n");
    // Anunciar el resultado
    printf("Se votó %d veces por el jugador nro. %d\n", max_votos, jugador_max_votos);
}


// Función que envía los votos a través del FIFO
void send_votes(int votes[], int size) {
    int fd;

    fd = open(myfifo, O_WRONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    write(fd, votes, size * sizeof(int));
    printf(".................\n");
    printf("¡VOTOS ENVIADOS!\n");
    printf(".................\n");
    close(fd);
}

// Función principal
int main(int argc, char *argv[]) {
    int ctd_players;
    
    // Pedir al usuario el número de jugadores
    printf("Ingresa el número de jugadores: ");
    scanf("%d", &ctd_players);

    // Asignar memoria dinámicamente para los arreglos de PIDs y votos
    int *pids = malloc(ctd_players * sizeof(int));
    int *votos = malloc(ctd_players * sizeof(int));

    srand(time(NULL));  // Inicializar la semilla aleatoria

    // Eliminar cualquier FIFO previo y crear uno nuevo
    unlink(myfifo);
    if (mkfifo(myfifo, 0666) == -1) {
        perror("mkfifo failed");
        exit(1);
    }

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
        int tiempo_espera = (rand() % 5) + 1;  // Generar un valor entre 1 y 5
        printf("---------------------------------------------------------------------\n");
        printf("Esperando %d segundos antes de la ronda de votación...\n", tiempo_espera);
        printf("---------------------------------------------------------------------\n");
        sleep(tiempo_espera);  // Pausa aleatoria antes de la ronda de votación
        printf("\n");
        // Crear un proceso hijo para el observer
        // Enviar votos al observer y eliminar jugador
        pid_t pid_observer = fork();
        if (pid_observer == -1) {
            perror("fork failed");
            exit(1);
        } else if (pid_observer == 0) {
            observer(ctd_players);  // Pasar la cantidad de jugadores al observer
            exit(0);
        } else {
            // Generar y enviar los votos después de la pausa
            generar_votos(pids, ctd_players, votos);
            send_votes(votos, ctd_players);

            // Esperar a que el observer termine
            wait(NULL);

            // Leer el índice del jugador a eliminar desde la memoria compartida
            key_t key = ftok("shmfile", 65);
            int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
            char* str = (char*)shmat(shmid, (void*)0, 0);
            int jugador_eliminar = atoi(str);

            // Eliminar el jugador basado en el índice recibido
            eliminar_jugador(pids, &ctd_players);

            // Desasociar y eliminar la memoria compartida
            shmdt(str);
            shmctl(shmid, IPC_RMID, NULL);  // Eliminar la memoria compartida

            // Mostrar la lista actualizada de jugadores
            printf("\n");
            printf("---------------------------------------------------------------------\n");
            printf("Lista actualizada de jugadores:\n");
            printf("................................\n");
            for (int i = 0; i < ctd_players; i++) {
                printf("Jugador %d con PID %d\n", i+1, pids[i]);
            }
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

    return 0;
}
