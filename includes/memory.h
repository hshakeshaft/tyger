#ifndef TYGER_MEMORY_H_
#define TYGER_MEMORY_H_
#include <stdlib.h>

#define TYGER_MALLOC(SIZE) malloc(SIZE)

#define TY_MOVE_PTR(A, B)   \
do {                        \
    (B) = (A);              \
    (A) = NULL;             \
} while (0) 

#endif  /* TYGER_MEMORY_H_ */
