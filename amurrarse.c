#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

void amurrarse(){
    /*
    Imprime en pantalla el amurre ANTES de ser eliminado.

    *Deberia recibir la SIGNAL para ejecutarse. Y hacer exec
    */
    char message[] = "Estoy amurrado :(";

}