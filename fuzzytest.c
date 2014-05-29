/*
    test fuzzy matching
    getting this right is crucial
*/
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int a,b;
} tPair;

tPair pairs[] = {
        {10,11},
        {11,10},
        {14,16},
        {15,16},
        {17,16},
        {18,16},
        {16,14},
        {16,15},
        {16,17},
        {16,18},
        {0,0}
};

/*
int fuzzyMatch(int reference, int value)
{
    return ( abs(value - reference) * 10 < reference );
}
*/

int main(int arc, const char *argv[])
{
    tPair *pair;
    
    pair = pairs;
    while ( pair->a != 0 || pair->b != 0 )
    {
        printf("(%d,%d) = %d\t", pair->a, pair->b,
                (abs(pair->a - pair->b) * 2000) / ((pair->a + pair->b)) );
        printf("(%d,%d) = %d\n", pair->a, pair->b,
                (abs(pair->b - pair->a) * 1000) / (pair->b) );
        ++pair;
    }
}
