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

#ifdef __cplusplus
extern "C" {
#endif
    

int prihlasenie(const char* prihlasNick, const char* prihlasHeslo, void* prihlaseni_p, void* registrovani_p);

int odhlasenie(const char* nick, void* prihlaseni_p);

int registracia(const char* registrujNick, const char* registrujHeslo, void* registrovani_p, int socket);

int zrusenieUctu(const char* zrusNick, void* registrovani_p);

void swapOdhlasenie(int indexNekoncovy, void* prihlaseni_p);

void swapZrusenie(int indexNekoncovy, void* registrovani_p);

void* pridajPriatela(void* pdata);

void* zrusPriatela(void* pdata);

void vypisPriatelov(void* uziv);

void* posliZiadosti(void* pdata);

void* najdiUziPodlaNicku(const char* nick, void* registrovani_p);

void* jePrihlaseny(const char* nick, void* prihlaseni_p);


#ifdef __cplusplus
}
#endif

#endif /* MANAZERUCTOV_H */




