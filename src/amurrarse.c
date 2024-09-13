#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <stdio.h>
#include <unistd.h>

int main() {
    printf("El jugador del PID %d está amurrado\n", getpid());
    fflush(stdout); // Asegurar que el mensaje se imprima inmediatamente
    sleep(1); // Pausa para permitir que el mensaje se muestre antes de que el proceso termine
    return 0;
}
