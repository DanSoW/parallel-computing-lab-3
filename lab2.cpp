#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include "mpi.h"
using namespace std;

#define TAG_N 1
#define TAG_A 2 
#define TAG_S 3

#define SIZE_STR 8
#define SIZE_STR_DOUBLE 23

/* Функция получения целочисленного значения из входного потока */
int getIntValue(bool zero, const char* text = "Введите размер массива : ") {
	printf(text);

	size_t length = 0;
	char* end = NULL;
	char buf[SIZE_STR] = "";
	int value = 0;
	bool t = true;

	while ((fgets(buf, sizeof(buf), stdin)) && (t)) {
		length = strlen(buf);
		t = false;

		if (buf[length - 1] == '\n') {
			buf[--length] = '\0';

			errno = 0;
			value = strtol(buf, &end, 10);

			if (length == 0) {
				fprintf(stderr, "\nНеобходимо ввести данные.\n");
				t = true;
			}
			if (errno != 0 || *end != '\0') {
				fprintf(stderr, "\nПри вводе данных возникли ошибки\n");
				fprintf(stderr, "\t%s\n", buf);
				fprintf(stderr, "\t%*c\n", (int)(end - buf) + 1, '^');
				t = true;
			}
		}
		else {
			scanf_s("%*[^\n]");
			scanf_s("%*c");
			fprintf(stderr, "Возникло %d ошибок: \n", (SIZE_STR - 2));

			t = true;
		}

		if ((!t) && (value <= 0) && (!zero)) {
			printf("\nОшибка: число не может быть меньше 0\n");
			t = true;
		}

		if (t) {
			printf(text);
		}
	}

	return value;
}

/* Выполнение операции */
int Execute(int* arr, int N, int rank, int size) {// массив а, кол-во эл, № процесса, кол-во процессов
    int nt, beg, h, end, i;
    int sum = 0;            // Сумма
    nt = rank;              
    h = N / size;           // Количество элементов входящих в процесс
    beg = h * nt;           // Начало процесса

    if (nt == size - 1)
    {
        end = N - 1;
    }
    else
    {
        end = beg + h;
    }

    cout << "Номер процесса; начало массива; конец массива :" << nt << " " << beg << " " << end << "\n";
    
    for (i = beg; i <= end; i++)
    {
        if (i % 2 == 0)
        {
            sum += arr[i];
        }
    }

    return sum;
}

int main(int argc, char** argv) {
    srand(time(0));
    setlocale(LC_ALL, "Rus");

    int rank, size;
    int N;
    int* arr;

    // Статус 
    MPI_Status status;

    // Инициализация MPI
    MPI_Init(&argc, &argv);

    // Определение общего числа параллельных процессов
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Определение ранга процесса
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    cout << "Количество процессов: " << size << " ранг: " << rank << "\n";

    // Блокировка процесса
    MPI_Barrier(MPI_COMM_WORLD);

    // Запуск таймера
    double time = MPI_Wtime();

    if (rank == 0) {
        N = getIntValue(false);

        arr = new int[N];
        for (int i = 0; i < N; i++)
        {
            arr[i] = 100 + rand() % 10000000;
        }

        for (int i = 1; i < size; i++)
        {
            int val = N % size;
            MPI_Send(&val, 1, MPI_INT, i, TAG_N, MPI_COMM_WORLD);
        }

        for (int i = 1; i < size; i++)
        {
            MPI_Send(arr, N % size, MPI_INT, i, TAG_A, MPI_COMM_WORLD);
        }


        int sum = Execute(arr, N / size, rank, size);
        for (int i = 1; i < size; i++) {
            int s = 0;
            MPI_Recv(&s, 1, MPI_INT, i, TAG_S, MPI_COMM_WORLD, &status);
            cout << "status.MPI_SOURCE " << status.MPI_SOURCE << "\n";
            cout << "status.MPI_ERROR " << status.MPI_ERROR << "\n";
            cout << "status.MPI_TAG " << status.MPI_TAG << "\n";
            sum += s;
        }
        cout << "Результат: " << sum << "\n";

    }
    else {
        int N1 = 0;
        MPI_Recv(&N1, 1, MPI_INT, 0, TAG_N, MPI_COMM_WORLD, &status);
        int* a1 = new int[N1];
        MPI_Recv(a1, N1, MPI_INT, 0, TAG_A, MPI_COMM_WORLD, &status);
        int s = Execute(a1, N1, rank, size);
        cout << "Ранг: " << rank << " Результат ранга = " << s << "\n";
        MPI_Send(&s, 1, MPI_INT, 0, TAG_S, MPI_COMM_WORLD);

    }

    // Вычисление времени выполнения
    time = MPI_Wtime() - time;
    cout << "Время выполнения = " << time << "\n";

    // Остановка параллельного процесса
    MPI_Finalize();
    return 0;
}