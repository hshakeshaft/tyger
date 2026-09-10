#include <stdlib.h>
#include <string.h>

#include "ht.h"

/* taken from http://www.cse.yorku.ca/~oz/hash.html
    NOTE(HS): original implementation uses `long` rather than `int` - the platforms
    I aim to support define `int` consistently as 32-bits in width, whereas `long`
    is 32-bit on Windows, and 64-bit on *nix.

    unsigned long
    hash(unsigned char *str)
    {
        unsigned long hash = 5381;
        int c;

        while (c = *str++)
            hash = ((hash << 5) + hash) + c;

        return hash;
    }
*/
static unsigned int ht__djb2_hash_key(const char *key, size_t key_len)
{
    unsigned int hash;
    size_t i;
    char c;
    for (i = 0; i < key_len; ++i)
    {
        c = key[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}


void ht_init(HT *ht, size_t buckets)
{
    HT_Header *header;
    size_t alloc_size;

    alloc_size = sizeof(*header);
    alloc_size = sizeof(**ht) * buckets;
    header     = malloc(alloc_size);
    memset(header, 0x00, alloc_size);

    header->buckets = buckets;
    (*ht)           = (HT) (header + 1);
}

void ht_deinit(HT *ht)
{
    HT_Header *header;
    header = ((HT_Header*) *ht) - 1;
    free(header);
}


/* TODO(HS): hash collision resolution */

void ht_insert(HT *ht, const char *key, TyObject *value)
{
    HT_Header *header;
    unsigned int hash;
    size_t key_len;
    size_t bucket;
    HT_Slot slot_info;

    header  = ((HT_Header*) *ht) - 1;
    key_len = strlen(key);
    hash    = ht__djb2_hash_key(key, key_len);
    bucket  = hash % header->buckets;

    slot_info.key   = key;
    slot_info.value = value;
    slot_info.next  = NULL;
    memcpy(&(*ht)[bucket], &slot_info, sizeof(slot_info));
}

HT_Slot *ht_get(HT *ht, const char *key)
{
    HT_Header *header;
    HT_Slot *slot;
    unsigned int hash;
    size_t key_len;
    size_t bucket;

    header  = ((HT_Header*) *ht) - 1;
    key_len = strlen(key);
    hash    = ht__djb2_hash_key(key, key_len);
    bucket  = hash % header->buckets;

    slot = &(*ht)[bucket];

    return slot;
}
