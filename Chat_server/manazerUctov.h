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
        char* od_koho;
        char* msg;
    } message;

    typedef struct {
        char* nick;             // Dvaja s rovnakym nickom nemozu byt v chate
        char* heslo;
        char** priatelia;       // Nick je unikatny, mame len 1 zoznam registrovanych
        int pocetPriatelov;
        char** ziadosti_op;
        int pocetZiadostiOP;
        char** ziadosti_zp;     // O zrusenie priatelstva
        int pocetZiadostiZP;
        int cisloSocketu;
        message** messages;
        int pocetSprav;
    } uzivatel;

    typedef struct {
        char *buffer;
        char *nick; // temp
        char *heslo; // temp
        char* komu_nick;
        char* koho_nick;
        int uspech;
        char* option;
        char* msg; // Navratova sprava pouzivatelovi
        char* userMsg;
        int socket;
        int index;
        uzivatel** prihlaseni;
        uzivatel** registrovani;
    } client_data;

    typedef struct {
        int vlaknoId;
        int skoncene;
        client_data* my_client;
        pthread_mutex_t *mutex_register;
        pthread_mutex_t *mutex_prihlas;
        pthread_mutex_t *mutex_ziadosti_op;
        pthread_mutex_t *mutex_ziadosti_zp;
        pthread_mutex_t *mutex_spravy;
    } thread_data;

    void* najdiUziPodlaNicku(char* nick, uzivatel** registrovani_p);

    void* jePrihlaseny(char* nick, uzivatel** prihlaseni_p);

    void* obsluzClienta(void* pdata);

#ifdef __cplusplus
}
#endif

#endif /* MANAZERUCTOV_H */




