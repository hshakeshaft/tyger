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

/* TODO(HS): implement some kind of "find_matching_slot" function which finds either
a slot matching the provided key, or NULL if no match found.
Should also write to an "out-param" the pointer to the last found value such that
I can perform insertions of new slots at the end of a collision resolution chain
*/

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

    while (slot != NULL)
    {
        *prev_slot = slot;
        if (slot->key == NULL || strcmp(slot->key, key) == 0) { break; }
        slot = slot->next;
    }

    return slot;
}


/* TODO(HS): I want failing look ups to return some invalid slot, need to have a
"nil" like instance - stick a slot in the header which acts as the "nil"?
*/

void ht_init(HT *ht, size_t buckets)
{
    HT_Header *header;
    size_t alloc_size;

    alloc_size  = sizeof(*header);
    alloc_size += sizeof(**ht) * buckets;
    header      = malloc(alloc_size);
    memset(header, 0x00, alloc_size);

    header->buckets = buckets;
    (*ht)           = (HT) (header + 1);
}

/* TODO(HS): "chase" slot chains */

void ht_deinit(HT *ht)
{
    HT_Header *header;
    header = ((HT_Header*) *ht) - 1;
    free(header);
}


void ht_insert(HT *ht, const char *key, TyObject *value)
{
    HT_Header *header;
    unsigned int hash;
    size_t key_len;
    size_t bucket;
    HT_Slot *slot_info;

    header    = ((HT_Header*) *ht) - 1;
    key_len   = strlen(key);
    hash      = ht__djb2_hash_key(key, key_len);
    bucket    = hash % header->buckets;
    slot_info = &(*ht)[bucket];

    if (slot_info->key != NULL)
    {
        if (strcmp(slot_info->key, key) == 0)
        {
            slot_info->value = value;
        }
        else
        {
            HT_Slot *cur_slot;
            HT_Slot *prev_slot;

            prev_slot = slot_info;
            cur_slot  = prev_slot->next;

            while (cur_slot != NULL)
            {
                if (strcmp(cur_slot->key, key) == 0)
                {
                    cur_slot->value = value;
                    break;
                }
                else
                {
                    prev_slot = cur_slot;
                    cur_slot  = cur_slot->next;
                }
            }

            /*  NOTE(HS): in cases where no matching key found, then we insert a new
            one in the resolution chain
            */
            if (cur_slot == NULL)
            {
                prev_slot->next = malloc(sizeof(*prev_slot));
                cur_slot        = prev_slot->next;
                cur_slot->key   = key;
                cur_slot->value = value;
                cur_slot->next  = NULL;
            }
        }
    }
    else
    {
        slot_info->key = key;
        slot_info->value = value;
        slot_info->next = NULL;
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
