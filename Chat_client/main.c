/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Mato
 *
 * Created on January 3, 2019, 12:58 PM
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "menu.h"
/*
 * 
 */

typedef struct friends_p {
    char **priatelia;
} friends;


int main(int argc, char** argv) {

    // TODO uvodne menu - prihlasenie/registracia/koniec
    // prihlasenie - do spravy v sockete dat na zaciatok 'P' a delimiter je |
    // po prihlaseni sa natiahne jeho 'uzivatel' zo servera na clienta
    // potom sa mu zobrazia ziadosti o priatelstvo(caka sa na vstup Y/N)
    // po ziadostiach spravy - format [od koho]: sprava
    // po spravach menu - napis spravu/pridaj priatela/zrus priatela/odhlas->uvod.menu
    // napis spravu - vypise sa mu zoznam priatelov, zada meno, ak je OK, zada spravu
    // pridaj priatela - zada nick, ak sa najde na serveri, OK
    // priatel sa prida aj na serveri(uz nakodene) aj do clientovho 'uzivatela'
    // zrus priatela - detto
    // odhlas - posle sa socket s 'O', na serveri sa odhlasi(uz nakodene)
    
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

	char buffer[256];

	if (argc < 3)
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
	}

	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		fprintf(stderr, "Error, no such host\n");
	}

	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy(
		(char*)server->h_addr,
		(char*)&serv_addr.sin_addr.s_addr,
		server->h_length
	);
	serv_addr.sin_port = htons(11111);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Chyba pri vytvarani socketu");
        }
        
	// Pripojim sa na server
        if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Chyba pri pripajani sa na server pri prihlasovani");
        }
        
        friends* zoznamPriatelov = (friends*)malloc(sizeof(friends));
        zoznamPriatelov->priatelia = (char**)malloc(sizeof(char*)*256);
        int pocetPriatelov = 0;
            
    int koniec = 0;
    int uspechPrihlasenie = 0;
    int uspechRegistracia = 0;
    int uvodnaMoznost = 0;
    int hlavnaMoznost = 0;
    
    while (!koniec) {
        if (!uspechPrihlasenie) {
            uvodnaMoznost = uvodneMenu();
            
            switch (uvodnaMoznost) {
            case 1:
                uspechPrihlasenie = prihlasenie(sockfd, serv_addr, &zoznamPriatelov, &pocetPriatelov);
                break;
            
            case 2:
                uspechRegistracia = registracia(sockfd, serv_addr);
                break;
                
            case 3:
                koniec = 1;
                printf("Dakujeme za vyuzivanie nasho Chatu.\nMG&JB");
                break;
            }
        }
        
    
        if (uspechPrihlasenie) {
            hlavnaMoznost = hlavneMenu();
            switch(hlavnaMoznost) {
                case 1:
                    napisSpravu(sockfd, serv_addr, &zoznamPriatelov, &pocetPriatelov);
                    break;
                    
                case 2:
                    //pridajPriatela();
                    break;
                    
                case 3:
                    //zrusPriatelstvo();
                    break;
                    
                case 4:
                    uspechPrihlasenie = zrusUcet(sockfd, serv_addr);
                    // kvoli uvodnemu menu
                    if (uspechPrihlasenie) {
                        uspechPrihlasenie = 0;
                    } else {
                        uspechPrihlasenie = 1;
                    }
                    break;
                    
                case 5:
                    uspechPrihlasenie = odhlasenie(sockfd, serv_addr);
                    if (uspechPrihlasenie) {
                        uspechPrihlasenie = 0;
                    } else {
                        uspechPrihlasenie = 1;
                    }
                    break;
            }
        }
    }
    
    close(sockfd);
    return (EXIT_SUCCESS);
}

