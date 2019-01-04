/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   menu.h
 * Author: Mato
 *
 * Created on January 3, 2019, 1:29 PM
 */

#ifndef MENU_H
#define MENU_H

#ifdef __cplusplus
extern "C" {
#endif

    int uvodneMenu();
    
    int prihlasenie(int sockfd, struct sockaddr_in serv_addr, void* pPriatelia, int pPocetPria);
        
    int hlavneMenu();
    
    int registracia(int sockfd, struct sockaddr_in serv_addr);

    int napisSpravu(int sockfd, struct sockaddr_in serv_addr, void* pPriatelia, int pPocetPriatelov);
    
    int pridajPriatela(int sockfd, struct sockaddr_in serv_addr);
    
    int zrusPriatelstvo(int sockfd, struct sockaddr_in serv_addr);
    
    int zrusUcet(int sockfd, struct sockaddr_in serv_addr);
        
    int odhlasenie(int sockfd, struct sockaddr_in serv_addr);
    
    void vypisPriatelov();
    
#ifdef __cplusplus
}
#endif

#endif /* MENU_H */

