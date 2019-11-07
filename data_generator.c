#include <stdio.h>
#include <stdlib.h>
#include <time.h>

FILE *file;
int i, n;
time_t t;

float temp = 88.5;

int main()
{
    file = fopen("sensor_values", "w");

    n = 1000;

    srand((unsigned)time(&t));

    for (i = 0; i < n; i++)
    {
        temp -= ((rand() - rand()) % 200) / 100.0;
        if (i != n - 1)
            fprintf(file, "%d\t%.2f\n", rand() % (5 * 1000 * 100), temp);
        else
            fprintf(file, "%d\t%.2f", rand() % 5000000, temp);
    }
    fclose(file);

    return (0);
}