//Implement a C function called ex4 such that it achieves 
//the same functionality as the machine code of objs/ex4_sol.o
//Note: you need to figure out ex4()'s function signature yourself; 
//the signature is not void ex4()
  
#include <assert.h>

int
ex4(int x) {
	if(x == 0)	
		return 0;
	else if(x ==1) 
		return 1;	
	else
		return (ex4(x-1) + ex4(x-2));
	
}
