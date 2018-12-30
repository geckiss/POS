#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "manazerUctov.h"

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

typedef struct message_p {
    uzivatel* komu;
    uzivatel* koho;
    char* msg;
} message;

int pocetVlakienZOP = 0;
int pocetZiadostiOP = 0;
int pocetZiadostiZP = 0;
int pocetSprav = 0;
//int prihlasenie(const char* nick, const char* heslo);
int main(int argc, char *argv[])
{
    // TODO half_wrapper asi nepotrebujem, kedze uzivatel si uklada socket
    // TODO mtex - vedlajsei vlakno posiela ziadosti, treba zamknut ziadosti_op 
    // aby tam hlavne vlakno nepridalo novu pri posielani, by sa poslal komu???
    uzivatel** registrovani = (uzivatel**)malloc(sizeof(uzivatel*)*POCET_UZIVATELOV);
    uzivatel** prihlaseni = (uzivatel**)malloc(sizeof(uzivatel*)*POCET_UZIVATELOV);
    ziadost_op** ziadosti_op = (ziadost_op**)malloc(sizeof(ziadost_op*)*(POCET_UZIVATELOV-1));
    half_ziadost** ziadosti_zp = (half_ziadost**)malloc(sizeof(half_ziadost*)*(POCET_UZIVATELOV-1));
    message** messages = (message**)malloc(sizeof(message*)*POCET_UZIVATELOV*5);
    pthread_t* vlaknaZiadosti = (pthread_t*)malloc(sizeof(pthread_t)*POCET_UZIVATELOV);
	int sockfd, newsockfd;
	socklen_t cli_len;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	char buffer[256];

	if (argc < 2)
	{
		fprintf(stderr,"usage %s port\n", argv[0]);
		return 1;
	}

        // while (true) ???
        
	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(atoi(argv[1]));

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Error creating socket");
		return 1;
	}

	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		 perror("Error binding socket address");
		 return 2;
	}

	listen(sockfd, 5);
	cli_len = sizeof(cli_addr);

        // Reg/login ??? msg ???
	newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
	if (newsockfd < 0)
	{
		perror("ERROR on accept");
		// return 3;
	}

	bzero(buffer,256);
	n = read(newsockfd, buffer, 255);
        if (n < 0)
	{
		perror("Error reading from socket");
		// return 4;
	}
        
        char* nick;
        char* heslo;
        char* komu_nick;
        char* koho_nick;
        int uspechPrihlas, uspechRegister, uspechOdhlas, uspechVymaz, uspechPridaj, uspechZrus;
        char* option;
        char* msg;      // Navratova sprava pouzivatelovi
        char* userMsg;   // Sprava, ktoru chce uzivatel poslat
        uzivatel* komu;
        uzivatel* koho;
        
        // Rozlisujem prihlasenie, registraciu, odhlasenie, vymazanie uctu, 
        // pridaj priatela(A), zrus priatela(Z), spravu(S)
        if (
            buffer[0] == 'P' || buffer[0] == 'R' || buffer[0] == 'O' || 
            buffer[0] == 'V' || buffer[0] == 'A' || buffer[0] == 'Z' ||
            buffer[0] == 'S'
            ) {
            
            option = strtok(buffer, "|");   // odstranim option

            switch (*option) {
                case 'P':
                    nick = strtok(buffer, "|");
                    heslo = buffer;
                    uspechPrihlas = prihlasenie(nick, heslo, &prihlaseni, &registrovani);
                    if (uspechPrihlas) {
                        msg = "Boli ste uspesne prihlaseny.";
                        // musim ho najst v prihlasenych
                        // TODO server-side:
                        // TODO posli mu vsetky spravy, ziadosti o priatelstvo, notifikacie o zruseni priatelstva
                        // TODO vytvor vlakno ktore stiahne ziadosti o p. a posle mu ich
                        pthread_t vlakno;
                        vlaknaZiadosti[pocetVlakienZOP++] = vlakno;
                    
                        full_ziadost* fz = (full_ziadost*)malloc(sizeof(full_ziadost));
                        fz->komu = prihlaseni[uspechPrihlas-1];
                        fz->ziadosti = ziadosti_op;
                        fz->pocetZiadosti = pocetZiadostiOP;
                        fz->odpoved_socket = newsockfd;
                    
                        // TODO posli celeho uzivatela v sockete po prihlaseni
                        // client si to ulozi
                        //// TODO ak to nebude fungovat, daj do faky adresy argumentov ˇˇˇ
                        pthread_create(&vlakno, NULL, &posliZiadosti, &fz);
                        pthread_join(vlakno, NULL);
                    
                        free(fz);
                        
                    } else {
                        msg = "Prihlasenie NEUSPESNE, zle vstupne udaje!";
                    }
                    break;
                    
                case 'R':
                    uspechRegister = registracia(nick, heslo, registrovani, newsockfd);
                    if (uspechRegister) {
                        msg = "Boli ste uspesne registrovany.";
                        // TODO server-side: automaticky ho prihlas?
                        
                    } else {
                        msg = "Registracia NEUSPESNA, zle vstupne udaje!";
                    }
                    break;
                    
                case 'O':
                    uspechOdhlas = odhlasenie(nick, prihlaseni);
                    if (uspechOdhlas) {
                        msg = "Boli ste uspesne odhlaseny.";
                    } else {
                        msg = "Odhlasenie NEUSPESNE, zly nick!";
                    }
                    break;
                    
                case 'V':
                    uspechVymaz = zrusenieUctu(nick, registrovani);
                    if (uspechVymaz) {
                        msg = "Ucet bol zruseny.";
                    } else {
                        msg = "Zrusenie NEUSPESNE, zly nick!";
                    }
                    break;
                    
                case 'A':
                    komu_nick = strtok(buffer, "|");
                    koho_nick = buffer;
                    
                    komu = najdiUziPodlaNicku((const char*)komu_nick, &registrovani);     // mne
                    koho = najdiUziPodlaNicku((const char*)koho_nick, &registrovani);     // jeho
                    
                    if (komu != NULL && koho != NULL) {
                        pthread_t vlakno;
                        vlaknaZiadosti[pocetVlakienZOP++] = vlakno;
                        
                        ziadost_op* novaZiadost = (ziadost_op*)malloc(sizeof(ziadost_op));
                        novaZiadost->komu = komu;
                        novaZiadost->koho = koho;
                        
                        half_ziadost* wrapper = (half_ziadost*)malloc(sizeof(half_ziadost));
                        wrapper->ziadost = novaZiadost;
                        wrapper->odpoved_socket = newsockfd;
                        
                        if (jePrihlaseny((const char*)koho_nick, prihlaseni)) {
                            pthread_create(&vlakno, NULL, &pridajPriatela, &wrapper);
                        } else {
                            // ulozim ziadost
                            ziadosti_op[pocetZiadostiOP++] = novaZiadost;
                            // TODO tieto ziadosti niekde vymazat
                        }
                        
                        pthread_join(vlakno, NULL);
                        
                        //uspechPridaj = pridajPriatela(&wrapper);
                    }
                    break;
                    
                case 'Z':
                    komu_nick = strtok(buffer, "|");
                    koho_nick = buffer;
                    
                    komu = najdiUziPodlaNicku((const char*)komu_nick, &registrovani);
                    koho = najdiUziPodlaNicku((const char*)koho_nick, &registrovani);
                    
                    
                    if (komu != NULL && koho != NULL) {
                        pthread_t vlakno;
                        vlaknaZiadosti[pocetVlakienZOP++] = vlakno;
                        
                        ziadost_op* novaZiadost = (ziadost_op*)malloc(sizeof(ziadost_op));
                        novaZiadost->komu = komu;
                        novaZiadost->koho = koho;
                        
                        half_ziadost* wrapper = (half_ziadost*)malloc(sizeof(half_ziadost));
                        wrapper->ziadost = novaZiadost;
                        wrapper->odpoved_socket = newsockfd;
                        
                        if (jePrihlaseny((const char*)koho_nick, prihlaseni)) {
                            pthread_create(&vlakno, NULL, &zrusPriatela, &wrapper);
                        } else {
                            // ulozim ziadost
                            ziadosti_zp[pocetZiadostiZP++] = wrapper;
                            // TODO tieto ziadosti niekde vymazat
                        }
                        
                        pthread_join(vlakno, NULL);
                    }
                    break;
                    
                case 'S':
                    komu_nick = strtok(buffer, "|");
                    koho_nick = strtok(buffer, "|");
                    userMsg = buffer;
                    
                    koho = jePrihlaseny((const char*)koho_nick, prihlaseni);
                    komu = jePrihlaseny((const char*)komu_nick, prihlaseni);
                    
                    if (komu != NULL) {
                        // poslem spravu
                        write(komu->cisloSocketu, userMsg, strlen(userMsg) + 1);
   
                    } else {
                        // je offline, ulozim si spravu
                        message* newMessage = (message*)malloc(sizeof(message));
                        newMessage->komu = komu;
                        newMessage->koho = koho;
                        newMessage->msg = userMsg;
                        
                        messages[pocetSprav++] = newMessage;
                        // TODO ak sa prihlasi, vamazat danu spravu(asi swap)
                    }
                    break;
            }
             
            // Poslem odpoved klientovi
            n = write(newsockfd, msg, strlen(msg)+1);
            if (n < 0) {
                perror("Chyba zapisovania do socketu");
                // return ???
            }
	
        } else {
            // sprava
            printf("Sprava: %s\n", buffer);
        }      
        
        // TODO vsetky sockety uzivatelov zatvorit
        // close vracia int
        close(newsockfd);
        
        // koniec while-u ???
	close(sockfd);

	return 0;
}