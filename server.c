/* Copyright 2023 <Niculici Mihai-Daniel> */

#include "server.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Functia initializeaza serverul
server_memory *init_server_memory()
{
	server_memory *server = malloc(sizeof(server_memory));
	DIE(server == NULL, "malloc() server failed!\n");
	server->ht = ht_create(HMAX, hash_function_key,
				compare_function_strings);
	return server;
}

// Aceasta functie adauga un element in hashtable-ul serverului
void server_store(server_memory *server, char *key, char *value) {
	ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

// Aceasta functie returneaza valoarea asociata cheii din hashtable-ul
// serverului
char *server_retrieve(server_memory *server, char *key) {
	return ht_get(server->ht, key);
}

// Aceasta functie sterge un element din hashtable-ul serverului
void server_remove(server_memory *server, char *key) {
	for(unsigned int i = 0; i < server->ht->hmax; i++) {
		if(server->ht->buckets[i] != NULL) {
			int idx = 0;
			ll_node_t *node = server->ht->buckets[i]->head;
			while(node != NULL) {
				if(strcmp((char *)((info *)(node->data))->key, key) == 0) {
					ll_node_t *trash = ll_remove_nth_node(server->ht->buckets[i], idx);
					if(trash != NULL) {
						free(((info *)(trash->data))->key);
						free(((info *)(trash->data))->value);
						free(trash->data);
						free(trash);
					}
					return;
				}
				idx++;
				node = node->next;
			}
		}
	}
}

// Aceasta functie elibereaza memoria serverului
void free_server_memory(server_memory *server) {
	ht_free(server->ht);
	free(server);
}
