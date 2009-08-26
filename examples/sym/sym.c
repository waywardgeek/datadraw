#include "ddutil.h"

int main(void)
{
    char first, second, third, fourth;
    uint32 xLookup;

    utStart();
    printf("Building symbols\n");
    for(first = 'a'; first <= 'z'; first++) {
	for(second = 'a'; second <= 'z'; second++) {
	    for(third = 'a'; third <= 'z'; third++) {
		for(fourth = 'a'; fourth <= 'z'; fourth++) {
		    utSymCreateFormatted("%c%c%c%c", first, second, third, fourth);
		}
	    }
	}
    }
    printf("Looking up symbols\n");
    for(xLookup = 0; xLookup < 24*24*24*24; xLookup++) {
	utSymCreateFormatted("%c%c%c%c", 'a' + utRandN(24), 'a' + utRandN(24), 'a' + utRandN(24), 'a' + utRandN(24));
    }
    utStop(false);
    return 0;
}
