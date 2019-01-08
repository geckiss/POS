#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "manazerUctov.h"

int pocetVlakien = 0;
int loggedInCount = 0;

int main(int argc, char *argv[])
{
    // TODO half_wrapper asi nepotrebujem, kedze uzivatel si uklada socket
    // TODO mtex - vedlajsei vlakno posiela ziadosti, treba zamknut ziadosti_op 
    // aby tam hlavne vlakno nepridalo novu pri posielani, by sa poslal komu???
    printf("Zaciname\n");
    uzivatel** registrovani = (uzivatel**)malloc(sizeof(uzivatel*)*POCET_UZIVATELOV);
    uzivatel** prihlaseni = (uzivatel**)malloc(sizeof(uzivatel*)*POCET_UZIVATELOV);
    thread_data** vlakna = (thread_data**)malloc(sizeof(thread_data*)*POCET_UZIVATELOV);
    client_data** logged_clients = (client_data**)malloc(sizeof(client_data*)*POCET_UZIVATELOV);
    printf("Struktury OK\n");
    
    pthread_mutex_t mutex_register;
    pthread_mutex_init(&mutex_register, NULL);
    pthread_mutex_t mutex_prihlas;
    pthread_mutex_init(&mutex_prihlas, NULL);
    pthread_mutex_t mutex_ziadosti_op;
    pthread_mutex_init(&mutex_ziadosti_op, NULL);
    pthread_mutex_t mutex_ziadosti_zp;
    pthread_mutex_init(&mutex_ziadosti_zp, NULL);
    pthread_mutex_t mutex_clients;
    pthread_mutex_init(&mutex_clients, NULL);
    pthread_mutex_t mutex_vlakna;
    pthread_mutex_init(&mutex_vlakna, NULL);
    pthread_mutex_t mutex_spravy;
    pthread_mutex_init(&mutex_spravy, NULL);
    
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
        
        for (int i = 0; i < pocetVlakien; i++) {
            thread_data* vlakno_data = vlakna[i];
            if (vlakno_data->skoncene == 1) {
                int index = vlakno_data->my_client->index;
                free(vlakno_data->my_client->nick);
                free(vlakno_data->my_client->heslo);
                
                //swap na koniec
                client_data* pom = logged_clients[index];
                pthread_mutex_lock(&mutex_clients);
                logged_clients[index] = logged_clients[loggedInCount - 1];
                logged_clients[loggedInCount - 1] = NULL;
                --loggedInCount;
                pthread_mutex_unlock(&mutex_clients);
                free(vlakno_data->my_client);
                
                // swap na koniec
                pthread_mutex_lock(&mutex_vlakna);
                vlakna[i] = vlakna[pocetVlakien];
                vlakna[pocetVlakien] = NULL;

                
                pthread_detach(vlakno_data->vlaknoId);
                free(vlakno_data);
                --pocetVlakien;
                pthread_mutex_unlock(&mutex_vlakna);
                printf("Vlakno %d aj cdata uvolnene\n", vlakno_data->vlaknoId);
                usleep(1000000);
            }
        }
        
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
        cdata->registrovani = registrovani;
        cdata->prihlaseni = prihlaseni;
        cdata->uspech = 0;
        cdata->index = loggedInCount;
        
        pthread_mutex_lock(&mutex_clients);
        logged_clients[loggedInCount++] = cdata;
        pthread_mutex_unlock(&mutex_clients);
        
        
        pthread_t vlakno;
        thread_data* thread_d = (thread_data*)malloc(sizeof(thread_data));
        thread_d->vlaknoId = vlakno;
        thread_d->skoncene = 0;
        thread_d->my_client = cdata;
        thread_d->mutex_register = &mutex_register;
        thread_d->mutex_prihlas = &mutex_prihlas;
        thread_d->mutex_ziadosti_op = &mutex_ziadosti_op;
        thread_d->mutex_ziadosti_zp = &mutex_ziadosti_zp;
        thread_d->mutex_spravy = &mutex_spravy;
        
        pthread_mutex_lock(&mutex_vlakna);
        vlakna[pocetVlakien++] = thread_d;
        pthread_mutex_unlock(&mutex_vlakna);
        
        printf("Idem do vlakna %d\n\n", vlakno);
        pthread_create(&vlakno, NULL, &obsluzClienta, thread_d);
    }
    
    pthread_mutex_destroy(&mutex_register);
    pthread_mutex_destroy(&mutex_prihlas);
    pthread_mutex_destroy(&mutex_ziadosti_op);
    pthread_mutex_destroy(&mutex_ziadosti_zp);
    pthread_mutex_destroy(&mutex_clients);
    pthread_mutex_destroy(&mutex_vlakna);
    pthread_mutex_destroy(&mutex_spravy);
    close(sockfd);
    return 0;
}