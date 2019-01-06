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

int prihlasenie(char* prihlasNick, char* prihlasHeslo, uzivatel** prihlaseni, uzivatel** registrovani) {
    //if (prihlasNick > 0 && prihlasHeslo > 0) {
    for (int i = 0; i < pocetRegistrovanych; i++) {
        if (strcmp(registrovani[i]->nick, prihlasNick) == 0) {
            printf("dobry nick\n");
            if (strcmp(registrovani[i]->heslo, prihlasHeslo) == 0) {
                printf("dobre heslo\n");
                // Prihlasenie
                prihlaseni[pocetAktualPrihlasenych] = registrovani[i];
                ++pocetAktualPrihlasenych;
                return pocetAktualPrihlasenych;
            }
        }
    }
    //}
    printf("ziadna zhoda\n");
    return 0;
}

int odhlasenie(char* odhlasNick, uzivatel** prihlaseni) {
    for (int i = 0; i < pocetAktualPrihlasenych; i++) {
        if (prihlaseni[i]->nick == odhlasNick) {
            swapOdhlasenie(i, prihlaseni);
            prihlaseni[pocetAktualPrihlasenych - 1] = NULL;
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
                printf("Registracia: uzivatel uz existuje\n");
                return 2;
            }
        }
        uzivatel* novy = (uzivatel*) malloc(sizeof (uzivatel));

        novy->nick = (char*) malloc(sizeof (char) * 20);
        strcpy(novy->nick, (char*) registrujNick);

        novy->heslo = (char*) malloc(sizeof (char) * 20);
        strcpy(novy->heslo, (char*) registrujHeslo);

        novy->priatelia = (char**) malloc(sizeof (char*)*(POCET_UZIVATELOV - 1));

        novy->pocetPriatelov = 0;
        novy->cisloSocketu = socket;

        registrovani[pocetRegistrovanych++] = novy;
        printf("Registracia ok, count: %d\n", pocetRegistrovanych);
        return 1;
    }
    return 0;
}

int zrusenieUctu(char* zrusNick, uzivatel** registrovani) {
    for (int i = 0; i < pocetRegistrovanych; i++) {
        if (registrovani[i]->nick == zrusNick) {

            swapZrusenie(i, registrovani); // dam ho na koniec pola

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
    return 0;
}

void swapOdhlasenie(int indexNekoncovy, void* prihlaseni_p) {
    uzivatel** prihlaseni = (uzivatel**) prihlaseni_p;
    if (indexNekoncovy != pocetAktualPrihlasenych - 1) {
        uzivatel* pom = prihlaseni[indexNekoncovy];
        prihlaseni[indexNekoncovy] = prihlaseni[pocetAktualPrihlasenych - 1];
        prihlaseni[pocetAktualPrihlasenych - 1] = pom;
    }
}

void swapZrusenie(int indexNekoncovy, void* registrovani_p) {
    uzivatel** registrovani = (uzivatel**) registrovani_p;
    if (indexNekoncovy != pocetRegistrovanych - 1) {
        uzivatel* pom = registrovani[indexNekoncovy];
        registrovani[indexNekoncovy] = registrovani[pocetRegistrovanych - 1];
        registrovani[pocetRegistrovanych - 1] = pom;
    }
}

void* pridajPriatela(void* pdata) {
    // este skontrolovat ci to smeruje na dobre miesto
    half_ziadost* wrapper = (half_ziadost*) pdata;

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
    half_ziadost* wrapper = (half_ziadost*) pdata;
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
            komu->priatelia[i] = komu->priatelia[komu->pocetPriatelov - 1];
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
                koho->priatelia[i] = koho->priatelia[koho->pocetPriatelov - 1];
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

void* posliZiadosti(full_ziadost* fz) {
    int maZiadosti, n;

    // poslem pocet priatelov
    int c = fz->komu->pocetPriatelov;
    char buff[255];
    sprintf(buff, "%d", c);
    printf(buff);
    printf("\n");
    printf(c);
    printf("\n");

    n = write(fz->odpoved_socket, buff, 3);
    printf("poslal som pocet priatelov, %d\n", c);
    usleep(500000);
    bzero(buff, 20);
    for (int i = 0; i < fz->komu->pocetPriatelov; i++) {
        strcat(buff, fz->komu->priatelia[i]);
        n = write(fz->odpoved_socket, buff, strlen(buff) + 1);
        printf("poslal som nick priatela\n");
        usleep(500000);
    }
    printf("posielanie priatelov dokoncene\n\n");

    char *odpoved_clienta, *msg; // Y/N


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


    n = write(fz->odpoved_socket, msg, strlen(msg) + 1);
    printf("poslal som ze ci ma ziadosti\n");
    usleep(500000);
    //n = read(fz->odpoved_socket, odpoved_clienta, 2);   // len OK

    for (int i = 0; i < fz->pocetZiadosti; i++) {
        // sockety na clienta
        if (fz->komu->nick == fz->ziadosti[i]->komu->nick) {
            // ziadost o priatelstvo
            // socket na clienta
            // char* msg = "Mate ziadost o priatelstvo od " + fz->ziadosti[i]->odKoho;
            msg = "Mate ziadost o priatelstvo od ";
            strcat(msg, fz->ziadosti[i]->koho->nick);

            // poslem mu spravu
            n = write(fz->odpoved_socket, msg, strlen(msg) + 1);
            printf("poslal som mu ziadost o pria\n");
            // cakam, ci Y priatelstvo alebo N
            n = read(fz->odpoved_socket, odpoved_clienta, 1); // 2 ? null terminating ?
            printf("prisla mi odpoved\n");

        }
    }

}

void vypisPriatelov(void* uziv) {
    uzivatel* uziv_p = (uzivatel*) uziv;
    for (int i = 0; i < uziv_p->pocetPriatelov; i++) {
        printf("%d %s", i, uziv_p->priatelia[i]);
    }
}

void* najdiUziPodlaNicku(const char* nick, void* registrovani_p) {
    uzivatel** registrovani = (uzivatel**) registrovani_p;
    for (int i = 0; i < pocetRegistrovanych; i++) {
        if (nick == registrovani[i]->nick) {
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
    int koniec = 0;
    char buffer[256];
    char save_buffer[256];
    int n;
    client_data* client = (client_data*) pdata;

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

            //printf("Precital som option - je dobra\n");
            // odstranim option, buffer bude iba option

            char msg[20];
            int uspechVymaz, uspechOdhlas;
            char *komu_nick;
            char *koho_nick;
            uzivatel *komu;
            uzivatel *koho;
            char c[5];
            switch (*option) {
                case 'P':
                    client->nick = strtok(NULL, "|");
                    printf("Nick: %s\n", client->nick);
                    client->heslo = strtok(NULL, "|");
                    printf("Heslo: %s\n", client->heslo);
                    printf("Prihlasujem ho\n");
                    client->uspech = 0;

                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        if (strcmp(client->registrovani[i]->nick, client->nick) == 0) {
                            printf("Dobry nick\n");
                            if (strcmp(client->registrovani[i]->heslo, client->heslo) == 0) {
                                printf("Dobre heslo\n");
                                // Prihlasenie
                                client->prihlaseni[pocetAktualPrihlasenych] = client->registrovani[i];
                                printf("Nick prihlaseneho: %s\n", client->prihlaseni[pocetAktualPrihlasenych]->nick);
                                ++pocetAktualPrihlasenych;
                                client->uspech = pocetAktualPrihlasenych;
                            }
                        }
                    }

                    //client->uspech = prihlasenie(client->nick, client->heslo, client->prihlaseni, client->registrovani);

                    if (client->uspech != 0) {
                        printf("Posielam, ze prihlasenie uspesne\n");
                        usleep(2000000);
                        strcpy(msg, "OK");

                        n = write(client->socket, msg, strlen(msg) + 1);
                        usleep(2000000);

                        //full_ziadost* fz = (full_ziadost*)malloc(sizeof (full_ziadost));
                        //fz->komu = client->prihlaseni[client->uspech - 1];
                        //printf("Nick prihlaseneho je %s\n", fz->komu->nick);
                        //fz->ziadosti = client->ziadosti_op;
                        //fz->pocetZiadosti = pocetZiadostiOP;
                        //fz->odpoved_socket = client->socket;

                        uzivatel* prihlaseny = client->prihlaseni[client->uspech - 1];
                        int maZiadosti;

                        // poslem pocet priatelov
                        /*
                        int count = client->prihlaseni[client->uspech - 1]->pocetPriatelov;
                        printf("Pocet priatelov je %d\n", count);
                        char buff[21];
                        sprintf(buff, "%d", count);
                        printf(buff);
                        printf("\n");

                        
                        n = write(client->socket, buff, 4);
                        printf("Poslal som pocet priatelov, %d\n", count);
                        usleep(2000000);

                        for (int i = 0; i < count; i++) {
                            strcpy(buff, prihlaseny->priatelia[i]);
                            n = write(client->socket, buff, strlen(buff) + 1);

                            printf("Poslal som nick priatela: %s\n", buff);
                            usleep(2000000);
                        }
*/
                        bzero(buffer, 255);
                        //printf("Posielanie priatelov dokoncene\n");
                        //printf("Cakam na potvrdenie priatelov\n");
                        //n = read(client->socket, buffer, 30);   // len OK

                        if (pocetZiadostiOP > 0) {
                            strcpy(msg, "Z|");
                            strcat(msg, sprintf(msg, "%d", pocetZiadostiOP));
                            printf("Sprava je %s\n", msg);
                        } else {
                            strcpy(msg, "OK|");
                            printf("Sprava je %s\n", msg);
                        }

                        n = write(client->socket, msg, strlen(msg) + 1);
                        printf("Poslal som ze ci ma ziadosti\n");
                        usleep(2000000);

                        //

                        for (int i = 0; i < pocetZiadostiOP; i++) {
                            // sockety na clienta
                            if (prihlaseny->nick == client->ziadosti_op[i]->koho->nick) {
                                bzero(buffer, 255);
                                // ziadost o priatelstvo
                                // socket na clienta
                                // char* msg = "Mate ziadost o priatelstvo od " + fz->ziadosti[i]->odKoho;
                                strcpy(msg, "Mate ziadost o priatelstvo od ");
                                strcat(msg, client->ziadosti_op[i]->komu->nick);

                                // poslem mu spravu
                                n = write(client->socket, msg, strlen(msg) + 1);
                                printf("Poslal som mu ziadost o priatelstvo\n");
                                // cakam, ci Y priatelstvo alebo N
                                n = read(client->socket, buffer, 20); // 2 ? null terminating ?
                                printf("Prisla mi odpoved\n");

                                if (strcmp(buffer, "OK") == 0) {
                                    printf("Pridavam ich do priatelov\n");
                                    client->ziadosti_op[i]->komu->priatelia[client->ziadosti_op[i]->komu->pocetPriatelov++] = koho->nick;
                                    client->ziadosti_op[i]->koho->priatelia[client->ziadosti_op[i]->koho->pocetPriatelov++] = komu->nick;
                                }
                                // TODO co ak nie je online ten, ktory si ma chcel pridat?
                                // TODO spravu mu niekde ulozit
                            }
                        }
                        //posliZiadosti(fz);

                        //free(fz);
                        
                    } else {
                        strcpy(msg, "NOK");
                        n = write(client->socket, msg, strlen(msg) + 1);
                    }

                    break;

                case 'R':
                    client->nick = strtok(NULL, "|");
                    client->heslo = strtok(NULL, "|");
                    printf("registrujem ho\n");

                    client->uspech = 1;
                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        // Uz je registrovany
                        if (strcmp(client->registrovani[i]->nick, client->nick) == 0) {
                            printf("Registracia: uzivatel uz existuje\n");
                            client->uspech = 2;
                            break;
                        }
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
                        novy->cisloSocketu = client->socket;

                        client->registrovani[pocetRegistrovanych] = novy;
                        ++pocetRegistrovanych;
                        printf("Registracia ok, count: %d\n", pocetRegistrovanych);
                        
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
                    for (int i = 0; i < pocetAktualPrihlasenych; i++) {
                        if (strcmp(client->prihlaseni[i]->nick, client->nick) == 0) {
                            //swapOdhlasenie(i, client->prihlaseni);
                            //swap
                            printf("Swapujem\n");
                            if (i != pocetAktualPrihlasenych - 1) {
                                uzivatel* pom = client->prihlaseni[i];
                                client->prihlaseni[i] = client->prihlaseni[pocetAktualPrihlasenych - 1];
                                client->prihlaseni[pocetAktualPrihlasenych - 1] = pom;
                            }
                            
                            client->prihlaseni[pocetAktualPrihlasenych - 1] = NULL;
                            pocetAktualPrihlasenych--;
                            client->uspech = 1;
                        }
                    }
                    
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
                        }
                    }
                    
                    if (uspechOdhlas == 1) {
                        uspechVymaz = 0;
                        printf("Registrovanich pred zrusenim: %d\n", pocetRegistrovanych);
                        //uspechVymaz = zrusenieUctu(client->nick, client->registrovani);
                        for (int i = 0; i < pocetRegistrovanych; i++) {
                            if (strcmp(client->registrovani[i]->nick, client->nick) == 0) {

                                //swapZrusenie(i, registrovani); // dam ho na koniec pola
                                if (i != pocetRegistrovanych - 1) {
                                    uzivatel* pom = client->registrovani[i];
                                    client->registrovani[i] = client->registrovani[pocetRegistrovanych - 1];
                                    client->registrovani[pocetRegistrovanych - 1] = pom;
                                }
                                pocetRegistrovanych--;
                                free(client->registrovani[pocetRegistrovanych]->nick);
                                free(client->registrovani[pocetRegistrovanych]->heslo);
                                free(client->registrovani[pocetRegistrovanych]->priatelia);
                                free(client->registrovani[pocetRegistrovanych]);
                                client->registrovani[pocetRegistrovanych] = NULL;

                                // TODO vsetky smerniky na pouzivatela z inych poli zrusit ???...
                                uspechVymaz = 1;
                            }
                        }
                        
                        if (uspechVymaz == 1) {
                            strcpy(msg, "OK");
                            printf("Ucet %s zruseny\n", client->nick);
                            printf("Registrovanich po zruseni: %d\n", pocetRegistrovanych);
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
                    sprintf(c, "%d", pocetZiadostiOP);
                    printf("Posielam mu pocet ziadosti o priatelstvo: %d\n", pocetZiadostiOP);
                    n = write(client->socket, c, strlen(c) + 1);
                    usleep(200000);
                    
                    for (int i = 0; i < pocetZiadostiOP; i++) {
                        if (strcmp(client->nick, client->ziadosti_op[i]->koho->nick) == 0) {
                            bzero(msg, 2);
                            n = write(client->socket, client->ziadosti_op[i]->komu->nick, strlen(client->ziadosti_op[i]->komu->nick) + 1);
                            n = read(client->socket, msg, 2);
                            
                            if (msg == "Y") {
                                // TODO pridam ich do priatelov
                                
                                client->ziadosti_op[i]->komu->priatelia[client->ziadosti_op[i]->komu->pocetPriatelov] = client->ziadosti_op[i]->koho->nick; 
                                client->ziadosti_op[i]->koho->priatelia[client->ziadosti_op[i]->koho->pocetPriatelov] = client->ziadosti_op[i]->komu->nick; 
                                client->ziadosti_op[i]->komu->pocetPriatelov++;
                                client->ziadosti_op[i]->koho->pocetPriatelov++;
                            }
                        }
                        
                    }
                    
                    // poslem pocet registrovanych
                    sprintf(c, "%d", pocetRegistrovanych);
                    printf("Posielam mu pocet registrovanych: %d\n", pocetRegistrovanych);
                    n = write(client->socket, c, strlen(c) + 1);
                    usleep(200000);
                    
                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        n = write(client->socket, client->registrovani[i]->nick, strlen(client->registrovani[i]->nick) + 1);
                        usleep(200000);
                    }

                    n = read(client->socket, buffer, 42);
                    komu_nick = strtok(buffer, "|");
                    koho_nick = strtok(NULL, "|");
                    int koho_socket = -1;
                    printf("%s si chce pridat %s\n", komu_nick, koho_nick);
                    //komu = najdiUziPodlaNicku((const char*) komu_nick, client->registrovani);
                    komu = NULL;
                    //koho = najdiUziPodlaNicku((const char*) koho_nick, client->registrovani);
                    koho = NULL;
                    
                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        if (strcmp(komu_nick, client->registrovani[i]->nick) == 0) {
                            komu = client->registrovani[i];
                        }
                    }

                    for (int i = 0; i < pocetRegistrovanych; i++) {
                        if (strcmp(koho_nick, client->registrovani[i]->nick) == 0) {
                            koho = client->registrovani[i];
                            koho_socket = client->registrovani[i]->cisloSocketu;
                        }
                    }
                    
                    if (komu != NULL && koho != NULL) {
                        char* msgKomu;
                        char* msgKoho;
                        if (jePrihlaseny(koho_nick, client->prihlaseni)) {
                            //pridajPriatela(wrapper);

                            //half_ziadost* wrapper = (half_ziadost*) pdata;

                            // komu pridava koho
                            //uzivatel* komu = wrapper->ziadost->komu;
                            //uzivatel* koho = wrapper->ziadost->koho;


                            // seba si pridat nemozem
                            if (komu->nick == koho->nick) {
                                strcpy(msg, "NOK");
                                printf("Seba si pridat nemozem\n");
                                n = write(client->socket, msg, strlen(msg) + 1);
                            }

                            for (int i = 0; i < komu->pocetPriatelov; i++) {
                                // ak uz ho ma v priateloch
                                if (komu->priatelia[i] == koho->nick) {
                                    strcpy(msg, "NOK");
                                    printf("Uz je v priateloch\n");
                                    n = write(client->socket, msg, strlen(msg) + 1);
                                }
                            }

                            // uz som zistoval, ci je prihlaseny
                            msgKoho = komu->nick;
                            strcpy(msgKoho, " si Vas chce pridat do priatelov? Suhlasite?( Y/N )");
                            // TODO koho socket
                            write(koho_socket, msgKoho, strlen(msgKoho) + 1);
                            read(koho_socket, msgKoho, 2);

                            msgKomu = koho->nick;
                            if (*msgKoho == 'Y') {
                                strcpy(msgKomu, " suhlasil s pridanim do priatelov.");
                                // Pridam ho do zoznamu priatelov druheho
                                komu->priatelia[komu->pocetPriatelov++] = koho->nick;
                                koho->priatelia[koho->pocetPriatelov++] = komu->nick;

                                msgKoho = komu->nick;
                                strcpy(msgKoho, " bol pridany do priatelov.");
                                write(koho_socket, msgKoho, strlen(msgKoho) + 1);
                            } else {
                                strcpy(msgKomu, " nesuhlasil s pridanim do priatelov.");
                            }
                            write(client->socket, msgKomu, strlen(msgKomu) + 1);

                        } else {
                            msgKoho = komu->nick;
                            strcpy(msgKoho, " si Vas chce pridat do priatelov? Suhlasite?( Y/N )");
                            // ulozim ziadost
                            ziadost_op* novaZiadost = (ziadost_op*) malloc(sizeof (ziadost_op));
                            novaZiadost->komu = komu;
                            novaZiadost->koho = koho;
                            client->ziadosti_op[pocetZiadostiOP] = novaZiadost;
                            ++pocetZiadostiOP;


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

                        ziadost_op* novaZiadost = (ziadost_op*) malloc(sizeof (ziadost_op));
                        novaZiadost->komu = komu;
                        novaZiadost->koho = koho;

                        half_ziadost* wrapper = (half_ziadost*) malloc(sizeof (half_ziadost));
                        wrapper->ziadost = novaZiadost;
                        wrapper->odpoved_socket = client->socket;

                        if (jePrihlaseny(koho_nick, client->prihlaseni)) {
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

                    koho = jePrihlaseny(koho_nick, client->prihlaseni);
                    komu = jePrihlaseny(komu_nick, client->prihlaseni);

                    if (komu != NULL) {
                        // poslem spravu
                        write(komu->cisloSocketu, client->userMsg, strlen(client->userMsg) + 1);
                        n = write(client->socket, "OK", 3); // mozno do prec
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
                    
                case 'F':
                    koniec = 1;
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
    printf("Zatvaram socket %d a idem von z vlakna\n", client->socket);
    close(client->socket);
}