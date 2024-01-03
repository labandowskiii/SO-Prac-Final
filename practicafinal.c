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

struct Cajero {
    int id;
    int numClientesAtendidos;
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
        clientes[i] = NULL;
    }


    // Crear 3 hilos cajero
    pthread_t hiloCajero1, hiloCajero2, hiloCajero3;
    struct Cajero *cajero1 = malloc(sizeof(struct Cajero));
    struct Cajero *cajero2 = malloc(sizeof(struct Cajero));
    struct Cajero *cajero3 = malloc(sizeof(struct Cajero));
    pthread_create(&hiloCajero1, NULL, cajero, (void *) cajero1);
    pthread_create(&hiloCajero2, NULL, cajero, (void *) cajero2);
    pthread_create(&hiloCajero3, NULL, cajero, (void *) cajero3);

    // Crear 1 hilo reponedor
    pthread_t hiloReponedor;
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
        if (clientes[i] == NULL) {
            clientes[i] = malloc(sizeof(struct Cliente));
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


void *cajero(void *arg) {

    struct Cajero *cajero = (struct Cajero *) arg;

    // Buscamos el cliente con menor id
    while (1) {
        int posicion = -1;
        pthread_mutex_lock(&mutexListaClientes);
        for (int i = 0; i < 20; i++) {
            if (clientes[i] != NULL && clientes[i]->estado == 0) {
                posicion = i;
                // Cambiamos el estado del cliente a 1
                clientes[posicion]->estado = 1;
                break;
            }
        }
        pthread_mutex_unlock(&mutexListaClientes);

        // Si hay clientes en la lista
        if (posicion != -1) {


            // Cambiamos el estado del cliente a 1
            pthread_mutex_lock(&mutexListaClientes);
            clientes[posicion]->estado = 1;
            pthread_mutex_unlock(&mutexListaClientes);

            // Calculamos el tiempo de atención (aleatorio entre 1 y 5 segundos)
            int tiempoTrabajo = rand() % 5 + 1;

            // Escribimos la hora de la atención de la compra
            printf("Cajero %d: Atendiendo al cliente %d.\n", (int) pthread_self(), posicion + 1);
            pthread_mutex_lock(&mutexLog);
            writeLogMessage("Cajero", "Atendiendo al cliente.");
            pthread_mutex_unlock(&mutexLog);

            // Esperamos el tiempo de atención
            sleep(tiempoTrabajo);

            // Generamos un numero aleatorio entre 1 y 100
            int numeroAleatorio = rand() % 100 + 1;
            if (71 <= numeroAleatorio && numeroAleatorio <= 95) {
                // Avisamos al reponedor y esperamos a que vuelva
                printf("Cajero %d: Llamando al reponedor.\n", (int) pthread_self());
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", "Llamando al reponedor.");
                pthread_mutex_unlock(&mutexLog);

                pthread_mutex_lock(&mutexInteractuarReponedor);
                pthread_cond_signal(&condicionInteractuarReponedor);
                pthread_mutex_unlock(&mutexInteractuarReponedor);

                pthread_mutex_lock(&mutexInteractuarReponedor);
                pthread_cond_wait(&condicionInteractuarReponedor, &mutexInteractuarReponedor);
                pthread_mutex_unlock(&mutexInteractuarReponedor);

                printf("Cajero %d: Reponedor ha terminado.\n", (int) pthread_self());
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", "Reponedor ha terminado.");
                pthread_mutex_unlock(&mutexLog);

                // Imprimir precio compra
                char buffer[40];
                sprintf(buffer, "Precio compra de Cliente %d: %d.\n", posicion + 1,
                        numeroAleatorio);
                printf("Cajero %d: %s", (int) pthread_self(), buffer);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", buffer);
                pthread_mutex_unlock(&mutexLog);
                sleep(10);
            } else if (96 <= numeroAleatorio && numeroAleatorio <= 100) {
                // El cliente tiene algun problema y no se puede completar la compra
                printf("Cajero %d: Error en el cliente, no se puede realizar la compra.\n", (int) pthread_self());
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", "Error en el cajero.");
                pthread_mutex_unlock(&mutexLog);
                sleep(10);
            } else {
                // El cliente ha terminado la compra sin imprevistos
                printf("Cajero %d: Cliente %d ha terminado la compra.\n", (int) pthread_self(), posicion + 1);
                char buffer[40];
                sprintf(buffer, "Precio compra de Cliente %d: %d.\n", posicion + 1,
                        numeroAleatorio);
                printf("Cajero %d: %s", (int) pthread_self(), buffer);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", "Cliente ha terminado la compra.");
                writeLogMessage("Cajero", buffer);
                pthread_mutex_unlock(&mutexLog);
            }

            // Cambiamos el estado del cliente a 2
            pthread_mutex_lock(&mutexListaClientes);
            clientes[posicion]->estado = 2;
            pthread_mutex_unlock(&mutexListaClientes);

            // Aumentamos el número de clientes atendidos
            cajero->numClientesAtendidos++;

            // Si se ha atendido a 20 clientes, el cajero descansa 20 segundos
            if (cajero->numClientesAtendidos == 20) {
                printf("Cajero %d: He atendido 20 clientes, descansando.\n", (int) pthread_self());
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", "He atendido 20 clientes, descansando.");
                pthread_mutex_unlock(&mutexLog);
                sleep(20);
                cajero->numClientesAtendidos = 0;
                printf("Cajero %d: Descanso terminado.\n", (int) pthread_self());
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", "Descanso terminado.");
                pthread_mutex_unlock(&mutexLog);
            }
        }
    }

}

void *cliente(void *arg) {}

void *reponedor(void *arg) {


    while (1) {
        // Esperar a que algún cajero me avise
        pthread_mutex_lock(&mutexInteractuarReponedor);
        pthread_cond_wait(&condicionInteractuarReponedor, &mutexInteractuarReponedor);
        pthread_mutex_unlock(&mutexInteractuarReponedor);

        printf("Reponedor: Recibí una llamada para verificar un precio.\n");
        pthread_mutex_lock(&mutexLog);
        writeLogMessage("Reponedor", "Recibí una llamada para verificar un precio.");
        pthread_mutex_unlock(&mutexLog);

        // Simular tiempo de trabajo (aleatorio entre 1 y 5 segundos)
        int tiempoTrabajo = rand() % 5 + 1;
        sleep(tiempoTrabajo);

        // Avisar de que ha terminado el reponedor
        printf("Reponedor: Verificación de precio completada.\n");
        pthread_mutex_lock(&mutexLog);
        writeLogMessage("Reponedor", "Verificación de precio completada.");
        pthread_mutex_unlock(&mutexLog);

        pthread_mutex_lock(&mutexInteractuarReponedor);
        pthread_cond_signal(&condicionInteractuarReponedor);
        pthread_mutex_unlock(&mutexInteractuarReponedor);
    }
    return NULL;
}


void writeLogMessage(char *id, char *msg) {
// Calculamos la hora actual
    time_t now = time(0);
    struct tm *tlocal = localtime(&now);
    char stnow[25];
    strftime(stnow, 25, "%d/ %m/ %y %H: %M: %S", tlocal);
// Escribimos en el log
    archivoLog = fopen(rutaArchivoLog, "a");
    fprintf(archivoLog, "[%s] %s : %s \n ", stnow, id, msg);
    fclose(archivoLog);
}
