//Implement a C function called ex3 such that it achieves 
//the same functionality as the machine code of objs/ex3_sol.o
//Note: you need to figure out ex3()'s function signature yourself; 
//the signature is not void ex3()
  
#include <assert.h>

void
ex3(char* x, char *y, int z) {
	for(int i=0; i<z;i++)
	{
		char temp = x[i];
		x[i] = y[i];
		y[i] = temp;
	}
}
