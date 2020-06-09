/* NYU Computer Systems Organization Lab 2
 * Rabin-Karp Substring Matching
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>

#include "rkgrep.h"
#include "bloom.h"

#define PRIME 961748941

// calculate modulo addition, i.e. (a+b) % PRIME
long long
madd(long long a, long long b)
{
	return (a + b) % PRIME;
}

// calculate modulo substraction, i.e. (a-b) % PRIME
long long
msub(long long a, long long b)
{
	return (a>b)?(a-b):(a+PRIME-b);
}

// calculate modulo multiplication, i.e. (a*b) % PRIME
long long
mmul(long long a, long long b)
{
	return (a*b) % PRIME;
}

/* naive_substring_match returns number of positions in the document where
 * the pattern has been found.  In addition, it stores the first position 
 * where the pattern is found in the variable pointed to by first_match_ind
 *
 * Both its arguments "pattern" and "doc" are null-terminated C strings.
 */
int
naive_substring_match(const char *pattern, const char *doc, int *first_match_ind)
{
	/* Your code here */
		
	int i=0;
	int j=0;
	int found =0;
	int count=0;
	
	// as it can only be -1 once, only the first occurance index will be stored.
	*first_match_ind = -1; 
	while (*(doc+i)) 
	{
		j=0;
		found =1;
		while(*(pattern+j)) 
		{
			if( *(doc+i+j) !=  *(pattern+j)) 
			{
				found = 0;
				break;
			}
			j++;
		}
		if(found)
		{
			count++;
			if(*first_match_ind == -1)
				*first_match_ind = i;
		}
		i++;
	}
	return count;
}

/* initialize the Rabin-karp hash computation by calculating 
 * and returning the RK hash over a charbuf of m characters, 
 * i.e. The return value should be 
 * 256^(m-1)*charbuf[0] + 256^(m-2)*charbuf[1] + ... + charbuf[m-1],
 * where 256^(m-1) means 256 raised to the power m-1.
* where h is 256 raised to the power m(and given as an argument)
 * Note that all operations *, +, - are modulo arithematic, so you 
 * should use the provided functions mmul, madd, msub.
 * (We use "long long" to represent an RK hash)
 */
long long
rkhash_init(const char *charbuf, int m, long long *h)
{
	/* Your code here */
	int i, j ;
	int len = m;
	long long base = 256;
	long long exp = 1;
   	long long sum = 0;
	long long hash = 1;

	for(i=0; i<m; i++){     //calculate 256^m-1
        hash = mmul(base, hash);
   	}

	*h = hash;  //stores 256^m in *h

	for(i=0; i<m; i++)
	{
        	for(int j=1; j <len ; j++) 
        		exp = mmul(base, exp);
        	sum = madd(sum, mmul(exp, charbuf[i]));
        	exp = 1;
        	len --; //len subtracts in order to calculate 256^(m-1)*charbuf[0] + 256^(m-2)*charbuf[1] + ..
   	}

    return sum;
	
}


/* Given the rabin-karp hash value (curr_hash) over substring Y[i],Y[i+1],...,Y[i+m-1]
 * calculate the hash value over Y[i+1],Y[i+2],...,Y[i+m] = curr_hash * 256 - leftmost * h + rightmost
 * where h is 256 raised to the power m (and given as an argument).  
 * Note that *,-,+ refers to modular arithematic so you should use mmul, msub, madd.
 */
long long 
rkhash_next(long long curr_hash, long long h, char leftmost, char rightmost)
{
	/* Your code here */
	curr_hash= madd(msub(mmul(256,curr_hash),mmul(leftmost,h)),rightmost);
	return curr_hash;
}


/* rk_substring_match returns the number of positions in the document "doc" where
 * the "pattern" has been found, using the Rabin-karp substring matching algorithm.
 * Both pattern and doc are null-terminated C strings. The function also stores
 * the first position where pattern is found in the int variable pointed to by first_match_ind
 *
 * Note: You should implement the Rabin-Karp algorithm by completing the 
 * rkhash_init and rkhash_next functions and then use them here.
*/
int
rk_substring_match(const char *pattern, const char *doc, int *first_match_ind)
{
	
      /* Your code here */
	*first_match_ind = -1;
	int count=0;
	int found;
	int i,j;
	int m=strlen(pattern);
	int n = strlen(doc); 	
	long long p_hash ;
	long long d_hash;
	long long hash; 
	long long *h = malloc(sizeof(long long));

    	//initial hashes
	p_hash = rkhash_init(pattern, m, h); 
	d_hash = rkhash_init(doc, m, h);     

	hash = *h; //store 256^m that was calculated from rkhash_init to use later

	for(i=0; i<n-m+1; i++)
	{
        	if(p_hash == d_hash) 
        	{
           		found =1;
            		for(j=0; j<m; j++)
           		{
                		if(doc[i+j]  !=  pattern[j]) //even if hash is same, check the strings to confirm
                    		{
                        		found =0;
                        		break;
                    		}
            		}
            		if (found == 1) 
			{
                		count++;
                		if(*first_match_ind == -1)
                        		*first_match_ind = i;
                 	}
        	}

		if(i<n-m) // next rolling hash
			d_hash= rkhash_next(d_hash, hash, doc[i], doc[i+m]);
	}
    return count;
}


/* rk_create_doc_bloom returns a pointer to a newly created bloom_filter. 
 * The new bloom filter is populated with all n-m+1 rabin-karp hashes for 
 * all the substrings of length m in "doc".
 * Hint: use the provided bloom_init() and your implementation of bloom_add() here.
 */
bloom_filter *
rk_create_doc_bloom(int m, const char *doc, int bloom_size)
{
	/* Your code here */
	long long *h = malloc(sizeof(long long));
	long long hash;
	int i, j;
	int n  = strlen(doc); 
	
	bloom_filter *bf = bloom_init(bloom_size); //initialized the bloom filter
	
	long long d_hash= rkhash_init(doc, m, h); 
	hash= *h;
	
	for(i=0; i<n-m+1; i++)
	{
		bloom_add(bf, d_hash); //add the hash values of size m in bf

		if(i<n-m)
			d_hash= rkhash_next(d_hash, hash, doc[i], doc[i+m]); // calculate the next has to be added.
	}	
	return bf;
}

/* rk_substring_match_using_bloom returns the total number of positions where "pattern" 
 * is found in "doc".  It performs the matching by first checking against the 
 * pre-populated bloom filter "bf" (which has been created by rk_create_doc_bloom on "doc")
 * If the pattern is not found in "bf", then the function immediately returns 0.
 * Otherwise, the function invokes rk_substring_match() function to find "pattern" in "doc".
*/
int
rk_substring_match_using_bloom(const char *pattern, const char *doc, bloom_filter *bf, int *first_match_ind)
{
    /* Your code here */
 	int count =0;
	bool result ;
	int m = strlen(pattern);
	long long *h = malloc(sizeof(long long));
	long long p_hash = rkhash_init(pattern, m, h);  //calculate the hash for pattern that is being searched

	//check if the bloom filter has pattern
	result = bloom_query(bf, p_hash); 
	
	//if pattern not found, immediately returns 0
	if(result == false) 
		return 0;	

	if(result)  
		count = rk_substring_match(pattern, doc, first_match_ind);
		
	return count;
}