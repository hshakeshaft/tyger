/* da.h - dynamic array library */
#ifndef DA_H_
#define DA_H_
#include <string.h>

#if !defined(DA_MALLOC) && !defined(DA_REALLOC)
    #include <stdlib.h>
    #define DA_MALLOC(SIZE) malloc((SIZE))
    #define DA_REALLOC(PTR, SIZE) realloc((PTR), (SIZE))
#endif

#ifndef DA_ASSERT
    #include <assert.h>
    #define DA_ASSERT(COND, MSG) assert(COND && __FILE__ " :: " MSG);
#endif


#define DA_INIT(DA, CAPACITY)                                                   \
do {                                                                            \
    size_t da_memory_capacity;                                                  \
    da_memory_capacity = sizeof(*(DA)->elems) * (CAPACITY);                     \
    (DA)->elems = DA_MALLOC(da_memory_capacity);                                \
    (DA)->count = 0;                                                            \
    (DA)->capacity = (CAPACITY);                                                \
} while (0)

#define DA_APPEND(DA, ELEM)                                                     \
do {                                                                            \
    if ( (DA)->count + 1 >= (DA)->capacity ) {                                  \
        size_t new_capacity;                                                    \
        size_t new_memory_capacity;                                             \
        void *new_ptr;                                                          \
        new_capacity = (DA)->capacity * 2;                                      \
        new_memory_capacity = new_capacity * sizeof(*(DA)->elems);              \
        new_ptr = DA_REALLOC((DA)->elems, new_memory_capacity);                 \
        DA_ASSERT(new_ptr, "failed to reallocate space for dynamic array");     \
        (DA)->elems = new_ptr;                                                  \
        (DA)->capacity = new_memory_capacity;                                   \
    }                                                                           \
    memcpy(&(DA)->elems[(DA)->count], (ELEM), sizeof(*ELEM));                   \
    (DA)->count += 1;                                                           \
} while (0)

#endif  /* DA_H_ */
