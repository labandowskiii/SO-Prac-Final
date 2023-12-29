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

    printf("Programa en ejecución. PID: %d\n",getpid());
    printf("Añadir cliente: kill -10 %d\n",getpid());
    // TODO: Crear fichero log
    // TODO: Inicializar lista de clientes

    // TODO: Crear 3 hilos cajero
    // TODO: Crear 1 hilo reponedor

    /* Esperar señales*/
    fflush(stdout);
    while (1){
        pause();
    }

    return 0;
}

void crearNuevoCliente (int s){

    printf("Llamada a crearNuevoCliente\n");
   

    // Buscamos una posición vacía en la lista de clientes
    int posicion = -1;
    pthread_mutex_lock(&mutexListaClientes);
    for (int i = 0; i < 20; i++) {
        if (listaClientes[i].estado == -1) {
            posicion = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutexListaClientes);

    // Si hay hueco en la lista de clientes
    if (posicion != -1) {
        // Creamos un nuevo hilo cliente
        pthread_t hiloCliente;
        nuevoCliente->id = posicion + 1;
        nuevoCliente->estado = 0;
        pthread_create(&hiloCliente, NULL, accionesClientes, (void *)nuevoCliente);

        // Rellenamos los datos del cliente (id, estado=0)
        pthread_mutex_lock(&mutexListaClientes);
        listaClientes[posicion] = *nuevoCliente;
        pthread_mutex_unlock(&mutexListaClientes);
    } else {
        // Ignoramos la llamada si no hay hueco en la lista de clientes
        printf("El supermercado está lleno. Cliente se va sin entrar.\n");
    }

    return NULL;
}

}

void *cajero (void *arg){}

void *cliente(void *arg){}

void *reponedor(void *arg){

while (1) {
        // Esperar a que algún cajero me avise
        pthread_mutex_lock(&mutexInteractuarReponedor);
        pthread_mutex_unlock(&mutexInteractuarReponedor);

        // Lógica de trabajo del reponedor
        printf("Reponedor: Recibí una llamada para verificar un precio.\n");
        writeLogMessage("Reponedor", "Recibí una llamada para verificar un precio.");

        // Simular tiempo de trabajo (aleatorio entre 1 y 5 segundos)
        int tiempoTrabajo = rand() % 5 + 1;
        sleep(tiempoTrabajo);

        // Avisar de que ha terminado el reponedor
        printf("Reponedor: Verificación de precio completada.\n");
        writeLogMessage("Reponedor", "Verificación de precio completada.");
    }
    return NULL;
}
}

void writeLogMessage ( char * id , char * msg ) {
// Calculamos la hora actual
    time_t now = time (0) ;
    struct tm * tlocal = localtime (& now ) ;
    char stnow [25];
    strftime ( stnow , 25 , " %d/ %m/ %y %H: %M: %S " , tlocal ) ;
// Escribimos en el log
    archivoLog = fopen ( rutaArchivoLog , " a " ) ;
    fprintf ( archivoLog , "[ %s ] %s : %s \n " , stnow , id , msg ) ;
    fclose ( archivoLog ) ;
}
