// ParProg_Lab6.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "mpi.h"
#include <iostream>
#include <chrono>

using namespace std;

struct InputParams
{
	double from;
	double to;
	int divCount;
};

void Quest();

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	Quest();

	MPI_Finalize();

	return 0;
}

double Integraing(const InputParams &params, double(*fromFunc)(double x));

InputParams ReadUserInput();


void Quest()
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	InputParams input;

	MPI_Datatype InputParams_MPI;

	int length[3] = { 1,1,1 };

	MPI_Aint offsets[3] =
	{
		offsetof(InputParams, from),
		offsetof(InputParams, to),
		offsetof(InputParams, divCount)
	};

	MPI_Datatype types[3] = { MPI_DOUBLE, MPI_DOUBLE, MPI_INT };

	MPI_Type_create_struct(3, length, offsets, types, &InputParams_MPI);
	MPI_Type_commit(&InputParams_MPI);

	std::chrono::time_point<std::chrono::steady_clock> begin;

	if (rank == 0)
	{
		input = ReadUserInput();

		begin = chrono::high_resolution_clock::now();

		auto stepByProcess = (input.to - input.from) / size;

		input.to = input.from + stepByProcess;
		input.divCount;

		for (size_t i = 1; i < size; i++)
		{
			InputParams inputForOther;
			inputForOther.from = input.from + stepByProcess * i;
			inputForOther.to = input.from + stepByProcess * (i + 1);
			inputForOther.divCount = input.divCount;
			MPI_Send(&inputForOther, 1, InputParams_MPI, i, 1488, MPI_COMM_WORLD);
		}
	}
	else
	{
		MPI_Status status;
		MPI_Recv(&input, 1, InputParams_MPI, 0, 1488, MPI_COMM_WORLD, &status);
	}

	double sum = Integraing(input, [](double x) {return cos(x); });
	
	if (rank == 0)
	{
		MPI_Status status;
		for (size_t i = 1; i < size; i++)
		{
			double tempSum;
			MPI_Recv(&tempSum, 1, MPI_DOUBLE, i, 1489, MPI_COMM_WORLD, &status);
			sum += tempSum;
		}

		auto end = chrono::high_resolution_clock::now();

		std::cout << "Result: " << sum << endl <<
			"Time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << std::endl;

	}
	else
	{
		MPI_Send(&sum, 1, MPI_DOUBLE, 0, 1489, MPI_COMM_WORLD);
	}
}


double Integraing(const InputParams &params, double(*fromFunc)(double x))
{
	double sum = 0;
	double step = (params.to - params.from) / params.divCount;
	for (auto i = 0; i < params.divCount; i++)
	{
		auto x1 = params.from + step * i;
		auto x2 = params.from + step * (i + 1);
		sum += fromFunc((x1 + x2) / 2.0) * (x2 - x1);
	}
	return sum;
}

InputParams ReadUserInput()
{
	InputParams input;
	cout << "Integrate from:" << endl;
	cin >> input.from;
	cout << "to:" << endl;
	cin >> input.to;
	cout << "Divides per process" << endl;
	cin >> input.divCount;
	return input;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
