# Copyright 2023 Niculici Mihai-Daniel 

##    Tema 2 - Load Balancer

    Tema contine urmatoarele fisiere:
        -> hash.c (Este fisierul ce contine implementarea tuturor functiilor
        Legate de Hash_Table)

        -> hash.h (Este fisierul ce contine declararea tuturor functiilor
        Legate de Hash_Table)

            Functiile Legate de Hash_Table Sunt:
                - ht_create (Functia ce creeaza un Hash_Table)

                - ht_has_key (Functia ce verifica daca exista o cheie in
                              Hash_Table)

                - ht_put (Functia ce adauga o cheie si un value in Hash_Table)

                - ht_get (Functia ce returneaza un value in functie de cheie)

                - ht_remove_entry (Functia ce sterge o cheie si un value
                                   din Hash_Table)

                - ht_free (Functia ce elibereaza memoria alocata pentru
                           Hash_Table)

                - ht_get_size (Functia ce returneaza numarul de elemente din
                               Hash_Table)

                - ht_get_hmax (Functia ce returneaza numarul maxim de elemente
                               din Hash_Table)


        -> load_balancer.c (Este fisierul ce contine implementarea tuturor
                           functiilor Legate de Load_Balancer)

        -> load_balancer.h (Este fisierul ce contine declararea tuturor
                           functiilor Legate de Load_Balancer)

            Functiile legate de Load Balancer sunt:

                - init_load_balancer (Functia ce initializeaza Load_Balancer)

                - free_load_balancer (Functia ce elibereaza memoria alocata
                                      pentru Load_Balancer)

                - loader_store (Functia ce adauga un produs pe un server din
                                load Balancer)

                - loader_retrieve (Functia ce returneaza un produs de pe un
                                   server din load Balancer)

                - loader_add_server (Functia ce adauga un server in Load
                                     Balancer)

                - loader_remove_server (Functia ce sterge un server din Load
                                        Balancer)
        
        -> server.c (Este fisierul ce contine implementarea tuturor functiilor
        Legate de Server)

        -> server.h (Este fisierul ce contine declararea tuturor functiilor)

            Functiile legate de Server sunt:

                - init_server_memory (Functia ce initializeaza memoria unui
                                      server)
                                      
                - free_server_memory (Functia ce elibereaza memoria alocata
                                      pentru un server)

                - server_store (Functia ce adauga un produs pe un server)

                - server_retrieve (Functia ce returneaza un produs de pe un
                                   server)

                - server_remove (Functia ce sterge un produs de pe un server)

        -> utils.h (Acest Fisier contine "DIE")

            Alte functii pe care le-am folost pe langa cele date in schelet:

                - compare_hash (Functia ce compara doua hash-uri)

                - initialize_server (Functia face initializarea serverului)

                - initialize_realloc_load_balancer (Functia aceasta aloca
                memorie pentru servere si hash_ring, daca este primul server.
                Daca nu, realoca memoria pentru servere si hash_ring)

                - rebalance_1 (Functia aceasta, dupa adaugarea unui server nou,
                ia o parte din produsele de pe celelalte servere si le pune pe
                serverul nou)

                - rebalance_2 (Face acelasi lucru ca rebalance_1, doar ca
                pentru prima copie a serverului de pe Hash_ring)

                - rebalance_3 (Face acelasi lucru ca rebalance_1, doar ca
                pentru a doua copie a serverului de pe Hash_ring)

        -> Makefile (Acest fisier contine comenzile necesare pentru a executa
                    programul)
_____________________________________________________________________________


# Modul de Gandire in rezolvarea problemei

    Pentru a rezolva aceasta tema, m-am gandit sa folosesc 2
    vectori, unul in care tin serverele si unul in care tin hash_ring-ul.

    In primul rand, trebuie sa initializam Load Balancer-ul inainte de a
    primi comenzile din fisier. Atunci cand primim o comanda, trebuie sa
    o identificam.

## Daca intalnim Comanda "add_server"

    Mai intai trebuie sa cream un server si sa il initializam
    Dupa aceea, trebuie sa ii adaug hash-urile serverului nou
    pe Hash ring, iar apoi sa il adaugam in vectorul de servere.

    Dupa aceea se sorteaza vectorul de hash-uri, iar apoi se face o rebalansare a produselor, adica pentru fiecare hash
    in parte dintre cele 3 ale serverului nou adaugat, caut urmatorul server
    in Hash ring si iau unele produse de pe serverul respectiv, adica cele
    care corespund noului server adaugat.
    Acest lucru se repeta de 3 ori, pentru fiecare hash in parte.

## Daca intalnim Comanda "store"

    Calculam hash-ul cheii, dupa care caut in vectorul ce contine Hash Ring-ul,
    cel mai mic hash, mai mare decat hash-ul produsului astfel obtinand un
    hash al unui server.
    Apoi caut in vectorul de servere, serverul care ii
    corespunde hash-ul, iar apoi adaug produsul pe serverul respectiv.

## Daca intalnim Comanda "retrieve"

    Acest proces este asemanator cu cel de la "store", doar ca in loc sa adaug, trebuie doar sa vad daca exista produsul pe serverul respectiv, iar daca exista, il returnez.

## Daca intalnim Comanda "remove_server"

    Mai intai caut serverul in vectorul de servere, dupa ID, iar apoi sterg toate hash-urile lui din Hash ring.

    Realoc Hash Ring-ul, sterg serverul din vectorul de servere, si scad si numarul de servere din vector.

    Apoi, Parcurg tot hash_table-ul serverului pe care l-am sters din vectorul de servere, si adaug produsele pe Hash Ring, de data aceasta, acestea se vor stoca in alt server decat cel initial.

    Realoc si vector-ul de servere, dupa care eliberez memoria ocupata de serverul sters.
    