/* A hash-table implementation 

Implementation details:
    - the hash function is hard coded, and is the djb2
    - the hash itself wraps "slots" which have a hard coded dependency on the Tyger
    Object type (i.e. the only value that can be containined is an Object)
    - collision resolution is performed with the "separate chaining" technique
        - https://craftinginterpreters.com/hash-tables.html#separate-chaining
*/
#ifndef HT_LIB_H_
#define HT_LIB_H_
#include <stddef.h>

#include "object.h"

typedef struct
{
    size_t buckets;
} HT_Header;

typedef struct ht_slot
{
    /* NOTE(HS): key is a non-owning reference - this is intentional as an ident
    theoretically can enter & exit program scope many times.
    */
    const char *key;
    TyObject *value;
    struct ht_slot *next;
} HT_Slot;

/* NOTE(HS): hash table by default utillises the djb2 hash function */
typedef HT_Slot *HT;

void ht_init(HT *ht, size_t buckets);
void ht_deinit(HT *ht);

/* NOTE(HS): currently attempts at re-inserting the same key fail with assertion
TODO(HS): on insert of existing key, swap `value` pointers
*/
void ht_insert(HT *ht, const char *key, TyObject *value);

HT_Slot *ht_get(HT *ht, const char *key);

#endif  /* HT_LIB_H_ */
