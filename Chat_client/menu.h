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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef MENU_H
#define MENU_H

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct {
        char **priatelia;
    } friends;

    int uvodneMenu();
    
    int prihlasenie(int sockfd, struct sockaddr_in serv_addr);
        
    int hlavneMenu();
    
    int registracia(int sockfd, struct sockaddr_in serv_addr);

    int napisSpravu(int sockfd, struct sockaddr_in serv_addr, void* pPriatelia, int pPocetPriatelov);
    
    int pridajPriatela(int sockfd, struct sockaddr_in serv_addr, friends* pPriatelia, int pPocetPriatelov);
    
    int zrusPriatelstvo(int sockfd, struct sockaddr_in serv_addr);
    
    int zrusUcet(int sockfd, struct sockaddr_in serv_addr);
        
    int odhlasenie(int sockfd, struct sockaddr_in serv_addr);
    
    void vypisPriatelov();
    
#ifdef __cplusplus
}
#endif

#endif /* MENU_H */

