#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ht.h"

/* taken from http://www.cse.yorku.ca/~oz/hash.html
    NOTE(HS): original implementation uses `long` rather than `int` - the platforms
    I aim to support define `int` consistently as 32-bits in width, whereas `long`
    is 32-bit on Windows, and 64-bit on *nix.
*/
static unsigned int ht__djb2_hash_key(const char *key, size_t key_len)
{
    unsigned int hash;
    size_t i;
    char c;
    hash = 5381;
    for (i = 0; i < key_len; ++i)
    {
        c = key[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/* find the slot in the hash table which matches a supplied key
@param ht - the hash table
@param key - the key to match agains
@param prev_slot - an out pointer which is overriden with the previous slot, used
in collision resolution (insertion at end)
@return the first matching slot, or NULL
*/
static HT_Slot *ht__find_matching_slot_by_key(HT *ht, const char *key, HT_Slot **prev_slot)
{
    HT_Slot *slot;
    HT_Header *header;
    size_t key_len;
    size_t bucket;
    unsigned int hash;

    header  = ((HT_Header*) *ht) - 1;
    key_len = strlen(key);
    hash    = ht__djb2_hash_key(key, key_len);
    bucket  = hash % header->buckets;
    slot    = &(*ht)[bucket];
    *prev_slot = slot;

    while (slot != NULL)
    {
        if (slot->key == NULL || strcmp(slot->key, key) == 0) { break; }
        *prev_slot = slot;
        slot = slot->next;
    }

    if (slot == NULL) { slot = &header->nil_slot; }

    return slot;
}

void ht_init(HT *ht, size_t buckets)
{
    HT_Header *header;
    size_t alloc_size;

    alloc_size  = sizeof(*header);
    alloc_size += sizeof(**ht) * buckets;
    header      = malloc(alloc_size);
    memset(header, 0x00, alloc_size);

    header->buckets        = buckets;
    header->nil_slot.key   = NULL;
    header->nil_slot.value = NULL;
    header->nil_slot.next  = NULL;
    (*ht)                  = (HT) (header + 1);
}

/* TODO(HS): "chase" slot chains */

void ht_deinit(HT *ht)
{
    size_t i;
    HT_Header *header;
    header = ((HT_Header*) *ht) - 1;

    for (i = 0; i < header->buckets; ++i)
    {
        HT_Slot *cur_slot;
        HT_Slot *prev_slot;

        prev_slot = NULL;
        cur_slot  = (*ht)[i].next;

        while (cur_slot != NULL)
        {
            prev_slot = cur_slot;
            cur_slot  = cur_slot->next;
            free(prev_slot);
            prev_slot = NULL;
        }
    }

    free(header);
}

void ht_insert(HT *ht, const char *key, TyObject *value)
{
    HT_Header *header;
    HT_Slot *slot;
    HT_Slot *prev_slot;
    header = ((HT_Header*) *ht) - 1;
    slot = ht__find_matching_slot_by_key(ht, key, &prev_slot);
    if (slot == &header->nil_slot)
    {
        prev_slot->next = malloc(sizeof(*prev_slot));
        slot            = prev_slot->next;
        slot->key       = key;
        slot->value     = value;
        slot->next      = NULL;
    }
    else
    {
        if (slot->key == NULL) { slot->key = key; }
        slot->value = value;
    }
}

HT_Slot *ht_get(HT *ht, const char *key)
{
    HT_Slot *slot;
    HT_Slot *prev_slot;
    slot = ht__find_matching_slot_by_key(ht, key, &prev_slot);
    if (slot == NULL) { slot = prev_slot; }
    return slot;
}

int ht_delete(HT *ht, const char *key)
{
    HT_Slot *slot;
    HT_Slot *prev_slot;
    HT_Slot *next_slot;
    int success;

    success = 0;

    slot      = ht__find_matching_slot_by_key(ht, key, &prev_slot);
    next_slot = NULL;

    #if 0
    #else
    if (slot != NULL && slot->key != NULL)
    {
        if (slot->next != NULL) { next_slot = slot->next; }

        /* NOTE(HS): this should always refer to the initial slot in the resolution chain */
        if (prev_slot == slot)
        {
            if (next_slot == NULL)
            {
                slot->key   = NULL;
                slot->value = NULL;
                slot->next  = NULL;
            }
            else
            {
                slot->key   = next_slot->key;
                slot->value = next_slot->value;
                slot->next  = next_slot->next;
            }
            success     = 1;
        }
        else
        {
            free(slot);
            prev_slot->next = next_slot;
            success         = 1;
        }
    }
    #endif

    return success;
}
