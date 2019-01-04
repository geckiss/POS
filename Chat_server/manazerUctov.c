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

int prihlasenie(const char* prihlasNick, const char* prihlasHeslo, void* prihlaseni_p, void* registrovani_p) {
    uzivatel** prihlaseni = (uzivatel**)prihlaseni_p;
    uzivatel** registrovani = (uzivatel**)registrovani_p;
    
    if (prihlasNick > 0 && prihlasHeslo > 0) {
        for (int i = 0; i < pocetAktualPrihlasenych; i++) {
            if (strcmp(registrovani[i]->nick, prihlasNick) == 0) {
                if (strcmp(registrovani[i]->heslo, prihlasHeslo) == 0) {
                    // Prihlasenie
                    prihlaseni[pocetAktualPrihlasenych++] = registrovani[i];
                    return pocetAktualPrihlasenych;
                }
            }
        }
    }
    return 0;
}

int odhlasenie(const char* odhlasNick, void* prihlaseni_p) {
    uzivatel** prihlaseni = (uzivatel**)prihlaseni_p;
    for (int i = 0; i < pocetAktualPrihlasenych; i++) {
        if (prihlaseni[i]->nick == odhlasNick) {
            swapOdhlasenie(i, prihlaseni);
            prihlaseni[pocetAktualPrihlasenych-1] = NULL;
            pocetAktualPrihlasenych--;
            return 1;
        }
    }
    return 0;
}

int registracia(const char* registrujNick, const char* registrujHeslo, uzivatel** registrovani, int socket) {
    if (registrujNick > 0 && registrujHeslo > 0) {
        for (int i = 0; i < pocetRegistrovanych; i++) {
            // Uz je registrovany
            if (strcmp(registrovani[i]->nick, registrujNick) == 0) {
                return 2;
            }
        }
        printf("Kontrola OK\n");
        uzivatel* novy = (uzivatel*)malloc(sizeof(uzivatel));
        printf("novy uzivatel OK\n");
        novy->nick = (char*)malloc(sizeof(char) * 20);
        strcpy(novy->nick, (char*)registrujNick);
        printf("novy nick OK\n");
        novy->heslo = (char*)malloc(sizeof(char) * 20);
        strcpy(novy->heslo, (char*)registrujHeslo);
        printf("nove heslo OK\n");
        novy->priatelia = (char**)malloc(sizeof(char*)*(POCET_UZIVATELOV-1));
        printf("novy priatelia OK\n");
        // potom este malokovat ten char*(nick), dostane smernik, a ten priradit do priatelov
        novy->pocetPriatelov = 0;
        novy->cisloSocketu = socket;
        registrovani[pocetRegistrovanych++] = novy;
        printf("%d\n", pocetRegistrovanych);
        return 1;
    }
    return 0;
}

int zrusenieUctu(const char* zrusNick, void* registrovani_p) {
    uzivatel** registrovani = (uzivatel**)registrovani_p;
    if (zrusNick > 0) {
        for (int i = 0; i < pocetRegistrovanych; i++) {
            if (registrovani[i]->nick == zrusNick) {
                
                swapZrusenie(i, registrovani);      // dam ho na koniec pola
                
                pocetRegistrovanych--;
                free(registrovani[pocetRegistrovanych]->nick);
                free(registrovani[pocetRegistrovanych]->heslo);
                free(registrovani[pocetRegistrovanych]->priatelia);
                free(registrovani[pocetRegistrovanych]);
                registrovani[pocetRegistrovanych] = NULL;
                
                // TODO vsetky smerniky na pouzivatela z inych poli zrusit ???...
                return 1;
            }
        }
    } else {
        return 0;
    }
}

void swapOdhlasenie(int indexNekoncovy, void* prihlaseni_p) {
    uzivatel** prihlaseni = (uzivatel**)prihlaseni_p;
    if (indexNekoncovy != pocetAktualPrihlasenych-1) {
        uzivatel* pom = prihlaseni[indexNekoncovy];
        prihlaseni[indexNekoncovy] = prihlaseni[pocetAktualPrihlasenych-1];
        prihlaseni[pocetAktualPrihlasenych-1] = pom;
    }
}

void swapZrusenie(int indexNekoncovy, void* registrovani_p) {
    uzivatel** registrovani = (uzivatel**)registrovani_p;
    if (indexNekoncovy != pocetRegistrovanych-1) {
        uzivatel* pom = registrovani[indexNekoncovy];
        registrovani[indexNekoncovy] = registrovani[pocetRegistrovanych-1];
        registrovani[pocetRegistrovanych-1] = pom;
    }
}

void* pridajPriatela(void* pdata) {
    // este skontrolovat ci to smeruje na dobre miesto
    half_ziadost* wrapper = (half_ziadost*)pdata;
    
    // komu pridava koho
    uzivatel* komu = wrapper->ziadost->komu;
    uzivatel* koho = wrapper->ziadost->koho;
    char* msgKomu;
    char* msgKoho;
    
    // seba si pridat nemozem
    if (komu->nick == koho->nick) {
        return 0;
    }
    
    for (int i = 0; i < komu->pocetPriatelov; i++) {
        // ak uz ho ma v priateloch
        if (komu->priatelia[i] == koho->nick) {
            return 0;
        }
    }
    
    // ci je prihlaseny sa zistovalo v main-e
    msgKoho = komu->nick;
    strcpy(msgKomu, " si Vas chce pridat do priatelov? Suhlasite?( Y/N )");
    write(wrapper->ziadost->koho->cisloSocketu, msgKoho, strlen(msgKoho) + 1);
    read(wrapper->ziadost->koho->cisloSocketu, msgKoho, 2);
    
    msgKomu = koho->nick;
    if (*msgKoho == 'Y') {
        strcpy(msgKomu, " suhlasil s pridanim do priatelov.");  
        // Pridam ho do zoznamu priatelov druheho
        komu->priatelia[komu->pocetPriatelov++] = koho->nick;
        koho->priatelia[koho->pocetPriatelov++] = komu->nick;
    } else {
        strcpy(msgKomu, " nesuhlasil s pridanim do priatelov.");
    }
    write(wrapper->odpoved_socket, msgKomu, strlen(msgKomu) + 1);
    
    msgKomu = koho->nick;
    strcpy(msgKomu, " bol pridany do priatelov.");
    write(wrapper->odpoved_socket, msgKomu, strlen(msgKomu) + 1);
    
    return NULL;  
}

void* zrusPriatela(void* pdata) {
    half_ziadost* wrapper = (half_ziadost*)pdata;
    uzivatel* komu = wrapper->ziadost->komu;
    uzivatel* koho = wrapper->ziadost->koho;
    int suPriatelia = 0;
    
    // seba si vymazat nemozem
    if (komu->nick == koho->nick) {
        return NULL;
    }
    
    for (int i = 0; i < komu->pocetPriatelov; i++) {
        // ak ho ma v priateloch
        if (komu->priatelia[i] == koho->nick) {
            suPriatelia = 1;
            //treab tu pomocnu prem?
            //char* ovany = odstranujuci->priatelia[i];
            komu->priatelia[i] = komu->priatelia[komu->pocetPriatelov-1];
            //odstranujuci->priatelia[odstranujuci->pocetPriatelov-1] = ovany;
            komu->priatelia[komu->pocetPriatelov] = NULL;
            
            komu->pocetPriatelov--;
            

            // TODO odstranovany o tom musi byt informovany
            // TODO teda socket
            
            break;
        }
    }
    
    if (suPriatelia) {
        // z odstranovaneho priatelov musi byt odstraneny odstranujuci   
        for (int i = 0; i < koho->pocetPriatelov; i++) {
            if (koho->priatelia[i] == komu->nick) {
                //char* ujuci = odstranovany->priatelia[i];
                koho->priatelia[i] = koho->priatelia[koho->pocetPriatelov-1];
                //odstranovany->priatelia[odstranovany->pocetPriatelov-1] = ujuci;
                koho->priatelia[koho->pocetPriatelov] = NULL;
                koho->pocetPriatelov--;
            
                return NULL;
            }
        }
    } else {
        return NULL;
    } 
    
    return NULL;
}

void* posliZiadosti(void* pdata) {
    full_ziadost* fz = (full_ziadost*)pdata;
    char *odpoved_clienta, *msg;       // Y/N
    int maZiadosti, n;
    
    if (fz->pocetZiadosti > 0) {
        maZiadosti = 1;
    } else {
        maZiadosti = 0;
    }
    
    if (maZiadosti) {
        msg = "Z|";
        sprintf(msg, "%d", fz->pocetZiadosti);
    } else {
        msg = "OK|";
    }
    
    
    n = write(fz->odpoved_socket, msg, strlen(msg)+1);
    n = read(fz->odpoved_socket, odpoved_clienta, 2);   // len OK
    
    // poslem pocet priatelov
    int c = fz->komu->pocetPriatelov; 
    char buff[20];
    n = write(fz->odpoved_socket, c, 2);
    
    for (int i = 0; i < fz->komu->pocetPriatelov; i++) {
        strcat(buff, fz->komu->priatelia[i]);
        n = write(fz->odpoved_socket, buff, strlen(buff)+1);
    }
    
    
    
    for (int i = 0; i < fz->pocetZiadosti; i++) {
        // sockety na clienta
        if (fz->komu->nick == fz->ziadosti[i]->komu->nick) {
            // ziadost o priatelstvo
            // socket na clienta
            // char* msg = "Mate ziadost o priatelstvo od " + fz->ziadosti[i]->odKoho;
            msg = "Mate ziadost o priatelstvo od ";
            strcat(msg, fz->ziadosti[i]->koho->nick);
            
            // poslem mu spravu
            n = write(fz->odpoved_socket, msg, strlen(msg)+1);
            // cakam, ci Y priatelstvo alebo N
            n = read(fz->odpoved_socket, odpoved_clienta, 1);       // 2 ? null terminating ?
            
        }
    }
   
}

void vypisPriatelov(void* uziv) {
    uzivatel* uziv_p = (uzivatel*)uziv;
    for (int i = 0; i < uziv_p->pocetPriatelov; i++) {
        printf("%d %s", i, uziv_p->priatelia[i]);
    }
}

void* najdiUziPodlaNicku(const char* nick, void* registrovani_p) {
    uzivatel** registrovani = (uzivatel**)registrovani_p;
    for(int i = 0; i < pocetRegistrovanych; i++) {
        if (nick == registrovani[i]->nick) {
            return registrovani[i];
        }
    }
    
    return NULL;
}

void* jePrihlaseny(const char* nick, void* prihlaseni_p) {
    uzivatel** prihlaseni = (uzivatel**)prihlaseni_p;
    for(int i = 0; i < pocetAktualPrihlasenych; i++) {
        if (nick == prihlaseni[i]->nick) {
            return prihlaseni[i];
        }
    }
    
    return NULL;
}

void* obsluzClienta(void *pdata) {
    char buffer[256];
    client_data* client = (client_data*)pdata;
    bzero(buffer, 256);

    int n = read(client->socket, buffer, 255);
    if (n < 0) {
        perror("Error reading from socket\n");
        return NULL;
    }

    printf("precital som zo socketu\n");
    strcpy(client->save_buffer, buffer);

    // Rozlisujem prihlasenie, registraciu, odhlasenie, vymazanie uctu, 
    // pridaj priatela(A), zrus priatela(Z), spravu(S)
    //printf(buffer);
    if ((client->buffer)[0] == 'P' || (client->buffer)[0] == 'R' || 
        (client->buffer)[0] == 'O' || (client->buffer)[0] == 'V' || 
        (client->buffer)[0] == 'A' || (client->buffer)[0] == 'Z' || 
        (client->buffer)[0] == 'S') {

        // odstranim option, buffer bude iba option
        char *option = strtok(client->buffer, "|");
        int uspechVymaz;
        char *komu_nick;
        char *koho_nick;
        uzivatel *komu;
        uzivatel *koho;
        
        switch (*option) {
            case 'P':
                client->nick = strtok(NULL, "|");
                client->heslo = strtok(NULL, "|");
                client->uspech = prihlasenie(client->nick, client->heslo, client->prihlaseni, client->registrovani);
                if (client->uspech) {
                    client->msg = "Boli ste uspesne prihlaseny.";

                    full_ziadost* fz = (full_ziadost*)malloc(sizeof (full_ziadost));
                    fz->komu = client->prihlaseni[client->uspech - 1];
                    fz->ziadosti = client->ziadosti_op;
                    fz->pocetZiadosti = pocetZiadostiOP;
                    fz->odpoved_socket = client->socket;

                    posliZiadosti(fz);

                    free(fz);

                } else {
                    client->msg = "NOK";
                    n = write(client->socket, client->msg, strlen(client->msg) + 1);
                }
                break;

            case 'R':
                client->nick = strtok(NULL, "|");
                client->heslo = strtok(NULL, "|");
                client->uspech = registracia((const char*)client->nick, (const char*)client->heslo, client->registrovani, client->socket);
                if (client->uspech) { // 1
                    strcpy(client->msg, "OK");
                    // TODO server-side: automaticky ho prihlas?
                } else {
                    if (client->uspech == 2) {
                        // duplicitny nick
                        strcpy(client->msg, "DUP");
                    } else {
                        // 0
                        strcpy(client->msg, "NOK");
                    }
                }
                n = write(client->socket, client->msg, strlen(client->msg) + 1);
                break;

            case 'O':
                client->uspech = odhlasenie(client->nick, client->prihlaseni);
                if (client->uspech) {
                    client->msg = "Boli ste uspesne odhlaseny.";
                } else {
                    client->msg = "Odhlasenie NEUSPESNE, zly nick!";
                }
                break;

            case 'V':
                
                uspechVymaz = 0;
                client->nick = strtok(NULL, "|");
                int uspechOdhlas = odhlasenie(client->nick, client->prihlaseni);
                if (uspechOdhlas) {
                    uspechVymaz = zrusenieUctu(client->nick, client->registrovani);
                }
                if (uspechVymaz) {
                    client->msg = "OK";
                } else {
                    client->msg = "NOK";
                }
                n = write(client->socket, client->msg, strlen(client->msg) + 1);
                break;

            case 'A':

                komu_nick = strtok(NULL, "|");
                koho_nick = strtok(NULL, "|");

                komu = najdiUziPodlaNicku((const char*) komu_nick, client->registrovani);
                koho = najdiUziPodlaNicku((const char*) koho_nick, client->registrovani);

                if (komu != NULL && koho != NULL) {

                    ziadost_op* novaZiadost = (ziadost_op*) malloc(sizeof (ziadost_op));
                    novaZiadost->komu = komu;
                    novaZiadost->koho = koho;

                    half_ziadost* wrapper = (half_ziadost*) malloc(sizeof (half_ziadost));
                    wrapper->ziadost = novaZiadost;
                    wrapper->odpoved_socket = client->socket;

                    if (jePrihlaseny((const char*) koho_nick, client->prihlaseni)) {
                        pridajPriatela(wrapper);
                    } else {
                        // ulozim ziadost
                        client->ziadosti_op[pocetZiadostiOP++] = novaZiadost;
                        // TODO tieto ziadosti niekde vymazat
                    }

                    //uspechPridaj = pridajPriatela(&wrapper);
                }
                break;

            case 'Z':
                komu_nick = strtok(NULL, "|");
                koho_nick = strtok(NULL, "|");


                komu = najdiUziPodlaNicku((const char*) komu_nick, client->registrovani);
                koho = najdiUziPodlaNicku((const char*) koho_nick, client->registrovani);

                if (komu != NULL && koho != NULL) {

                    ziadost_op* novaZiadost = (ziadost_op*) malloc(sizeof(ziadost_op));
                    novaZiadost->komu = komu;
                    novaZiadost->koho = koho;

                    half_ziadost* wrapper = (half_ziadost*) malloc(sizeof(half_ziadost));
                    wrapper->ziadost = novaZiadost;
                    wrapper->odpoved_socket = client->socket;

                    if (jePrihlaseny((const char*) koho_nick, client->prihlaseni)) {
                        zrusPriatela(wrapper);
                    } else {
                        // ulozim ziadost
                        client->ziadosti_zp[pocetZiadostiZP++] = wrapper;
                        // TODO tieto ziadosti niekde vymazat
                    }

                }
                break;

            case 'S':
                komu_nick = strtok(NULL, "|");
                koho_nick = strtok(NULL, "|");
                client->userMsg = client->buffer;

                koho = jePrihlaseny((const char*) koho_nick, client->prihlaseni);
                komu = jePrihlaseny((const char*) komu_nick, client->prihlaseni);

                if (komu != NULL) {
                    // poslem spravu
                    write(komu->cisloSocketu, client->userMsg, strlen(client->userMsg) + 1);
                    n = write(client->socket, "OK", 3);     // mozno do prec
                    // to ide tomu, co napisal spravu

                } else {
                    n = write(client->socket, "OFF", 4);
                    // je offline, ulozim si spravu
                    message* newMessage = (message*) malloc(sizeof (message));
                    newMessage->komu = komu;
                    newMessage->koho = koho;
                    newMessage->msg = client->userMsg;

                    client->messages[pocetSprav++] = newMessage;
                    // TODO ak sa prihlasi, vamazat danu spravu(asi swap)
                }
                break;
        }

        if (n < 0) {
            perror("Chyba zapisovania do socketu");
            // return ???
        }

    } else {
        printf("Zly format socketu - zle option: %s\n", client->buffer);
    }
}