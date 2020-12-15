#include "pch.h"
#include <iostream>
#include <string>

// 9)    F[i]=A[i] *B[i] +C[i] -D[i] , i=1...8.

void calculations(__int8 *mass1, __int8 *mass2, __int8 *mass3, __int16 *mass4) {
	__int8 A[8] = {};
	__int8 B[8] = {};
	__int8 C[8] = {};
	__int16 D[8] = {};

	for (int i = 0; i < 8; i++) {
		A[i] = *(mass1 + i);
		B[i] = *(mass2 + i);
		C[i] = *(mass3 + i);
		D[i] = *(mass4 + i);
	}

	__int16 mass_res[8] = { 0,0,0,0,0,0,0,0 };

	__asm {
		//A * B
		movq mm0, [A]
		movq mm1, [B]
		pmullw mm0, mm1


		movq mm7, mm0

		movq mm0, mm7
		pcmpgtb mm2, mm0
		punpcklbw mm0, mm2
		movq mm6, mm0

		movq mm0, mm7
		pcmpgtb mm2, mm0
		punpckhbw mm2, mm0
		movq mm5, mm0
		mm5 - high b, mm6 - low b of multi AB

		//C
		movq mm0, [C]
		pcmpgtb mm2, mm0
		punpcklbw mm0, mm2
		movq mm4, mm0

		movq mm0, [C]
		pcmpgtb mm2, mm0
		punpckhbw mm0, mm2
		movq mm3, mm0
		//mm3 - high b, mm4 - low b of C

		//D
		movq mm2, [D]
		movq mm1, [D + 2 * 4]
		//mm1 - high b, mm2 - low b of D

		//A*B + C
		paddsw mm4, mm6
		paddsw mm3, mm5

		//A*B + C - D
		psubb mm2, mm4
		psubb mm1, mm3

		movq [mass_res], mm2
		movq [mass_res + 2 * 4], mm1
	}

	__int16 correct[8];
	for (int i = 0; i < 8; i++) {
		correct[i] = A[i] * B[i] + C[i] - D[i];
		std::cout << "Test" << "\n";
		std::cout << "Expected answer: " << correct[i] << "\n";
		std::cout << "My answer: " << mass_res[i] << "\n";
		if (correct[i] == mass_res[i]) {
			std::cout << "Passed\n\n";
		}
		else {
			std::cout << "Failed\n\n";
		}
	}
}

int main() {
	__int8 A[8] = { 1,2,3,4,5,6,7,8 };
	__int8 B[8] = { 9,10,11,12,13,14,15,16 };
	__int8 C[8] = { 4, 4, 4, 2, 7, -6, -3, 2 };
	__int16 D[8] = { 9, 3, 6, 4, 4, 3, 2, 5 };
	calculations(A, B, C, D);
	return 0;
}
