#include "foo.h"

int square_i32(int n)
{
    int res;
    res = n * n;
    return res;
}

int pow_i32(int x, int n)
{
    int acc;
    int i;
    if (n == 0) { return 1; }
    acc = x;
    for (i = 1; i < n; ++i)
    {
        acc *= x;
    }
    return acc;
}
