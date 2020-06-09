//Implement a C function called ex5 such that it achieves 
//the same functionality as the machine code of objs/ex5_sol.o
//Note: you need to figure out ex5()'s function signature yourself; 
//the signature is not void ex5(node *n). However, as a hint, 
//ex5 does take node * as an argument.
 
#include <assert.h>
#include "ex5.h"

int
ex5(node *n) {
	if(n==0)
		return 1;
	
	while (n->next != 0)
	{
		if(n->value > n->next->value)
			return 0;
		n=n->next;
	}
	
	return 1;
}
