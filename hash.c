/* Copyright 2023 <Niculici Mihai-Daniel> */
#include "./hash.h"
#include "./utils.h"

// Aceasta implementare am luat-o de la partialul de anul trecut
// si am modificat-o pentru a se potrivi cu cerintele temei
unsigned int hash_function_key(void *a)
{
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

// Creez lista
linked_list_t *
ll_create(unsigned int data_size)
{
    linked_list_t* linked_list;

    linked_list = malloc(sizeof(*linked_list));

    linked_list->head = NULL;
    linked_list->data_size = data_size;
    linked_list->size = 0;

    return linked_list;
}

// Adaug un nod in lista
void
ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    ll_node_t *prev, *curr;
    ll_node_t* node_to_add;

    if (!list) {
        return;
    }

    if (n > list->size) {
        n = list->size;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

    node_to_add = malloc(sizeof(*node_to_add));
    node_to_add->data = malloc(list->data_size);
    memcpy(node_to_add->data, new_data, list->data_size);

    node_to_add->next = curr;
    if (prev == NULL) {
        list->head = node_to_add;
    } else {
        prev->next = node_to_add;
    }

    list->size++;
}

// Sterg un nod din lista
ll_node_t *
ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t *prev, *curr;

    if (!list || !list->head) {
        return NULL;
    }

    if (n > list->size - 1) {
        n = list->size - 1;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

    if (prev == NULL) {
        list->head = curr->next;
    } else {
        prev->next = curr->next;
    }

    list->size--;

    return curr;
}

// Returnez dimensiunea listei
unsigned int
ll_get_size(linked_list_t* list)
{
     if (!list) {
        return -1;
    }

    return list->size;
}

// Eliberez lista
void
ll_free(linked_list_t** pp_list)
{
    ll_node_t* currNode;

    if (!pp_list || !*pp_list) {
        return;
    }

    while (ll_get_size(*pp_list) > 0) {
        currNode = ll_remove_nth_node(*pp_list, 0);
        free(currNode->data);
        currNode->data = NULL;
        free(currNode);
        currNode = NULL;
    }

    free(*pp_list);
    *pp_list = NULL;
}

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

// Creez hashtable-ul
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
    if (!hash_function || !compare_function) {
        return NULL;
    }

    hashtable_t* map = malloc(sizeof(hashtable_t));

    map->size = 0;
    map->hmax = hmax;
    map->hash_function = hash_function;
    map->compare_function = compare_function;

    map->buckets = malloc(map->hmax * sizeof(linked_list_t *));
    for (unsigned int i = 0; i < map->hmax; ++i) {
        map->buckets[i] = ll_create(sizeof(info));
    }
    return map;
}

// Verific daca exista cheia in hashtable
int
ht_has_key(hashtable_t *ht, void *key)
{
    if (!ht || !key) {
        return -1;
    }

    int hash_index = ht->hash_function(key) % ht->hmax;
    ll_node_t* node = ht->buckets[hash_index]->head;

    while (node != NULL) {
        info* data_info = (info *)node->data;
        if (!ht->compare_function(data_info->key, key)) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}
// Returnez valoarea asociata cheii
void *
ht_get(hashtable_t *ht, void *key)
{
	if (!ht || !key || ht_has_key(ht, key) != 1) {
        return NULL;
    }

    int hash_index = ht->hash_function(key) % ht->hmax;
    ll_node_t* node = ht->buckets[hash_index]->head;

    while (node != NULL) {
        info* data_info = (info *)node->data;
        if (!ht->compare_function(data_info->key, key)) {
            return data_info->value;
        }
        node = node->next;
    }
    return NULL;
}

// Adaug o noua intrare in hashtable
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
    void *value, unsigned int value_size)
{
    if (!ht || !key || !value) {
        return;
    }

    int hash_index = ht->hash_function(key) % ht->hmax;

    if (ht_has_key(ht, key) == 1) {
        ll_node_t* node = ht->buckets[hash_index]->head;
        while (node != NULL) {
            info* data_info = node->data;

            if (!ht->compare_function(data_info->key, key)) {
                free(data_info->value);

                data_info->value = malloc(value_size);
                memcpy(data_info->value, value, value_size);
                return;
            }

            node = node->next;
        }
    }

    info* data_info = malloc(sizeof(info));

    data_info->key = malloc(key_size);
    data_info->value = malloc(value_size);

    memcpy(data_info->key, key, key_size);
    memcpy(data_info->value, value, value_size);

    ll_add_nth_node(ht->buckets[hash_index], 0, data_info);
    ht->size++;

	if(data_info)
    	free(data_info);
}

// Sterg o intrare din hashtable
void
ht_remove_entry(hashtable_t *ht, void *key)
{
	if (!ht || !key || ht_has_key(ht, key) != 1) {
        return;
    }

    int hash_index = ht->hash_function(key) % ht->hmax;
    ll_node_t* node = ht->buckets[hash_index]->head;

    unsigned int node_nr = 0;
    while (node != NULL) {
        info* data_info = (info *)node->data;

        if (!ht->compare_function(data_info->key, key)) {
            free(data_info->key);
            free(data_info->value);
            free(data_info);

            ll_node_t* deleted_node =
            ll_remove_nth_node(ht->buckets[hash_index], node_nr);
            free(deleted_node);

            ht->size--;
            return;
        }

        node = node->next;
        node_nr++;
    }
}

// Eliberez hashtable-ul
void
ht_free(hashtable_t *ht)
{
    if (!ht) {
        return;
    }

    for (unsigned int i = 0; i < ht->hmax; ++i) {
        ll_node_t* node = ht->buckets[i]->head;

        while (node != NULL) {
            info* data_info = (info *)node->data;
            free(data_info->key);
            free(data_info->value);
            node = node->next;
        }

        ll_free(&ht->buckets[i]);
    }
    free(ht->buckets);
    free(ht);
}

// Returnez dimensiunea hashtable-ului
unsigned int
ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

// Returnez dimensiunea maxima a hashtable-ului
unsigned int
ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
