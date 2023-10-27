/* Copyright 2023 <Niculici Mihai-Daniel> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"
#include "server.h"

// Structura Load Balancer
struct load_balancer {
    server_memory **servers;
    unsigned int *hash_ring;
    int servers_count;
};

// Functia de Hash pentru servere
unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

// Functia de initializare a load balancerului
load_balancer *init_load_balancer() {
    load_balancer *main = malloc(sizeof(load_balancer));
    DIE(main == NULL, "malloc() load_balancer failed!\n");
    main->servers_count = 0;

    return main;
}

// Functia de comparare a hash-urilor
int compare_hash(const void *a, const void *b)
{
    unsigned int uint_a = *((unsigned int *)a);
    unsigned int uint_b = *((unsigned int *)b);

    if (uint_a == uint_b)
        return 0;
    if (uint_a < uint_b)
        return -1;
    return 1;
}

// Functia face initializarea serverului
void initialize_server(server_memory *server, int server_id)
{
    server->server_id = server_id;
    server->server_hash = hash_function_servers(&server_id);

    server->replicated_server_id_1 = Formula + server_id;
    server->replicated_server_id_1_hash = hash_function_servers
    (&server->replicated_server_id_1);

    server->replicated_server_id_2 = 2 * Formula + server_id;
    server->replicated_server_id_2_hash = hash_function_servers
    (&server->replicated_server_id_2);
}

// Functia aceasta aloca memorie pentru servere si hash_ring, daca este primul
//  server. Daca nu, realoca memoria pentru servere si hash_ring
void initialize_realloc_load_balancer(load_balancer *main)
{
    if (main->servers_count == 0) {
        main->servers_count++;
        main->servers = malloc(main->servers_count * sizeof(server_memory *));
        DIE(main->servers == NULL, "malloc() main->servers failed!\n");

        main->hash_ring = malloc((3 * main->servers_count) *
                                 sizeof(unsigned int));
        DIE(main->hash_ring == NULL, "malloc() main->hash_ring failed!\n");

    } else {
        main->servers_count++;
        main->servers = realloc(main->servers, main->servers_count *
                                sizeof(server_memory *));
        DIE(main->servers == NULL, "realloc() main->servers failed!\n");

        main->hash_ring = realloc(main->hash_ring, (3 * main->servers_count) *
                                  sizeof(unsigned int));
        DIE(main->hash_ring == NULL, "realloc() main->hash_ring failed!\n");
    }
}

// Functia aceasta, dupa adaugarea unui server nou,
// ia o parte din produsele de pe celelalte servere si le pune pe serverul nou
void rebalance_1(load_balancer *main, server_memory *server) {
    server_memory *next_server;
    unsigned int next_server_hash = 0;
    int ok = 0;

    // Cautam hash-ul urmator pe hash_ring

    for (int i = 0; i < main->servers_count * 3; i++) {
        if (server->server_hash < main->hash_ring[i]) {
            next_server_hash = main->hash_ring[i];
            ok = 1;
            break;
        }
    }
    if (ok == 0)
        next_server_hash = main->hash_ring[0];

    if (next_server_hash == server->server_hash ||
        next_server_hash == server->replicated_server_id_1_hash ||
        next_server_hash == server->replicated_server_id_2_hash)
        return;

    // Cautam serverul urmator in vectorul de servere cu ajutorul hash-ului.

    for (int i = 0; i < main->servers_count; i++)
        if (main->servers[i]->server_hash == next_server_hash\
           ||
            main->servers[i]->replicated_server_id_1_hash == next_server_hash ||
            main->servers[i]->replicated_server_id_2_hash == next_server_hash)
            next_server = main->servers[i];

    // Parcurg toate produsele din serverul urmator si pun o parte
    // din ele pe serverul nou

    for (unsigned int j = 0; j < next_server->ht->hmax; j++) {
        ll_node_t *node = next_server->ht->buckets[j]->head;
        while (node != NULL) {
            char *key = (char *)(((info *)(node->data))->key);
            char *value = (char *)(((info *)(node->data))->value);

            // Daca hash-ul produsului este mai mic decat hash-ul serverului
            // Sau daca serverul este primul de pe hash_ring, atunci pun
            // produsul pe serverul nou

            if (server->server_hash > hash_function_key(key) ||
                main->hash_ring[0] == server->server_hash) {
                server_store(server, key, value);
            }
            node = node->next;
        }
    }
}

// Aceasta functie face exact acelasi lucru ca rebalance1, doar ca pentru
// al doilea hash al serverului
void rebalance_2(load_balancer *main, server_memory *server) {
    server_memory *next_server;
    unsigned int next_server_hash;
    int ok = 0;

    for (int i = 0; i < main->servers_count * 3; i++) {
        if (server->replicated_server_id_1_hash < main->hash_ring[i]) {
            next_server_hash = main->hash_ring[i];
            ok = 1;
            break;
        }
    }
    if (ok == 0)
        next_server_hash = main->hash_ring[0];

    if (next_server_hash == server->server_hash ||
        next_server_hash == server->replicated_server_id_1_hash ||
        next_server_hash == server->replicated_server_id_2_hash)
        return;

    for (int i = 0; i < main->servers_count; i++)
        if (main->servers[i]->server_hash == next_server_hash ||
            main->servers[i]->replicated_server_id_1_hash == next_server_hash ||
            main->servers[i]->replicated_server_id_2_hash == next_server_hash)
            next_server = main->servers[i];
    for (unsigned int j = 0; j < next_server->ht->hmax; j++) {
        ll_node_t *node = next_server->ht->buckets[j]->head;
        while (node != NULL) {
            char *key = (char *)(((info *)(node->data))->key);
            char *value = (char *)(((info *)(node->data))->value);
            if (server->replicated_server_id_1_hash > hash_function_key(key)
                || main->hash_ring[0] == server->replicated_server_id_1_hash) {
                server_store(server, key, value);
            }
            if (node != NULL)
                node = node->next;
        }
    }
}

// Aceasta functie face exact acelasi lucru ca rebalance1, doar ca pentru al
// treilea hash al serverului
void rebalance_3(load_balancer *main, server_memory *server) {
    server_memory *next_server;
    unsigned int next_server_hash;
    int ok = 0;

    for (int i = 0; i < main->servers_count * 3; i++) {
        if (server->replicated_server_id_2_hash < main->hash_ring[i]) {
            next_server_hash = main->hash_ring[i];
            ok = 1;
            break;
        }
    }
    if (ok == 0)
        next_server_hash = main->hash_ring[0];

    if (next_server_hash == server->server_hash ||
        next_server_hash == server->replicated_server_id_1_hash ||
        next_server_hash == server->replicated_server_id_2_hash)
        return;
    for (int i = 0; i < main->servers_count; i++)
        if (main->servers[i]->server_hash == next_server_hash ||
            main->servers[i]->replicated_server_id_1_hash == next_server_hash ||
            main->servers[i]->replicated_server_id_2_hash == next_server_hash)
            next_server = main->servers[i];
    for (unsigned int j = 0; j < next_server->ht->hmax; j++) {
        ll_node_t *node = next_server->ht->buckets[j]->head;
        while (node != NULL) {
            char *key = (char *)(((info *)(node->data))->key);
            char *value = (char *)(((info *)(node->data))->value);
            if (server->replicated_server_id_2_hash > hash_function_key(key) ||
                main->hash_ring[0] == server->replicated_server_id_2_hash) {
                server_store(server, key, value);
            }
            node = node->next;
        }
    }
}


// Functia de adaugare a unui server pe load balancer
void loader_add_server(load_balancer *main, int server_id) {
    // Cream un server nou
    server_memory *server = init_server_memory();

    // Initializam serverul

    initialize_server(server, server_id);

    // Initializam load balancerul, daca este primul server
    // Daca nu, realocam vectorul de servere si hash ring

    initialize_realloc_load_balancer(main);

    // Adaugam hash-urile serverului pe hash_ring,
    // Apoi adaugam serverul in vectorul de servere.

    int index = (main->servers_count - 1) * 3;

    main->hash_ring[index] = server->server_hash;
    main->hash_ring[index + 1] = server->replicated_server_id_1_hash;
    main->hash_ring[index + 2] = server->replicated_server_id_2_hash;

    main->servers[main->servers_count - 1] = server;

    // Sortam hash-urile de pe hash_ring in ordine crescatoare.

    qsort(main->hash_ring, main->servers_count * 3,
          sizeof(unsigned int), compare_hash);

    // Dupa ce am adaugat serverul nou, acum trebuie sa
    // luam din produsele de pe celelalte servere si sa le
    // punem pe nou server.

    rebalance_1(main, server);
    rebalance_2(main, server);
    rebalance_3(main, server);
}

// Aceasta functie sterge un server din load balancer
void loader_remove_server(load_balancer *main, int server_id) {
    server_memory *removed_server;

    // Cautam serverul pe care vrem sa il stergem

    for (int i = 0; i < main->servers_count; i++)
        if (main->servers[i]->server_id == server_id)
            removed_server = main->servers[i];

    // Ne folosim de aceste variabile ca sa stim daca am sters hash-urile
    // serverului de pe hash_ring, pentru ca fiecare server are 3 hash-uri

    int ok_1;
    int ok_2;
    int ok_3;
    ok_1 = 0;
    ok_2 = 0;
    ok_3 = 0;

    // Stergem hash-urile serverului de pe hash_ring

    for (int i = 0; i < main->servers_count * 3; i++) {
        if (main->hash_ring[i] == removed_server->server_hash && ok_1 == 0) {
            for (int j = i; j < main->servers_count * 3 - 1; j++) {
                main->hash_ring[j] = main->hash_ring[j + 1];
            }
            ok_1 = 1;
            i--;
            continue;
        }

        if (main->hash_ring[i] ==
            removed_server->replicated_server_id_1_hash && ok_2 == 0) {
            for (int j = i; j < main->servers_count * 3 - 1; j++) {
                main->hash_ring[j] = main->hash_ring[j + 1];
            }
            ok_2 = 1;
            i--;
            continue;
        }
        if (main->hash_ring[i] ==
            removed_server->replicated_server_id_2_hash && ok_3 == 0) {
            for (int j = i; j < main->servers_count * 3 - 1; j++) {
                main->hash_ring[j] = main->hash_ring[j + 1];
            }
            ok_3 = 1;
            i--;
            continue;
        }
    }

    // Realocam memoria pentru hash_ring
    int index = (main->servers_count - 1) * 3;

    main->hash_ring = realloc(main->hash_ring, index * sizeof(unsigned int));
    DIE(main->hash_ring == NULL, "realloc() main->hash_ring failed!\n");

    // Stergem serverul din vectorul de servere

    for (int i = 0; i < main->servers_count; i++) {
        if (main->servers[i] == removed_server) {
            for (int j = i; j < main->servers_count - 1; j++) {
                main->servers[j] = main->servers[j + 1];
            }
        }
    }
    // Scadem numarul de servere

    main->servers_count--;

    // Pentru fiecare produs din serverul pe care vrem sa il stergem
    // il punem pe un alt server

    for (unsigned int i = 0; i < removed_server->ht->hmax; i++) {
        ll_node_t *node = removed_server->ht->buckets[i]->head;
        while (node != NULL) {
            char *key = (char *)(((info *)(node->data))->key);
            char *value = (char *)(((info *)(node->data))->value);
            int server_id;
            loader_store(main, key, value, &server_id);
            node = node->next;
        }
    }

    // Realocam memoria pentru vectorul de servere

    main->servers = realloc(main->servers, main->servers_count
                            * sizeof(server_memory *));
    DIE(main->servers == NULL, "realloc() main->servers failed!\n");
    free_server_memory(removed_server);
}

// Aceasta functie adauga un produs pe un server
void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
    // Calculez hash-ul cheii

    unsigned int hashed_key = hash_function_key(key);
    int i;

    // Caut hash-ul serverului pe care trebuie sa il adaug

    for (i = 0; i < main->servers_count * 3; i++) {
        if (hashed_key < main->hash_ring[i]) {
            break;
        }
    }

    // Daca nu am gasit hash-ul, inseamna ca trebuie sa il pun pe primul server

    if (i == main->servers_count * 3) {
        if (hashed_key > main->hash_ring[i - 1])
            i = 0;
        else
            i = main->servers_count * 3 - 1;
    }

    // Calculez hash-ul serverului

    unsigned int hashed_server = main->hash_ring[i];

    // Caut serverul pe care trebuie sa il adaug
    // Apoi adaug produsul pe server

    for (int j = 0; j < main->servers_count; j++) {
        if (main->servers[j]->server_hash == hashed_server) {
            server_store(main->servers[j], key, value);
            *server_id = main->servers[j]->server_id;
            return;
        }
        if (main->servers[j]->replicated_server_id_1_hash == hashed_server) {
            server_store(main->servers[j], key, value);
            *server_id = main->servers[j]->server_id;
            return;
        }
        if (main->servers[j]->replicated_server_id_2_hash == hashed_server) {
            server_store(main->servers[j], key, value);
            *server_id = main->servers[j]->server_id;
            return;
        }
    }
}

// Aceasta functie returneaza valoarea asociata cheii
char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
    // M-am chinuit o gramada si tot nu pot sa imi explic de ce
    // Imi pune un Vertical Tabulation pe ultimul caracter la cheie dar ok

    for (unsigned int j = 0; j < strlen(key); j++) {
        if (key[j] == 13)
            key[j] = '\0';
    }

    // Calculez hash-ul cheii

    unsigned int hashed_key = hash_function_key(key);

    // Caut hash-ul serverului pe care trebuie sa il adaug

    int i = 0;
    for (i = 0; i < main->servers_count * 3; i++) {
        if (hashed_key < main->hash_ring[i]) {
            break;
        }
    }

    // Daca nu am gasit hash-ul, inseamna ca trebuie sa il pun pe primul server

    if (i == main->servers_count * 3) {
        if (hashed_key > main->hash_ring[i - 1])
            i = 0;
        else
            i = main->servers_count * 3 - 1;
    }

    // Calculez hash-ul serverului

    unsigned int hashed_server = main->hash_ring[i];

    // Caut serverul de la care trebuie sa iau produsul
    // Apoi returnez valoarea asociata cheii

    for (int j = 0; j < main->servers_count; j++) {
        if (main->servers[j]->server_hash == hashed_server) {
            char *value = server_retrieve(main->servers[j], key);
            *server_id = main->servers[j]->server_id;
            return value;
        }
        if (main->servers[j]->replicated_server_id_1_hash == hashed_server) {
            char *value = server_retrieve(main->servers[j], key);
            *server_id = main->servers[j]->server_id;
            return value;
        }
        if (main->servers[j]->replicated_server_id_2_hash == hashed_server) {
            char *value = server_retrieve(main->servers[j], key);
            *server_id = main->servers[j]->server_id;
            return value;
        }
    }
    return NULL;
}

// Aceasta functie elibereaza memoria load balancerului
void free_load_balancer(load_balancer *main) {
    for (int i = 0; i < main->servers_count; i++) {
        free_server_memory(main->servers[i]);
    }
    free(main->servers);
    free(main->hash_ring);
    free(main);
}
