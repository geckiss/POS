/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   manazerUctov.h
 * Author: Mato
 *
 * Created on Utorok, 2018, decembra 18, 16:14
 */


#ifndef MANAZERUCTOV_H
#define MANAZERUCTOV_H

#ifndef POCET_UZIVATELOV
#define POCET_UZIVATELOV 256
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct {
    char* nick;                     // Dvaja s rovnakym nickom nemozu byt v chate
    char* heslo;    
    char** priatelia;               // Nick je unikatny, mame len 1 zoznam registrovanych
    int pocetPriatelov;
    int cisloSocketu;
} uzivatel;

typedef struct {       // o priatelstvo
    uzivatel* komu;
    uzivatel* koho;
} ziadost_op;

typedef struct {       // pridaj priatela
    ziadost_op* ziadost;
    int odpoved_socket;
} half_ziadost;

typedef struct {
    uzivatel* komu;
    ziadost_op** ziadosti;
    int pocetZiadosti;
    int odpoved_socket;
} full_ziadost;

typedef struct {
    uzivatel* komu;
    uzivatel* koho;
    char* msg;
} message;

typedef struct {
    char *buffer;
    char *save_buffer;
    char *nick;
    char *heslo;
    char* komu_nick;
    char* koho_nick;
    int uspech;
    char* option;
    char* msg;                          // Navratova sprava pouzivatelovi
    char* userMsg;
    int socket;
    uzivatel** prihlaseni;
    uzivatel** registrovani;
    ziadost_op** ziadosti_op;
    half_ziadost** ziadosti_zp;
    message** messages;
} client_data;

int prihlasenie(const char* prihlasNick, const char* prihlasHeslo, void* prihlaseni_p, void* registrovani_p);

int odhlasenie(const char* nick, void* prihlaseni_p);

int registracia(const char* registrujNick, const char* registrujHeslo, uzivatel** registrovani_p, int socket);

int zrusenieUctu(const char* zrusNick, void* registrovani_p);

void swapOdhlasenie(int indexNekoncovy, void* prihlaseni_p);

void swapZrusenie(int indexNekoncovy, void* registrovani_p);

void* pridajPriatela(void* pdata);

void* zrusPriatela(void* pdata);

void vypisPriatelov(void* uziv);

void* posliZiadosti(void* pdata);

void* najdiUziPodlaNicku(const char* nick, void* registrovani_p);

void* jePrihlaseny(const char* nick, void* prihlaseni_p);

void* obsluzClienta(void* pdata);

#ifdef __cplusplus
}
#endif

#endif /* MANAZERUCTOV_H */




