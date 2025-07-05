#ifndef TYGER_UTIL_H_
#define TYGER_UTIL_H_
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define va_array_init(T, DA)                        \
  do {                                              \
    (DA).capacity = 32;                             \
    (DA).len = 0;                                   \
    (DA).elems = malloc(sizeof(T) * (DA).capacity); \
  } while (0)

#define va_array_free(DA)                       \
  do {                                          \
    if ((DA).elems) {                           \
      free((DA).elems);                         \
    }                                           \
    (DA).elems = NULL;                          \
    (DA).len = 0;                               \
    (DA).capacity = 0;                          \
  } while (0)

/// appends an element to an array
#define va_array_append(DA, ELEM)                               \
  do {                                                          \
    if ( (DA).len + 1 > (DA).capacity ) {                       \
      size_t new_capacity = (DA).capacity * 2;                  \
      void *new_buffer = realloc((DA).elems, new_capacity);     \
      if (new_buffer != (DA).elems) {                           \
        (DA).elems = new_buffer;                                \
      }                                                         \
      (DA).capacity = new_capacity;                             \
    }                                                           \
    memcpy( &((DA).elems[(DA).len]), &(ELEM), sizeof((ELEM)) ); \
    (DA).len += 1;                                              \
  } while (0)

/// appends n ELEMS to the end of variable array
#define va_array_append_n(DA, ELEMS, N)                               \
  do {                                                                \
    if ( ((DA).len + (N)) > (DA).capacity ) {                         \
      size_t new_capacity = (DA).capacity * 2;                        \
      void *new_buffer = realloc((DA).elems, new_capacity);           \
      if (new_buffer != (DA).elems) {                                 \
        (DA).elems = new_buffer;                                      \
      }                                                               \
      (DA).capacity = new_capacity;                                   \
    }                                                                 \
    memcpy(&((DA).elems[(DA).len]), ELEMS, (N) * sizeof((ELEMS)[0])); \
    (DA).len += (N);                                                  \
  } while (0)

#define va_array_next(DA) &( (DA).elems[(DA).len] )

#endif // TYGER_UTIL_H_
