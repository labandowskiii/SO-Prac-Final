#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

/*STRUCTS*/
struct Cliente
{
    int id;
    int estado;         // 0: Esperando | 1: En cajero | 2: atendido
    time_t horaEntrada; // variable para controlar los 10 segundos antes de irse
    int seVa;           // simulador de 10% de irse
};

struct Cajero
{
    int id;
    int numClientesAtendidos;
    int numClientesAtendidosGlobal;
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
struct Cajero cajero1;
struct Cajero cajero2;
struct Cajero cajero3;
pthread_t hiloCajero1, hiloCajero2, hiloCajero3, hiloReponedor;
int clientesC1 = 0, clientesC2 = 0, clientesC3 = 0;

/* Declaración de funciones*/
void crearNuevoCliente(int s);

void *cajero(void *arg);

void *cliente(void *arg);

void *reponedor(void *arg);

void *verificarTiempoCliente(void *arg);

void terminarPrograma(int s);

void writeLogMessage(char *persona, int id, char *msg);

int main(int argc, char *argv[])
{
    // Crear sigaction para crear clientes
    struct sigaction sigCrearCliente;
    sigCrearCliente.sa_handler = crearNuevoCliente;

    if (sigaction(SIGUSR1, &sigCrearCliente, NULL) == -1)
    {
        perror("Error en sigaction sigCrearCliente");
        exit(-1);
    }

    // Inicializar recursos
    if (pthread_mutex_init(&mutexInteractuarReponedor, NULL) != 0)
    {
        perror("Error al inicializar mutexInteractuarReponedor");
        exit(-1);
    }
    if (pthread_mutex_init(&mutexListaClientes, NULL) != 0)
    {
        perror("Error al inicializar mutexListaClientes");
        exit(-1);
    }
    if (pthread_mutex_init(&mutexLog, NULL) != 0)
    {
        perror("Error al inicializar mutexLog");
        exit(-1);
    }

    if (pthread_cond_init(&condicionInteractuarReponedor, NULL) != 0)
    {
        perror("Error al inicializar condicionInteractuarReponedor");
        exit(-1);
    }

    // Crear fichero log
    archivoLog = fopen(rutaArchivoLog, "w");
    fclose(archivoLog);

    // Inicializar lista de clientes
    for (int i = 0; i < 20; i++)
    {
        clientes[i] = malloc(sizeof(struct Cliente));
        clientes[i]->estado = 2;
        clientes[i]->id = i + 1;
    }

    // Crear 3 hilos cajero
    struct Cajero *cajero1 = malloc(sizeof(struct Cajero));
    cajero1->id = 1;
    struct Cajero *cajero2 = malloc(sizeof(struct Cajero));
    cajero2->id = 2;
    struct Cajero *cajero3 = malloc(sizeof(struct Cajero));
    cajero3->id = 3;
    // pthread_t hiloCajero1, hiloCajero2, hiloCajero3;
    pthread_create(&hiloCajero1, NULL, cajero, (void *)cajero1);
    pthread_create(&hiloCajero2, NULL, cajero, (void *)cajero2);
    pthread_create(&hiloCajero3, NULL, cajero, (void *)cajero3);

    // Crear 1 hilo reponedor
    // pthread_t hiloReponedor;
    pthread_create(&hiloReponedor, NULL, reponedor, NULL);

    struct sigaction sigFinalPrograma;
    sigFinalPrograma.sa_handler = terminarPrograma;

    if (sigaction(SIGINT, &sigFinalPrograma, NULL) == -1)
    {
        perror("Error en sigaction sigFinalPrograma");
        exit(-1);
    }

    printf("Programa en ejecución. PID: %d\n", getpid());
    printf("Añadir cliente: kill -10 %d\n", getpid());
    printf("Terminar programa: ctrl + c\n");

    /* Esperar señales*/
    fflush(stdout);
    while (1)
    {
        pause();
    }

    return 0;
}

void crearNuevoCliente(int s)
{

    printf("Llamada a crearNuevoCliente\n");

    // Buscamos una posición vacía en la lista de clientes
    int posicion = -1;
    pthread_mutex_lock(&mutexListaClientes);
    for (int i = 0; i < 20; i++)
    {
        if (clientes[i]->estado == 2)
        {
            posicion = i;
            break;
        }
    }

    // Si hay hueco en la lista de clientes
    if (posicion != -1)
    {
        // Creamos un nuevo hilo cliente
        pthread_t hiloCliente;

        // Rellenamos los datos del cliente (id, estado=0)
        clientes[posicion]->estado = 0;
        clientes[posicion]->id = posicion + 1;
        clientes[posicion]->horaEntrada = time(NULL);
        int probabilidad = rand() % 100;
        clientes[posicion]->seVa = probabilidad < 10; // boolean del 10%

        pthread_create(&hiloCliente, NULL, cliente, (void *)clientes[posicion]);
    }
    else
    {
        // Ignoramos la llamada si no hay hueco en la lista de clientes
        printf("El supermercado está lleno. Cliente se va sin entrar.\n");
    }
    pthread_mutex_unlock(&mutexListaClientes);
}

void *cajero(void *arg)
{

    struct Cajero *cajero = (struct Cajero *)arg;
    // signal (SIGUSR2, terminarHilos);
    //  Buscamos el cliente con menor id
    while (1)
    {
        pthread_testcancel();
        int posicion = -1;
        pthread_mutex_lock(&mutexListaClientes);
        for (int i = 0; i < 20; i++)
        {
            if (clientes[i]->estado == 0)
            {
                posicion = i;
                // Cambiamos el estado del cliente a 1
                clientes[posicion]->estado = 1;
                break;
            }
        }
        pthread_mutex_unlock(&mutexListaClientes);

        // Si hay clientes en la lista
        if (posicion != -1)
        {

            // Cambiamos el estado del cliente a 1
            pthread_mutex_lock(&mutexListaClientes);
            clientes[posicion]->estado = 1;
            pthread_mutex_unlock(&mutexListaClientes);

            // Calculamos el tiempo de atención (aleatorio entre 1 y 5 segundos)
            int tiempoTrabajo = rand() % 5 + 1;

            // Escribimos la hora de la atención de la compra
            char buffer[40];
            sprintf(buffer, "Atendiendo al Cliente %d.", clientes[posicion]->id);
            printf("Cajero %d: %s\n", cajero->id, buffer);
            pthread_mutex_lock(&mutexLog);
            writeLogMessage("Cajero", cajero->id, buffer);
            pthread_mutex_unlock(&mutexLog);

            // Esperamos el tiempo de atención
            sleep(tiempoTrabajo);

            // Generamos un numero aleatorio entre 1 y 100
            int numeroAleatorio = rand() % 100 + 1;
            printf("Numero aleatorio: %d\n", numeroAleatorio);
            if (71 <= numeroAleatorio && numeroAleatorio <= 95)
            {
                // Avisamos al reponedor y esperamos a que vuelva
                printf("Cajero %d: Llamando al reponedor.\n", cajero->id);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", cajero->id, "Llamando al reponedor.");
                pthread_mutex_unlock(&mutexLog);

                pthread_mutex_lock(&mutexInteractuarReponedor);
                pthread_cond_signal(&condicionInteractuarReponedor);
                pthread_mutex_unlock(&mutexInteractuarReponedor);

                pthread_mutex_lock(&mutexInteractuarReponedor);
                pthread_cond_wait(&condicionInteractuarReponedor, &mutexInteractuarReponedor);
                pthread_mutex_unlock(&mutexInteractuarReponedor);

                printf("Cajero %d: Reponedor ha terminado.\n", cajero->id);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", cajero->id, "Reponedor ha terminado.");
                pthread_mutex_unlock(&mutexLog);

                // Imprimir precio compra
                char buffer[40];
                sprintf(buffer, "Precio compra de Cliente (%d): %d.", cajero->id,
                        numeroAleatorio);
                printf("Cajero %d: %s\n", cajero->id, buffer);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", cajero->id, buffer);
                pthread_mutex_unlock(&mutexLog);
                sleep(10);
            }
            else if (96 <= numeroAleatorio && numeroAleatorio <= 100)
            {
                // El cliente tiene algun problema y no se puede completar la compra
                printf("Cajero %d: Error en el cliente, no se puede realizar la compra.\n", cajero->id);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", cajero->id, "Error en el cajero.");
                pthread_mutex_unlock(&mutexLog);
                sleep(10);
            }
            else
            {
                // El cliente ha terminado la compra sin imprevistos
                char buffer1[40], buffer2[40];
                sprintf(buffer1, "Cliente %d ha terminado la compra", posicion + 1);
                sprintf(buffer2, "Precio compra de Cliente %d: %d.", posicion + 1,
                        numeroAleatorio);
                printf("Cajero %d: %s\n", cajero->id, buffer1);
                printf("Cajero %d: %s\n", cajero->id, buffer2);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", cajero->id, buffer1);
                writeLogMessage("Cajero", cajero->id, buffer2);
                pthread_mutex_unlock(&mutexLog);
            }

            // Cambiamos el estado del cliente a 2
            pthread_mutex_lock(&mutexListaClientes);
            clientes[posicion]->estado = 2;
            pthread_mutex_unlock(&mutexListaClientes);

            // Aumentamos el número de clientes atendidos
            cajero->numClientesAtendidos++;
            switch (cajero->id)
            {
            case 1:
                clientesC1++;
                break;
            case 2:
                clientesC2++;
                break;
            case 3:
                clientesC3++;
                break;
            }

            // Si se ha atendido a 20 clientes, el cajero descansa 20 segundos
            if (cajero->numClientesAtendidos == 20)
            {
                printf("Cajero %d: He atendido 20 clientes, descansando.\n", cajero->id);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", cajero->id, "He atendido 20 clientes, descansando.");
                pthread_mutex_unlock(&mutexLog);
                sleep(20);
                cajero->numClientesAtendidos = 0;
                printf("Cajero %d: Descanso terminado.\n", cajero->id);
                pthread_mutex_lock(&mutexLog);
                writeLogMessage("Cajero", cajero->id, "Descanso terminado.");
                pthread_mutex_unlock(&mutexLog);
            }
        }
    }
}

void *cliente(void *arg)
{
    // signal (SIGUSR2, terminarHilos);
    struct Cliente *cliente = (struct Cliente *)arg;

    // escribir en el log hora de llegada del cliente
    char buffer[100];
    time_t horaLlegada = time(NULL);
    strftime(buffer, sizeof(buffer), "[%d/%m/%y %H:%M:%S]", localtime(&horaLlegada));
    sprintf(buffer, "Entro al supermercado.", cliente->id);
    printf("Cliente(%d): %s\n", cliente->id, buffer);
    pthread_mutex_lock(&mutexLog);
    writeLogMessage("Cliente", cliente->id, buffer);
    pthread_mutex_unlock(&mutexLog);

    // Calculamos el tiempo de espera maximo
    int tiempoEspera = rand() % 10 + 1;
    int tiempoTranscurrido = 0;

    // Esperamos a que expire el tiempo de espera o que nos atienda un agente
    sleep(tiempoEspera);

    // Calcular posibilidad del 10% de irse
    int posibilidadIrse = rand() % 100 + 1;
    if (posibilidadIrse < 10)
    {
        cliente->seVa = 1;
        cliente->estado = 2;
        printf("Cliente %d: Se ha cansado de esperar y se ha ido.\n", cliente->id);
        pthread_mutex_lock(&mutexLog);
        writeLogMessage("Cliente", cliente->id, "Se ha cansado de esperar y se ha ido.");
        pthread_mutex_unlock(&mutexLog);
        exit(0);
    }

    // si un cliente nos ha atendido, esperamos a que termine
    if (cliente->estado == 0)
    {
        while (cliente->estado == 0)
        {
            usleep(1000); // evitar bucle infinito por si acaso.
        }
    }

    // log
    time_t horaFinalizacion = time(NULL);
    strftime(buffer, sizeof(buffer), "[%d/%m/%y %H:%M:%S]", localtime(&horaFinalizacion));
    sprintf(buffer, "Cliente (%d): He terminado las compras.", cliente->id);
    printf("%s\n", buffer);
    pthread_mutex_lock(&mutexLog);
    printf("Cliente %d: Ha terminado las compras.\n", cliente->id);
    writeLogMessage("Cliente", cliente->id, buffer);
    pthread_mutex_unlock(&mutexLog);

    // borrar información del cliente de la lista
    pthread_mutex_lock(&mutexListaClientes);
    cliente->estado = 2; // cliente ha sido atendido o se ha ido
    pthread_mutex_unlock(&mutexListaClientes);
}

void *reponedor(void *arg)
{

    // signal (SIGUSR2, terminarHilos);
    while (1)
    {
        pthread_testcancel();
        // Esperar a que algún cajero me avise
        pthread_mutex_lock(&mutexInteractuarReponedor);
        pthread_cond_wait(&condicionInteractuarReponedor, &mutexInteractuarReponedor);
        pthread_mutex_unlock(&mutexInteractuarReponedor);

        printf("Reponedor: Recibí una llamada para verificar un precio.\n");
        pthread_mutex_lock(&mutexLog);
        writeLogMessage("Reponedor", 1, "Recibí una llamada para verificar un precio.");
        pthread_mutex_unlock(&mutexLog);

        // Simular tiempo de trabajo (aleatorio entre 1 y 5 segundos)
        int tiempoTrabajo = rand() % 5 + 1;
        sleep(tiempoTrabajo);

        // Avisar de que ha terminado el reponedor
        printf("Reponedor: Verificación de precio completada.\n");
        pthread_mutex_lock(&mutexLog);
        writeLogMessage("Reponedor", 1, "Verificación de precio completada.");
        pthread_mutex_unlock(&mutexLog);

        pthread_mutex_lock(&mutexInteractuarReponedor);
        pthread_cond_signal(&condicionInteractuarReponedor);
        pthread_mutex_unlock(&mutexInteractuarReponedor);
    }
    return NULL;
}

void *verificarTiempoCliente(void *arg)
{
    // signal (SIGUSR2, terminarHilos);
    while (1)
    { // se ejecuta todo el programa
        pthread_testcancel();
        for (int i = 0; i < 20; i++)
        {
            if (clientes[i]->estado == 0)
            { // cliente esperando
                if (time(NULL) - clientes[i]->horaEntrada >= 10)
                { // ha esperado más de 10 segundos
                    if (clientes[i]->seVa)
                    {
                        clientes[i]->estado = 2; // señalizamos que el cliente se ha ido
                        printf("Cliente %d: Se ha cansado de esperar y se ha ido.\n", clientes[i]->id);
                        pthread_mutex_lock(&mutexLog);
                        writeLogMessage("Cliente", clientes[i]->id, "Se ha cansado de esperar y se ha ido.");
                        pthread_mutex_unlock(&mutexLog);
                    }
                }
            }
        }
    }
}

void terminarPrograma(int s)
{
    pthread_cancel(hiloCajero1);
    pthread_cancel(hiloCajero2);
    pthread_cancel(hiloCajero3);
    pthread_cancel(hiloReponedor);
    printf("\nCajero1 ha atendido a %d clientes\n", clientesC1);
    printf("Cajero2 ha atendido a %d clientes\n", clientesC2);
    printf("Cajero3 ha atendido a %d clientes\n", clientesC3);
    printf("Programa terminado.\n");
    pthread_mutex_lock(&mutexLog);
    for (int i = 0; i < 3; i++)
    {
        char buffer[100];
        switch (i)
        {
        case 0:
            sprintf(buffer, "He atendido %d clientes", clientesC1);
            writeLogMessage("Cajero", 1, buffer);
            break;
        case 1:
            sprintf(buffer, "He atendido %d clientes", clientesC2);
            writeLogMessage("Cajero", 2, buffer);
            break;
        case 2:
            sprintf(buffer, "He atendido %d clientes", clientesC3);
            writeLogMessage("Cajero", 3, buffer);
            break;
        }
    }
    pthread_mutex_unlock(&mutexLog);
    exit(0);
}

void writeLogMessage(char *persona, int id, char *msg)
{
    // Calculamos la hora actual
    time_t now = time(0);
    struct tm *tlocal = localtime(&now);
    char stnow[25];
    strftime(stnow, 25, "%d/ %m/ %y %H: %M: %S", tlocal);
    // Escribimos en el log
    archivoLog = fopen(rutaArchivoLog, "a");
    fprintf(archivoLog, "[%s] %s (%d): %s \n", stnow, persona, id, msg);
    fclose(archivoLog);
}
