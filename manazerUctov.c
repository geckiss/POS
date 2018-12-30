/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "manazerUctov.h"
//#include "uzivatel.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 1111

#ifndef POCET_UZIVATELOV
#define POCET_UZIVATELOV 256
#endif

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

//ziadost_op** ziadosti;
//uzivatel** registrovani;
//uzivatel** prihlaseni;
int pocetAktualPrihlasenych = 0;
int pocetRegistrovanych = 0;

struct sockaddr_in serv_addr;

int prihlasenie(const char* prihlasNick, const char* prihlasHeslo, void* prihlaseni_p, void* registrovani_p) {
    uzivatel** prihlaseni = (uzivatel**)prihlaseni_p;
    uzivatel** registrovani = (uzivatel**)registrovani_p;
    char *nick, *heslo;
    if (prihlasNick > 0 && prihlasHeslo > 0) {
        for (int i = 0; i < POCET_UZIVATELOV; i++) {
            nick = registrovani[i]->nick;
            heslo = registrovani[i]->heslo;
        
            if (nick == prihlasNick) {
                if (heslo == prihlasHeslo) {
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

int registracia(const char* registrujNick, const char* registrujHeslo, void* registrovani_p, int socket) {
    uzivatel** registrovani = (uzivatel**)registrovani_p;
    char* nick;
    if (registrujNick > 0 && registrujHeslo > 0) {
        for (int i = 0; i < POCET_UZIVATELOV; i++) {
            nick = registrovani[i]->nick;
        
            if (nick == registrujNick) {
                return 0;
            }
        }
        // Dobry malloc ???
        uzivatel* novy = (uzivatel*)malloc(sizeof(uzivatel));
        novy->nick = (char*)registrujNick;
        novy->heslo = (char*)registrujHeslo;
        novy->priatelia = (char**)malloc(sizeof(char*)*(POCET_UZIVATELOV-1));
        // potom este malokovat ten char*(nick), dostane smernik, a ten priradit do priatelov
        novy->pocetPriatelov = 0;
        novy->cisloSocketu = socket;
        registrovani[pocetRegistrovanych++] = novy;
        return 1;
    }
    return 0;
}

int zrusenieUctu(const char* zrusNick, void* registrovani_p) {
    uzivatel** registrovani = (uzivatel**)registrovani_p;
    if (zrusNick > 0) {
        for (int i = 0; i < pocetRegistrovanych; i++) {
            if (registrovani[i]->nick == zrusNick) {
                swapZrusenie(i, registrovani);
                registrovani[pocetRegistrovanych-1] = NULL;
                pocetRegistrovanych--;
                // TODO vsetky smerniky na pouzivatela z inych poli zrusit ???...
            }
        }
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
    // este skotntrolovat ci to smeruje na dobre miesto
    half_ziadost* wrapper = (half_ziadost*)pdata;
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
    
    msgKoho = komu->nick;
    strcpy(msgKomu, " si Vas chce pridat do priatelov? Suhlasite?( Y/N )");
    write(wrapper->ziadost->koho->cisloSocketu, msgKoho, strlen(msgKoho) + 1);
    read(wrapper->ziadost->koho->cisloSocketu, msgKoho, 2);
    
    msgKomu = koho->nick;
    if (*msgKoho == 'Y') {
        strcpy(msgKomu, " suhlasil s pridanim do priatelov.");  
        komu->priatelia[komu->pocetPriatelov++] = koho->nick;
        koho->priatelia[koho->pocetPriatelov++] = komu->nick;
    } else {
        strcpy(msgKomu, " nesuhlasil s pridanim do priatelov.");
    }
    write(wrapper->odpoved_socket, msgKomu, strlen(msgKomu) + 1);
    
    // zisti, ci je pridavany online
    // ak hej, posli mu socket a cakaj na odpoved
    

    
    
    
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
    char* odpoved_clienta;       // Y/N
    int n;
    for (int i = 0; i < fz->pocetZiadosti; i++) {
        // sockety na clienta
        if (fz->komu->nick == fz->ziadosti[i]->komu->nick) {
            // ziadost o priatelstvo
            // socket na clienta
            // char* msg = "Mate ziadost o priatelstvo od " + fz->ziadosti[i]->odKoho;
            char* msg = "Mate ziadost o priatelstvo od ";
            strcpy(msg, fz->ziadosti[i]->koho->nick);
            
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