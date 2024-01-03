#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

/*STRUCTS*/
struct Cliente {
    int id;
    int estado; // 0: Esperando | 1: En cajero | 2: atendido
};

/*VARIABLES GLOBALES */
char rutaArchivoLog[20] = "programa.log";
FILE *archivoLog;
// Lista de clientes
struct Cliente *clientes[20];
pthread_mutex_t mutexLog;
pthread_mutex_t mutexListaClientes;
pthread_cond_t condicionInteractuarReponedor;
pthread_mutex_t mutexInteractuarReponedor;


/* Declaración de funciones*/
void crearNuevoCliente(int s);

void *cajero(void *arg);

void *cliente(void *arg);

void *reponedor(void *arg);

void writeLogMessage(char *id, char *msg);

int main(int argc, char *argv[]) {
    // Crear sigaction para crear clientes
    struct sigaction sigCrearCliente;
    sigCrearCliente.sa_handler = crearNuevoCliente;

    if (sigaction(SIGUSR1, &sigCrearCliente, NULL) == -1) {
        perror("Error en sigaction sigCrearCliente");
        exit(-1);
    }

    // Inicializar recursos
    if (pthread_mutex_init(&mutexInteractuarReponedor, NULL) != 0) {
        perror("Error al inicializar mutexInteractuarReponedor");
        exit(-1);
    }
    if (pthread_mutex_init(&mutexListaClientes, NULL) != 0) {
        perror("Error al inicializar mutexListaClientes");
        exit(-1);
    }
    if (pthread_mutex_init(&mutexLog, NULL) != 0) {
        perror("Error al inicializar mutexLog");
        exit(-1);
    }

    if (pthread_cond_init(&condicionInteractuarReponedor, NULL) != 0) {
        perror("Error al inicializar condicionInteractuarReponedor");
        exit(-1);
    }

    printf("Programa en ejecución. PID: %d\n", getpid());
    printf("Añadir cliente: kill -10 %d\n", getpid());

    // Crear fichero log
    archivoLog = fopen(rutaArchivoLog, "w");
    fclose(archivoLog);

    // Inicializar lista de clientes
    for (int i = 0; i < 20; i++) {
        clientes[i] = malloc(sizeof(struct Cliente));
        if (clientes[i] == NULL) {
            printf("Error: ¡No se pudo asignar memoria!\n");
            exit(1);
        }
    }


    // Crear 3 hilos cajero
    pthread_t hiloCajero1, hiloCajero2, hiloCajero3;
    pthread_t hiloReponedor;
    pthread_create(&hiloCajero1, NULL, cajero, NULL);
    pthread_create(&hiloCajero2, NULL, cajero, NULL);
    pthread_create(&hiloCajero3, NULL, cajero, NULL);
    // Crear 1 hilo reponedor
    pthread_create(&hiloReponedor, NULL, reponedor, NULL);

    /* Esperar señales*/
    fflush(stdout);
    while (1) {
        pause();
    }

    return 0;
}

void crearNuevoCliente(int s) {

    printf("Llamada a crearNuevoCliente\n");

    // Buscamos una posición vacía en la lista de clientes
    int posicion = -1;
    pthread_mutex_lock(&mutexListaClientes);
    for (int i = 0; i < 20; i++) {
        if (clientes[i]->estado == 0) {
            posicion = i;
            break;
        }
    }

    // Si hay hueco en la lista de clientes
    if (posicion != -1) {
        pthread_mutex_lock(&mutexLog);
        writeLogMessage("Cliente", "Entra en el supermercado.");
        pthread_mutex_unlock(&mutexLog);
        printf("El cliente %d entra en el supermercado.\n", posicion + 1);

        // Creamos un nuevo hilo cliente
        pthread_t hiloCliente;
        struct Cliente *nuevoCliente = malloc(sizeof(struct Cliente));
        if (nuevoCliente == NULL) {
            printf("Error: ¡No se pudo asignar memoria!\n");
            exit(1);
        }
        nuevoCliente->id = posicion + 1;
        nuevoCliente->estado = 0;
        pthread_create(&hiloCliente, NULL, cliente, (void *) nuevoCliente);

        // Rellenamos los datos del cliente (id, estado=0)
        clientes[posicion] = nuevoCliente;
    } else {
        // Ignoramos la llamada si no hay hueco en la lista de clientes
        printf("El supermercado está lleno. Cliente se va sin entrar.\n");
    }
    pthread_mutex_unlock(&mutexListaClientes);

}


void *cajero(void *arg) {}

void *cliente(void *arg) {}

void *reponedor(void *arg) {

    while (1) {
        // Esperar a que algún cajero me avise
        pause();


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


void writeLogMessage(char *id, char *msg) {
// Calculamos la hora actual
    time_t now = time(0);
    struct tm *tlocal = localtime(&now);
    char stnow[25];
    strftime(stnow, 25, " %d/ %m/ %y %H: %M: %S ", tlocal);
// Escribimos en el log
    archivoLog = fopen(rutaArchivoLog, "a");
    fprintf(archivoLog, "[ %s ] %s : %s \n ", stnow, id, msg);
    fclose(archivoLog);
}
