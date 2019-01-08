/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "manazerUctov.h"

/*
typedef struct uzivatel_d {
    char* nick;                     // Dvaja s rovnakym nickom nemozu byt v chate
    char* heslo;    
    char** priatelia;               // Nick je unikatny, mame len 1 zoznam registrovanych
    int pocetPriatelov;
    int cisloSocketu;
} uzivatel;

typedef struct ziadost_op_d {       // o priatelstvo
    uzivatel* komu;
    uzivatel* koho;
} ziadost_op;

typedef struct ziadost_pp_d {       // pridaj priatela
    ziadost_op* ziadost;
    int odpoved_socket;
} half_ziadost;

typedef struct full_ziadosti_p {
    uzivatel* komu;
    ziadost_op** ziadosti;
    int pocetZiadosti;
    int odpoved_socket;
} full_ziadost;
 */
//ziadost_op** ziadosti;
//uzivatel** registrovani;
//uzivatel** prihlaseni;

int pocetAktualPrihlasenych = 0;
int pocetRegistrovanych = 0;
int pocetZiadostiOP = 0;
int pocetZiadostiZP = 0;
int pocetSprav = 0;

//struct sockaddr_in serv_addr;

void* najdiUziPodlaNicku(char* nick, uzivatel** registrovani) {
    for (int i = 0; i < pocetRegistrovanych; i++) {
        if (strcmp(nick, registrovani[i]->nick) == 0) {
            return registrovani[i];
        }
    }

    return NULL;
}

void* jePrihlaseny(char* nick, uzivatel** prihlaseni_p) {
    uzivatel** prihlaseni = (uzivatel**) prihlaseni_p;
    for (int i = 0; i < pocetAktualPrihlasenych; i++) {
        if (strcmp(nick, prihlaseni[i]->nick) == 0) {
            return prihlaseni[i];
        }
    }

    return NULL;
}

void* obsluzClienta(void *pdata) {
    int n;
    int koniec = 0;
    char buffer[256];
    char save_buffer[256];
    char msg[21];
    char help_buff[5];
    int uspechVymaz, uspechOdhlas;
    
    char *komu_nick;
    char *koho_nick;
    uzivatel *komu;
    uzivatel *koho;
    uzivatel* prihlaseny;
    thread_data* thread_d = (thread_data*)pdata;
    client_data* client = thread_d->my_client;

    while (koniec == 0) {
        printf("Som na zaciatku cyklu obsluhy\n");
        n = 0;
        bzero(buffer, 256);
        
        printf("Cakam na socket\n");
        n = read(client->socket, buffer, 255);
        printf("precital som zo socketu\n\n");
        if (n < 0) {
            perror("Error reading from socket\n");
            return NULL;
        }

        // Rozlisujem prihlasenie, registraciu, odhlasenie, vymazanie uctu, 
        // pridaj priatela(A), zrus priatela(Z), spravu(S)

        char *option = strtok(buffer, "|");
        if (*option == 'P' || *option == 'R' ||
                *option == 'O' || *option == 'V' ||
                *option == 'A' || *option == 'Z' ||
                *option == 'S' || *option == 'F') {

            switch (*option) {
                case 'P':
                    client->nick = strtok(NULL, "|");
                    client->heslo = strtok(NULL, "|");
                    printf("Prihlasujem %s, heslo je %s\n", client->nick, client->heslo);

                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        if (strcmp(client->registrovani[i]->nick, client->nick) == 0) {
                            printf("Dobry nick\n");
                            if (strcmp(client->registrovani[i]->heslo, client->heslo) == 0) {
                                printf("Dobre heslo\n");
                                
                                // Prihlasenie
                                pthread_mutex_lock(thread_d->mutex_prihlas);
                                pthread_mutex_lock(thread_d->mutex_register);
                                client->prihlaseni[pocetAktualPrihlasenych] = client->registrovani[i];
                                prihlaseny = client->registrovani[i];
                                ++pocetAktualPrihlasenych;
                                pthread_mutex_unlock(thread_d->mutex_register);
                                pthread_mutex_unlock(thread_d->mutex_prihlas);
                                client->uspech = pocetAktualPrihlasenych;
                            }
                        }
                    }

                    if (client->uspech != 0) {
                        printf("Posielam, ze prihlasenie uspesne\n");
                        strcpy(msg, "OK");
                        n = write(client->socket, msg, strlen(msg) + 1);
                        usleep(500000);

                        //full_ziadost* fz = (full_ziadost*)malloc(sizeof (full_ziadost));
                        //fz->komu = client->prihlaseni[client->uspech - 1];
                        //printf("Nick prihlaseneho je %s\n", fz->komu->nick);
                        //fz->ziadosti = client->ziadosti_op;
                        //fz->pocetZiadosti = pocetZiadostiOP;
                        //fz->odpoved_socket = client->socket;

                        pthread_mutex_lock(thread_d->mutex_prihlas);
                        prihlaseny = client->prihlaseni[client->uspech - 1];
                        
                        if (prihlaseny->pocetZiadostiOP > 0) {
                            strcpy(msg, "Z|");
                            sprintf(help_buff, "%d", prihlaseny->pocetZiadostiOP);
                            strcat(msg, help_buff);
                            printf("Sprava je %s\n", msg);
                            strcpy(help_buff, "");
                        } else {
                            strcpy(msg, "OK|");
                            printf("Sprava je %s\n", msg);
                        }
                        pthread_mutex_unlock(thread_d->mutex_prihlas);
                        
                        n = write(client->socket, msg, strlen(msg) + 1);
                        printf("Poslal som ze ci ma ziadosti\n");
                        usleep(500000);
                        
                        char* ziadajuci;
                        pthread_mutex_lock(thread_d->mutex_ziadosti_op);
                        int pocetZiadOP = prihlaseny->pocetZiadostiOP;
                        pthread_mutex_unlock(thread_d->mutex_ziadosti_op);
                        for (int i = 0; i < pocetZiadOP; i++) {
                            pthread_mutex_lock(thread_d->mutex_ziadosti_op);
                            ziadajuci = prihlaseny->ziadosti_op[i];
                            pthread_mutex_unlock(thread_d->mutex_ziadosti_op);
                                strcpy(buffer, "Mate ziadost o priatelstvo od ");
                                strcat(buffer, ziadajuci);

                                n = write(client->socket, buffer, strlen(buffer) + 1);
                                printf("Poslal som mu ziadost o priatelstvo\n");

                                bzero(buffer, 256);
                                n = read(client->socket, buffer, 20); // 2 ? null terminating ?
                                printf("Prisla mi odpoved\n");

                                if (strcmp(buffer, "Y") == 0) {
                                    printf("Pridavam ich do priatelov\n");
                                    pthread_mutex_lock(thread_d->mutex_prihlas);
                                    prihlaseny->priatelia[prihlaseny->pocetPriatelov++] = ziadajuci;
                                    pthread_mutex_unlock(thread_d->mutex_prihlas);
                                    // TODO este ziadajucemu
                                }

                        }
                        
                    } else {
                        strcpy(msg, "NOK");
                        n = write(client->socket, msg, strlen(msg) + 1);
                    }

                    break;

                case 'R':
                    client->nick = strtok(NULL, "|");
                    client->heslo = strtok(NULL, "|");
                    printf("Registrujem ho\n");

                    client->uspech = 1;
                    pthread_mutex_lock(thread_d->mutex_register);
                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        // Uz je registrovany
                        if (strcmp(client->registrovani[i]->nick, client->nick) == 0) {
                            printf("Uzivatel uz existuje\n");
                            client->uspech = 2;
                            pthread_mutex_unlock(thread_d->mutex_register);
                            break;
                        }
                    }
                    
                    if (client->uspech != 2) {
                        pthread_mutex_unlock(thread_d->mutex_register);
                    }
                    //client->uspech = registracia((const char*) client->nick, (const char*) client->heslo, client->registrovani, client->socket);
                    if (client->uspech == 1) { // 1
                        uzivatel* novy = (uzivatel*) malloc(sizeof (uzivatel));

                        novy->nick = (char*) malloc(sizeof (char) * 20);
                        strcpy(novy->nick, (char*) client->nick);

                        novy->heslo = (char*) malloc(sizeof (char) * 20);
                        strcpy(novy->heslo, (char*) client->heslo);

                        novy->priatelia = (char**) malloc(sizeof (char*)*(POCET_UZIVATELOV - 1));
                        novy->pocetPriatelov = 0;
                        
                        novy->ziadosti_op = (char**)malloc(sizeof(char*)*(POCET_UZIVATELOV-1));
                        novy->pocetZiadostiOP = 0;
                        
                        novy->ziadosti_zp = (char**)malloc(sizeof(char*)*(POCET_UZIVATELOV-1));
                        novy->pocetZiadostiZP = 0;
                        
                        novy->messages = (message**)malloc(sizeof(message*)*POCET_UZIVATELOV);
                        novy->pocetSprav = 0;
                                
                        novy->cisloSocketu = client->socket;

                        pthread_mutex_lock(thread_d->mutex_register);
                        client->registrovani[pocetRegistrovanych] = novy;
                        ++pocetRegistrovanych;
                        printf("Registracia ok, count: %d\n", pocetRegistrovanych);
                        pthread_mutex_unlock(thread_d->mutex_register);
                        strcpy(msg, "OK");
                    } else {
                        if (client->uspech == 2) {
                            // duplicitny nick
                            strcpy(msg, "DUP");
                        } else {
                            // 0
                            strcpy(msg, "NOK");
                        }
                    }
                    n = write(client->socket, msg, strlen(msg) + 1);
                    break;

                case 'O':
                    client->nick = strtok(NULL, "|");
                    //koniec = 1;
                    //client->uspech = odhlasenie(client->nick, client->prihlaseni);
                    client->uspech = 0;
                    printf("Nick: %s\n", client->nick);
                    printf("Prihlasenych: %d", pocetAktualPrihlasenych);
                    pthread_mutex_lock(thread_d->mutex_prihlas);
                    for (int i = 0; i < pocetAktualPrihlasenych; i++) {
                        if (strcmp(client->prihlaseni[i]->nick, client->nick) == 0) {
                            
                            //swap
                            if (i != pocetAktualPrihlasenych - 1) {
                                uzivatel* pom = client->prihlaseni[i];
                                client->prihlaseni[i] = client->prihlaseni[pocetAktualPrihlasenych - 1];
                                client->prihlaseni[pocetAktualPrihlasenych - 1] = pom;
                            }
                            
                            client->prihlaseni[pocetAktualPrihlasenych - 1] = NULL;
                            pocetAktualPrihlasenych--;
                            client->uspech = 1;
                            break;  // keby nejde, toto zmaz
                        }
                        
                    }
                    pthread_mutex_unlock(thread_d->mutex_prihlas);
                    
                    if (client->uspech == 1) {
                        strcpy(msg, "OK");
                        printf("Odhlasujem %s\n", client->nick);
                    } else {
                        strcpy(msg, "NOK");
                        printf("Odhlasovanie %s NEUSPESNE\n", client->nick);
                    }
                    n = write(client->socket, msg, strlen(msg) + 1);
                    //usleep(500000);
                    break;

                case 'V':
                    client->nick = strtok(NULL, "|");
                    uspechOdhlas = 0;
                    pthread_mutex_lock(thread_d->mutex_prihlas);
                    for (int i = 0; i < pocetAktualPrihlasenych; i++) {
                        if (strcmp(client->prihlaseni[i]->nick, client->nick) == 0) {
                            //swap
                            if (i != pocetAktualPrihlasenych - 1) {
                                uzivatel* pom = client->prihlaseni[i];
                                client->prihlaseni[i] = client->prihlaseni[pocetAktualPrihlasenych - 1];
                                client->prihlaseni[pocetAktualPrihlasenych - 1] = pom;
                            }
                            
                            client->prihlaseni[pocetAktualPrihlasenych - 1] = NULL;
                            pocetAktualPrihlasenych--;
                            printf("Aktualne prihlasenych: %d\n", pocetAktualPrihlasenych);
                            uspechOdhlas = 1;
                            break;  // keby nejde, toto zmaz
                        }
                    }
                    pthread_mutex_unlock(thread_d->mutex_prihlas);
                    
                    if (uspechOdhlas == 1) {
                        uspechVymaz = 0;
                        
                        pthread_mutex_lock(thread_d->mutex_register);
                        printf("Registrovanich pred zrusenim: %d\n", pocetRegistrovanych);
                        for (int i = 0; i < pocetRegistrovanych; i++) {
                            if (strcmp(client->registrovani[i]->nick, client->nick) == 0) {

                                //swap
                                if (i != pocetRegistrovanych - 1) {
                                    uzivatel* pom = client->registrovani[i];
                                    client->registrovani[i] = client->registrovani[pocetRegistrovanych - 1];
                                    client->registrovani[pocetRegistrovanych - 1] = pom;
                                }
                                --pocetRegistrovanych;
                                free(client->registrovani[pocetRegistrovanych]->nick);
                                free(client->registrovani[pocetRegistrovanych]->heslo);
                                free(client->registrovani[pocetRegistrovanych]->priatelia);
                                free(client->registrovani[pocetRegistrovanych]);
                                client->registrovani[pocetRegistrovanych] = NULL;

                                // TODO vsetky smerniky na pouzivatela z inych poli zrusit ???...
                                uspechVymaz = 1;
                                break;      // keby nejde, toto vymaz
                            }
                        }
                        pthread_mutex_unlock(thread_d->mutex_register);
                        
                        if (uspechVymaz == 1) {
                            strcpy(msg, "OK");
                            printf("Ucet %s zruseny\n", client->nick);
                        } else {
                            strcpy(msg, "NOK");
                            printf("Ucet %s NEZRUSENY\n", client->nick);
                        }
                    } else {
                        strcpy(msg, "NOK");
                        printf("Ucet %s NEZRUSENY, problem s odhlasovanim\n", client->nick);
                    }
                    
                    n = write(client->socket, msg, strlen(msg) + 1);
                    break;

                case 'A':
                    strcpy(help_buff, "");
                    pthread_mutex_lock(thread_d->mutex_ziadosti_op);
                    sprintf(help_buff, "%d", prihlaseny->pocetZiadostiOP);
                    printf("Posielam mu pocet ziadosti o priatelstvo, to je: %d\n", prihlaseny->pocetZiadostiOP);
                    pthread_mutex_unlock(thread_d->mutex_ziadosti_op);
                    
                    n = write(client->socket, help_buff, strlen(help_buff) + 1);
                    usleep(500000);
                    
                    uzivatel* ziadajuci;
                    char *ziadajuci_nick;
                    int minusZiadosti = 0;
                    printf("Prihlaseny je %s\n", prihlaseny->nick);
                    
                    pthread_mutex_lock(thread_d->mutex_ziadosti_op);
                    for (int i = 0; i < prihlaseny->pocetZiadostiOP; i++) {
                        ziadajuci_nick = prihlaseny->ziadosti_op[i];
                        printf("Ziadajuci je %s\n", ziadajuci_nick);
                        
                        if (ziadajuci_nick != NULL) {
                            pthread_mutex_lock(thread_d->mutex_register);
                            ziadajuci = najdiUziPodlaNicku(ziadajuci_nick, client->registrovani);
                            pthread_mutex_unlock(thread_d->mutex_register);
                            
                            if (ziadajuci != NULL) {
                                bzero(msg, 21);
                                strcpy(msg, ziadajuci->nick);
                                n = write(client->socket, msg, strlen(msg) + 1);
                                n = read(client->socket, msg, 2);
                                printf("Odpoved je %s\n", msg);
                                
                                if (strcmp(msg, "Y") == 0) {
                                    pthread_mutex_lock(thread_d->mutex_prihlas);
                                    
                                    prihlaseny->priatelia[prihlaseny->pocetPriatelov] = (char*)malloc(sizeof(char)*21);
                                    strcpy(prihlaseny->priatelia[prihlaseny->pocetPriatelov++], ziadajuci->nick);
                                    // swapnem ju nakoniec, aby som ju mohol vymazat
                                    char *pom = prihlaseny->ziadosti_op[i];
                                    prihlaseny->ziadosti_op[i] = prihlaseny->ziadosti_op[prihlaseny->pocetZiadostiOP - 1];
                                    prihlaseny->ziadosti_op[prihlaseny->pocetZiadostiOP - 1] = pom;
                                    prihlaseny->ziadosti_op[prihlaseny->pocetZiadostiOP - 1] = NULL;

                                    ziadajuci->priatelia[ziadajuci->pocetPriatelov] = (char*)malloc(sizeof(char)*21);
                                    strcpy(ziadajuci->priatelia[ziadajuci->pocetPriatelov++], prihlaseny->nick);
                                    printf("%s a %s su priatalia!\n", prihlaseny->nick, ziadajuci->nick);
                                    
                                    pthread_mutex_unlock(thread_d->mutex_prihlas);
                                    minusZiadosti++;
                                }
                            }
                        }
                    }
                    
                    prihlaseny->pocetZiadostiOP -= minusZiadosti;
                    pthread_mutex_unlock(thread_d->mutex_ziadosti_op);
                    printf("Ziadosti o priatelstvo vybavene\n");
                    
                    // poslem pocet registrovanych, lebo zvolil moznost pridat
                    // do priatelov, tak sa mu vypise zoznam registrovanych
                    // z ktorych si vyberie nick toho, s ktorym sa chce zpriatelit
                    strcpy(help_buff, "");
                    pthread_mutex_lock(thread_d->mutex_register);
                    sprintf(help_buff, "%d", pocetRegistrovanych);
                    printf("Posielam mu %d registrovanych\n", pocetRegistrovanych);
                    pthread_mutex_unlock(thread_d->mutex_register);
                    n = write(client->socket, help_buff, strlen(help_buff) + 1);
                    
                    pthread_mutex_lock(thread_d->mutex_register);
                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        usleep(200000);
                        // nie je to original ziadajuci... len pomocna premenna
                        ziadajuci_nick = client->registrovani[i]->nick;
                        
                        if (strcmp(client->registrovani[i]->nick, prihlaseny->nick) != 0) {
                            n = write(client->socket, client->registrovani[i]->nick, strlen(client->registrovani[i]->nick) + 1); 
                        }
                    }
                    pthread_mutex_unlock(thread_d->mutex_register);
                    
                    n = read(client->socket, buffer, 42);
                    // komu_nick nie je nahodou client->nick ?
                    komu_nick = strtok(buffer, "|");        // pridavajuci
                    koho_nick = strtok(NULL, "|");          // pridavany
                    
                    printf("%s si chce pridat %s\n", komu_nick, koho_nick);
                    strcpy(msg, "OK");
                    n = write(client->socket, msg, strlen(msg) + 1);
                    komu = prihlaseny;
                    
                    pthread_mutex_lock(thread_d->mutex_register);
                    koho = najdiUziPodlaNicku(koho_nick, client->registrovani);
                    pthread_mutex_unlock(thread_d->mutex_register);
                    
                    if (koho != NULL) {
                        char* msgKomu;
                        char* msgKoho;
                        int oka = 1;
                            
                        if (strcmp(komu->nick, koho->nick) == 0) {
                            strcpy(msg, "NOK");
                            printf("Seba si pridat nemozem\n");
                            n = write(client->socket, msg, strlen(msg) + 1);
                            oka = 0;
                        }

                        if (oka == 1) {
                            // ak uz ho ma v priateloch
                            pthread_mutex_lock(thread_d->mutex_prihlas);
                            for (int i = 0; i < komu->pocetPriatelov; i++) {
                                
                                if (strcmp(komu->priatelia[i], koho->nick) == 0) {
                                    strcpy(msg, "NOK");             // DUP?
                                    printf("Uz je v priateloch\n");
                                    n = write(client->socket, msg, strlen(msg) + 1);
                                    oka = 0;
                                    break;
                                }
                            }
                            pthread_mutex_unlock(thread_d->mutex_prihlas);

                            if (oka == 1) {
                                pthread_mutex_lock(thread_d->mutex_ziadosti_op);
                                koho->ziadosti_op[koho->pocetZiadostiOP] = (char*)malloc(sizeof(char)*20);
                                strcpy(koho->ziadosti_op[koho->pocetZiadostiOP++], komu_nick);
                                pthread_mutex_unlock(thread_d->mutex_ziadosti_op);
                            }
                        } 
                    } else {
                        printf("Zly nick pri pridavani do priatelov: %s neexistuje", koho_nick);
                    }
                    break;

                case 'Z':
                    // TODo komu_nick nie je nahodou client->nick alebo prihlaseny->nick?
                    komu_nick = prihlaseny->nick;               // odstranujuci
                    
                    komu = prihlaseny;

                    strcpy(help_buff, "");
                    pthread_mutex_lock(thread_d->mutex_ziadosti_zp);
                    sprintf(help_buff, "%d", prihlaseny->pocetZiadostiZP);
                    printf("Posielam mu %d ziadosti o zrusenie priatelstva\n", prihlaseny->pocetZiadostiZP);
                    pthread_mutex_unlock(thread_d->mutex_ziadosti_zp);
                    n = write(client->socket, help_buff, strlen(help_buff) + 1);
                    usleep(200000);

                    int minusZiadostiZP = 0;
                    int minusPriatelia = 0;
                    pthread_mutex_lock(thread_d->mutex_ziadosti_zp);
                    for (int i = 0; i < prihlaseny->pocetZiadostiZP; i++) {
                        
                        bzero(msg, 21);
                        strcpy(msg, prihlaseny->ziadosti_zp[i]);
                        n = write(client->socket, msg, strlen(msg) + 1);
                        usleep(200000);

                        for (int j = 0; j < prihlaseny->pocetPriatelov; j++) {
                            
                            if (strcmp(prihlaseny->priatelia[j], prihlaseny->ziadosti_zp[i]) == 0) {
                                // swapnem ju/ho nakoniec, aby som ju/ho mohol vymazat
                                char *pom = prihlaseny->ziadosti_zp[i];
                                prihlaseny->ziadosti_zp[i] = prihlaseny->ziadosti_zp[prihlaseny->pocetZiadostiZP - 1];
                                prihlaseny->ziadosti_zp[prihlaseny->pocetZiadostiOP - 1] = pom;
                                prihlaseny->ziadosti_zp[prihlaseny->pocetZiadostiOP - 1] = NULL;

                                pom = prihlaseny->priatelia[j];
                                prihlaseny->priatelia[j] = prihlaseny->priatelia[prihlaseny->pocetPriatelov - 1];
                                prihlaseny->priatelia[prihlaseny->pocetPriatelov - 1] = pom;
                                free(prihlaseny->priatelia[prihlaseny->pocetPriatelov - 1]);

                                minusZiadostiZP++;
                                minusPriatelia++;
                            }
                        }
                    }
                    
                    
                    prihlaseny->pocetZiadostiZP -= minusZiadostiZP;
                    pthread_mutex_unlock(thread_d->mutex_ziadosti_zp);
                    prihlaseny->pocetPriatelov -= minusPriatelia;
                    printf("Ziadosti o zruseni priatelstva vybavene\n");
                    
                    strcpy(help_buff, "");
                    sprintf(help_buff, "%d", prihlaseny->pocetPriatelov);
                    printf("Posielam mu %d priatelov(pocet)\n", prihlaseny->pocetPriatelov);
                    n = write(client->socket, help_buff, strlen(help_buff) + 1);
                    usleep(500000);
                    
                    printf("Posielam nicky priatelov\n");
                    char priatel_nick[21];
                    
                    for (int i = 0; i < prihlaseny->pocetPriatelov; i++) {
                        pthread_mutex_lock(thread_d->mutex_prihlas);
                        strcpy(priatel_nick, client->prihlaseni[i]->nick);
                        pthread_mutex_unlock(thread_d->mutex_prihlas);
                        n = write(client->socket, priatel_nick, strlen(priatel_nick) + 1);
                        usleep(200000);
                    }
                    
                    printf("Posielanie nickov dokoncene. Cakam na nick\n");
                    n = read(client->socket, msg, 21);
                    koho_nick = strtok(msg, "|");              // odstranovany
                    pthread_mutex_lock(thread_d->mutex_register);
                    koho = najdiUziPodlaNicku(koho_nick, client->registrovani);
                    pthread_mutex_unlock(thread_d->mutex_register);
                    
                    printf("Spravny nick\n");
                    if (koho != NULL) {
                        pthread_mutex_lock(thread_d->mutex_ziadosti_zp);
                        koho->ziadosti_zp[koho->pocetZiadostiZP] = (char*)malloc(sizeof(char)*21);
                        printf("Vytvoril som novu ziadost o zrusenie priatelstva\n");
                        strcpy(koho->ziadosti_zp[koho->pocetZiadostiZP++], komu_nick);
                        pthread_mutex_unlock(thread_d->mutex_ziadosti_zp);
                        
                        for (int i = 0; i < prihlaseny->pocetPriatelov; i++) {
                            
                            if (strcmp(koho_nick, prihlaseny->priatelia[i]) == 0) {
                                
                                // swap na koniec
                                char *pom = prihlaseny->priatelia[i];
                                prihlaseny->priatelia[i] = prihlaseny->priatelia[prihlaseny->pocetPriatelov - 1];
                                prihlaseny->priatelia[prihlaseny->pocetPriatelov] = pom;
                                free(prihlaseny->priatelia[--prihlaseny->pocetPriatelov]);
                                printf("Zrusil som priatela\n");
                                printf("Pocet priatelov: %d\n", prihlaseny->pocetPriatelov);
                                strcpy(msg, "OK");
                                n = write(client->socket, msg, strlen(msg) + 1);
                                break;
                            }
                        }

                    } else {
                        strcpy(msg, "NOK");
                        n = write(client->socket, msg, strlen(msg) + 1);
                    }
                    break;

                case 'S':
                    // TODO vypisem mu vsetky spravy
                    komu = prihlaseny;
                    
                    bzero(buffer, 256);
                    strcpy(msg, "");
                    pthread_mutex_lock(thread_d->mutex_spravy);
                    sprintf(msg, "%d", komu->pocetSprav);
                    printf("Posielam spravy: %d sprav\n", komu->pocetSprav);
                    n = write(client->socket, msg, strlen(msg) + 1);
                    usleep(200000);
                    
                    for (int i = 0; i < komu->pocetSprav; i++) {
                        strcpy(buffer, komu->messages[i]->od_koho);
                        strcat(buffer, "|");
                        strcat(buffer, komu->messages[i]->msg);
                        n = write(client->socket, buffer, strlen(buffer) + 1);
                        usleep(200000);
                    }
                    
                    pthread_mutex_unlock(thread_d->mutex_spravy);
                    
                    usleep(5000000);        // aby si stihol precitat spravy
                    
                    // poslem pocet priatelov
                    int count = komu->pocetPriatelov;
                    printf("Pocet priatelov %s je %d\n", komu->nick, count);
                    strcpy(msg, "");
                    sprintf(msg, "%d", count);

                    n = write(client->socket, msg, 4);
                    usleep(2000000);
                    
                    for (int i = 0; i < count; i++) {
                        strcpy(buffer, prihlaseny->priatelia[i]);
                        n = write(client->socket, buffer, strlen(buffer) + 1);

                        printf("Poslal som nick priatela: %s\n", buffer);
                        usleep(2000000);
                    }
                    printf("Posielanie priatelov dokoncene\n");
                    
                    n = read(client->socket, buffer, 255);
                    koho_nick = strtok(buffer, "|");                // prijimatel
                    char *sprava = strtok(NULL, "|");
                    printf("Prijal som spravu pre %s\n", koho_nick);
                         
                    pthread_mutex_lock(thread_d->mutex_register);
                    koho = najdiUziPodlaNicku(koho_nick, client->registrovani);
                    pthread_mutex_unlock(thread_d->mutex_register);

                    // TODO spravu prijat az ak sa skontroluje koho
                    if (koho != NULL) {
                        printf("Sprava OK\n");
                        n = write(client->socket, "OK", 3);
                        
                        message* newMessage = (message*) malloc(sizeof (message));
                        newMessage->od_koho = (char*)malloc(sizeof(char)*21);
                        strcpy(newMessage->od_koho, komu_nick);
                        
                        newMessage->msg = (char*)malloc(sizeof(char)*256);
                        strcpy(newMessage->msg, sprava);

                        pthread_mutex_lock(thread_d->mutex_spravy);
                        koho->messages[koho->pocetSprav++] = newMessage;
                        pthread_mutex_unlock(thread_d->mutex_spravy);
                    } else {
                        n = write(client->socket, "NOK", 4);
                        printf("Sprava NOK\n");
                    }
                    break;
                    
                case 'F':
                    koniec = 1;
                    break;
            }

            if (n < 0) {
                perror("Chyba zapisovania do socketu");
            }

        } else {
            printf("Zly format socketu - zle option: %s\n", client->buffer);
        }
    }
    
    if (prihlaseny != NULL) {
        pthread_mutex_lock(thread_d->mutex_ziadosti_op);
        for (int i = 0; i < prihlaseny->pocetZiadostiOP; i++) {
            free(prihlaseny->ziadosti_op[i]);
        }
        free(prihlaseny->ziadosti_op);
        pthread_mutex_unlock(thread_d->mutex_ziadosti_op);
        
        pthread_mutex_lock(thread_d->mutex_ziadosti_zp);
        for (int i = 0; i < prihlaseny->pocetZiadostiZP; i++) {
            free(prihlaseny->ziadosti_zp[i]);
        }
        free(prihlaseny->ziadosti_zp);
        pthread_mutex_unlock(thread_d->mutex_ziadosti_zp);
        
        pthread_mutex_lock(thread_d->mutex_prihlas);
        for (int i = 0; i < prihlaseny->pocetSprav; i++) {
            free(prihlaseny->messages[i]->msg);
            free(prihlaseny->messages[i]->od_koho);
            free(prihlaseny->messages[i]);
        }
        free(prihlaseny->messages);
        pthread_mutex_unlock(thread_d->mutex_prihlas);
    }
    
    thread_d->skoncene = 1;
    printf("Zatvaram socket %d a idem von z vlakna\n", client->socket);
    close(client->socket);
}