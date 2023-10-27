/* Copyright 2023 <Niculici Mihai-Daniel> */
#ifndef HASH_H_
#define HASH_H_

#define MAX_STRING_SIZE	256
#define HMAX 5
#define VALUE_LENGTH 65536
#define KEY_LENGTH 128

typedef struct info info;
struct info {
	void *key;
	void *value;
};

typedef struct ll_node_t
{
    void* data;
    struct ll_node_t* next;
} ll_node_t;

typedef struct linked_list_t
{
    ll_node_t* head;
    unsigned int data_size;
    unsigned int size;
} linked_list_t;

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	linked_list_t **buckets;
	unsigned int size;
	unsigned int hmax;
	unsigned int (*hash_function)(void*);
	int (*compare_function)(void*, void*);
};

linked_list_t *ll_create(unsigned int data_size);
void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data);
ll_node_t *ll_remove_nth_node(linked_list_t* list, unsigned int n);
unsigned int ll_get_size(linked_list_t* list);
void ll_free(linked_list_t** pp_list);
int compare_function_strings(void *a, void *b);
unsigned int hash_function_key(void *a);

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*));

int ht_has_key(hashtable_t *ht, void *key);
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);
void ht_remove_entry(hashtable_t *ht, void *key);
void ht_free(hashtable_t *ht);
unsigned int ht_get_size(hashtable_t *ht);
unsigned int ht_get_hmax(hashtable_t *ht);
void *ht_get(hashtable_t *ht, void *key);

#endif  /* HASH_H_ */
