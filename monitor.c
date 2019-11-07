/*
Para realizar el monitor de temperaturas, escriba un programa que reciba los datos
del simulador. Dicho programa debe contener dos threads que están a la espera
de recibir datos de temperatura. Dichos threads están bloqueados hasta la llegada
de un dato que será extraído del pipe generado por el sensor de temperatura.
Cada uno de los threads tiene una prioridad diferente.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sched.h>
#include <semaphore.h>

void *main_thread_function(void *ptr);
void *secondary_thread_function(void *ptr);

pthread_t main_thread, secondary_thread;
pthread_attr_t main_attr, sec_attr;
int ret1, ret2;
struct sched_param main_param, sec_param;

char *fifo_file = "fifo_file";
int fd;
char buff[10];

sem_t sem;
float last_value = 0;

void set_priority(void)
{
    /* initialized with default attributes */
    ret1 = pthread_attr_init(&main_attr);
    ret2 = pthread_attr_init(&sec_attr);

    /* safe to get existing scheduling param */
    ret1 = pthread_attr_getschedparam(&main_attr, &main_param);
    ret2 = pthread_attr_getschedparam(&sec_attr, &sec_param);

    /* set the priority; others are unchanged */
    main_param.sched_priority = 1;
    sec_param.sched_priority = 10;

    /* setting the new scheduling param */
    ret1 = pthread_attr_setschedparam(&main_attr, &main_param);
    ret2 = pthread_attr_setschedparam(&sec_attr, &sec_param);
}

int main()
{
    // mkfifo(<pathname>,<permission>)
    mkfifo(fifo_file, 0666);

    set_priority();

    sem_init(&sem, 0, 1);

    pthread_create(&main_thread, &main_attr, main_thread_function, NULL);
    pthread_create(&secondary_thread, &sec_attr, secondary_thread_function, NULL);

    pthread_join(main_thread, NULL);
    pthread_join(secondary_thread, NULL);

    sem_destroy(&sem);
    exit(EXIT_SUCCESS);
}

void *main_thread_function(void *ptr)
{
    /*
    El thread de mayor prioridad
    es el encargado de leer la temperatura del pipe y debe verificar que la temperatura
    no exceda los 90 °C. Si la temperatura excede ese límite debe imprimir un aviso
    de la situación. Por el contrario, si la temperatura no excede el límite debe pasar el
    dato al thread de menor prioridad
    */
    float temp;
    sem_wait(&sem);
    while (1)
    {
        fd = open(fifo_file, O_RDONLY);
        read(fd, buff, 10);
        temp = strtof(buff, NULL);
        printf("MAIN\tTemp=%.2f", temp);
        if (temp > 90)
        {
            printf("\t(Mayor a 90)\n");
        }
        else
        {   
            printf("\t");
            //Pasar el valor al de menor prioridad
            last_value = temp;
            sem_post(&sem);
        }
        close(fd);
    }
}

void *secondary_thread_function(void *ptr)
{
    /*
    thread de menor prioridad (sin que este paso implique un bloqueo del thread de
    prioridad alta). El thread de menor prioridad debe informar un promedio
    de los últimos 3 valores de temperatura que obtuvo. 
    */
   float values[] = {0, 0, 0};
   char index = 0;
    while (1)
    {
        sem_wait(&sem);
        values[index] = last_value;
        index = (index+1) % 3;
        printf("SEC\tPromedio=%.2f\n", (values[0] + values[1] + values[2]) / 3);
    }
}