#include <stdio.h>
#include <pthread.h>


/*STRCUTS*/
struct  Cliente{
    int clienteId;
    int estado; // 0: Esperando | 1: En cajero | 2: atendido
};

/*VARIABLES GLOBALES */
char rutaArchivoLog[20]="programa.log";
FILE *fichero;
// Lista de clientes
struct Cliente *clientes;
pthread_mutex_t mutexLog;
pthread_mutex_t mutexListaClientes;
pthread_cond_t condicionInteractuarReponedor;
pthread_mutex_t mutexInteractuarReponedor;



int main(int argc, char *argv[]){
    return 0;
}
