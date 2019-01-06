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

typedef struct message_p {
    uzivatel* komu;
    uzivatel* koho;
    char* msg;
} message;
*/
int pocetVlakienZOP = 0;
int loggedInCount = 0;
//int prihlasenie(const char* nick, const char* heslo);,

int main(int argc, char *argv[])
{
    // TODO half_wrapper asi nepotrebujem, kedze uzivatel si uklada socket
    // TODO mtex - vedlajsei vlakno posiela ziadosti, treba zamknut ziadosti_op 
    // aby tam hlavne vlakno nepridalo novu pri posielani, by sa poslal komu???
    printf("Zaciname\n");
    uzivatel** registrovani = (uzivatel**)malloc(sizeof(uzivatel*)*POCET_UZIVATELOV);
    printf("Registrovani OK\n");
    uzivatel** prihlaseni = (uzivatel**)malloc(sizeof(uzivatel*)*POCET_UZIVATELOV);
    printf("Prihlaseni OK\n");
    ziadost_op** ziadosti_op = (ziadost_op**)malloc(sizeof(ziadost_op*)*(POCET_UZIVATELOV-1));
    printf("Ziadosti OP OK\n");
    half_ziadost** ziadosti_zp = (half_ziadost**)malloc(sizeof(half_ziadost*)*(POCET_UZIVATELOV-1));
    printf("Ziadosti ZP OK\n");
    message** messages = (message**)malloc(sizeof(message*)*POCET_UZIVATELOV*5);
    printf("Messages OK\n");
    pthread_t* vlaknaZiadosti = (pthread_t*)malloc(sizeof(pthread_t)*POCET_UZIVATELOV);
    printf("Vlakna OK\n");
    client_data** logged_clients = (client_data**)malloc(sizeof(client_data*)*POCET_UZIVATELOV);
    printf("Miesto pre klientov OK\n");
    
    int uspechPrihlas, uspechRegister, uspechOdhlas, uspechVymaz, uspechPridaj, uspechZrus;
    uspechPrihlas = uspechRegister = uspechOdhlas = uspechVymaz = uspechPridaj = uspechZrus = 0;

    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[256];
    char save[256];

    // Program
    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char*) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket\n");
        return 1;
    }
    printf("Socket OK\n");
    
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof (serv_addr)) < 0) {
        perror("Error binding socket address\n");
        return 2;
    }
    printf("Bind OK\n");
    
    listen(sockfd, 5);
    while (1) {
        
        cli_len = sizeof (cli_addr);
        printf("Cakam na accept\n");
        newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &cli_len);
        if (newsockfd < 0) {
            perror("ERROR on accept\n");
            return 3;
        }
        printf("Accept OK\n");
        client_data* cdata = (client_data*) malloc(sizeof (client_data));
        cdata->socket = newsockfd;
        cdata->nick = (char*)malloc(sizeof(char)*20);
        cdata->heslo = (char*)malloc(sizeof(char)*20);
        cdata->ziadosti_op = ziadosti_op;
        cdata->registrovani = registrovani;
        cdata->prihlaseni = prihlaseni;
        cdata->ziadosti_zp = ziadosti_zp;
        cdata->messages = messages;
        logged_clients[loggedInCount] = cdata;
        loggedInCount++;
        
        pthread_t vlakno;
        vlaknaZiadosti[pocetVlakienZOP++] = vlakno;
        printf("Idem do vlakna\n\n");
        pthread_create(&vlakno, NULL, &obsluzClienta, cdata);
    }
    // koniec while-u ???


    // TODO vsetky sockety uzivatelov zatvorit
    // TODO vycisti struktury
    // close vracia int
    
    close(sockfd);
    return 0;
}