#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
/*STRUCTS*/
struct  Cliente{
    int clienteId;
    int estado; // 0: Esperando | 1: En cajero | 2: atendido
};

/*VARIABLES GLOBALES */
char rutaArchivoLog[20]="programa.log";
FILE *archivoLog;
// Lista de clientes
struct Cliente *clientes[20];
pthread_mutex_t mutexLog;
pthread_mutex_t mutexListaClientes;
pthread_cond_t condicionInteractuarReponedor;
pthread_mutex_t mutexInteractuarReponedor;


/* Declaración de funciones*/
void crearNuevoCliente (int s);
void *cajero (void *arg);
void *cliente(void *arg);
void *reponedor(void *arg);
void writeLogMessage ( char * id , char * msg );

int main(int argc, char *argv[]){
    // Crear sigaction para crear clientes
    struct sigaction sigCrearCliente;
    sigCrearCliente.sa_handler=crearNuevoCliente;

    if (sigaction(SIGUSR1,&sigCrearCliente,NULL)==-1){
        perror("Error en sigaction sigCrearCliente");
        exit(-1);
    }

    // Inicializar recursos
    if(pthread_mutex_init(&mutexInteractuarReponedor,NULL)!=0){
        perror("Error al inicializar mutexInteractuarReponedor");
        exit(-1);
    }
    if(pthread_mutex_init(&mutexListaClientes,NULL)!=0){
        perror("Error al inicializar mutexListaClientes");
        exit(-1);
    }
    if(pthread_mutex_init(&mutexLog,NULL)!=0){
        perror("Error al inicializar mutexLog");
        exit(-1);
    }

    if(pthread_cond_init(&condicionInteractuarReponedor,NULL)!=0){
        perror("Error al inicializar condicionInteractuarReponedor");
        exit(-1);
    }

    // TODO: Crear fichero log
    // TODO: Inicializar lista de clientes

    // TODO: Crear 3 hilos cajero
    // TODO: Crear 1 hilo reponedor

    /* Esperar señales*/
    while (1){
        pause();
    }

    return 0;
}

void crearNuevoCliente (int s){}

void *cajero (void *arg){}

void *cliente(void *arg){}

void *reponedor(void *arg){}

void writeLogMessage ( char * id , char * msg ) {
// Calculamos la hora actual
    time_t now = time (0) ;
    struct tm * tlocal = localtime (& now ) ;
    char stnow [25];
    strftime ( stnow , 25 , " %d/ %m/ %y %H: %M: %S " , tlocal ) ;
// Escribimos en el log
    archivoLog = fopen ( rutaArchivoLog , " a " ) ;
    fprintf ( archivoLog , "[ %s ] %s : %s \ n " , stnow , id , msg ) ;
    fclose ( archivoLog ) ;
}