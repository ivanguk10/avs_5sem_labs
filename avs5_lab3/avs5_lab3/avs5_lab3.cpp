#include <iostream>
#include <stdio.h>
#include <omp.h>
#include <ctime>
#include <vector>
#define N 100000

void work() {
	int i = 0;

	int a[N];
	int b[N];

	int res = 0;

	int res2 = 0;
	clock_t start = clock();
	srand(time(0));
	for (i = 0; i < N; i++) {
		a[i] = rand();
		b[i] = rand();
	}
	for (int i = 0; i < N; i++) {
		res2 += a[i] * b[i];
	}
	clock_t end = clock();

	std::cout << "Strategy = casual" << ", millisec = " << (end - start) / (CLOCKS_PER_SEC / 1000) << std::endl;
	start = clock();
#pragma opm parallel for reduction(+:res)
	for (i = 0; i < N; i++) {
		res += a[i] * b[i];
	}

	end = clock();
	std::cout << "Strategy = omp" << ", millisec = " << (end - start) / (CLOCKS_PER_SEC / 1000) << std::endl;
	std::cout << res << std::endl;
	std::cout << res2;
}

int main() {
	work();
	return 0;
}