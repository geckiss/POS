/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
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

typedef struct friends_p {
    char **priatelia;
} friends;

char prihlasenyNick[20];

int uvodneMenu() {
    char moznost_c[1];
    int moznost = 0;
    printf("1. Prihlasenie\n");
    printf("2. Registracia\n");
    printf("3. Koniec\n");

    while (moznost < 1 || moznost > 3) {
        printf("Zvolte cislo moznosti: ");
        scanf("%c", moznost_c);
        moznost = strtol(moznost_c, NULL, 10);
    }
    
    return moznost;
}

int prihlasenie(int sockfd, struct sockaddr_in serv_addr, void* pPriatelia, int pPocetPria) {
    
    char nick[20], pass[20], msg[256], buff[20];
    int pocetPriatelov = 0;
    printf("Nick(maximalne 20 znakov): ");
    scanf("%s", nick);
    printf("Heslo(maximalne 20 znakov): ");
    scanf("%s", pass);
    strcpy(msg, "P");
    strcat(msg, "|");
    strcat(msg, nick);
    strcat(msg, "|");
    strcat(msg, pass);
    
    // Poslem prihlasovacie udaje
    int n = write(sockfd, msg, 45);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri prihlasovani");
        return 0;
    }

    bzero(msg,45);
    n = read(sockfd, msg, 255);     // Odpoved od servera, prihlasil/ziadosti
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani");
        return 0;
    } else {
        // Zistim, ci som sa prihlasil
        if (msg == "NOK") {
            printf("Prihlasenie neuspesne, zle prihlasovacie udaje!");
            return 0;
        } else {
            n = write(sockfd, "OK", 2);
            printf("Prihlasenie uspesne!");
            strcpy(prihlasenyNick, nick);
            
            friends* zoznamPriatelov = (friends*)pPriatelia;
            
            int pocetPriatelov[1];
            n = read(sockfd, pocetPriatelov, 4); 
            
            for (int i = 0; i < pocetPriatelov[0]; i++) {
                n = read(sockfd, buff, 20); 
                strcpy(zoznamPriatelov->priatelia[i], buff);
            }
            
            char *mamZiadosti = strtok(msg, '|');
            if (mamZiadosti == "Z") {
                char res;   //result
                long pocetZiadosti = strtol(msg, NULL, 10);
                for (long i = 0; i < pocetZiadosti; i++) {
                    bzero(msg,256);
                    bzero(res, 1);
                    n = read(sockfd, msg, 255);
                    printf(msg);
                    printf("Prijimate? (Y/N)\n");
                    scanf("%c", res);
                    n = write(sockfd, res, 1);      // alebo 2 ?
                }
                return 1;
            } else {
                // Prihlasil som sa a nemam ziadosti
                return 1;
            }
        }
    }
}

int registracia(int sockfd, struct sockaddr_in serv_addr) {
    char nick[20], pass[20], msg[44];
    printf("Zvolte si nick(maximalne 20 znakov): ");
    scanf("%s", nick);
    printf("Zvolte si heslo(maximalne 20 znakov): ");
    scanf("%s", pass);
    strcpy(msg, 'R');
    strcat(msg, '|');
    strcat(msg, nick);
    strcat(msg, '|');
    strcat(msg, pass);
    
    // Poslem registracne udaje
    int n = write(sockfd, msg, 44);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri registracii");
        return 0;
    }

    bzero(msg,44);
    n = read(sockfd, msg, 44);     // Odpoved od servera(OK/DUP/NOK)
    n = write(sockfd, "OK", 2);
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani");
        return 0;
    } else {
        if (msg == "NOK") {
            printf("Prihlasenie neuspesne, zle prihlasovacie udaje!");
            return 0;
        } else {
            if (msg == "DUP") {
                printf("Uzivatel s danym nickom uz existuje!");
                return 0;
            } else {
                printf("Registracia uspesna!");
                return 1;
            }
        }
    }
}

int napisSpravu(int sockfd, struct sockaddr_in serv_addr, void* pPriatelia, int pPocetPriatelov) {
    char msg[298];
    char priatel[20];
    friends* zoznamPriatelov = (friends*)pPriatelia;
    
    // Vypisem priatelov
    for (int i = 0; i < pPocetPriatelov; i++) {
        printf(i);
        printf(". ");
        printf(zoznamPriatelov->priatelia[i]);
        printf("\n");
    }
    
    int dobryNick = 0;
    while (!dobryNick) {
    printf("\n");
    printf("Zvolte nick, ktoremu chcete napisat spravu:\n");
    printf("Nick: ");
    scanf("%s", priatel);
    printf("\n");
    
    if (priatel == "ESC") {
        return 0;
    }
    // Ci zadal dobry nick
        for (int i = 0; i < pPocetPriatelov; i++) {
            if (priatel == zoznamPriatelov->priatelia[i]) {
                dobryNick = 1;
                break;
            }
        }
    }
    
    printf("Sprava(maximalne 255 znakov): ");
    scanf("%s", msg);
    
    strcpy(msg, "S");
    strcat(msg, "|");
    strcat(msg, priatel);
    strcat(msg, "|");
    strcat(msg, prihlasenyNick);
    strcat(msg, "|");
    strcat(msg, msg);
    
    // Poslem spravu
    int n = write(sockfd, msg, 298);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri registracii.");
        return 0;
    }

    bzero(msg,298);
    n = read(sockfd, msg, 3);     // Odpoved od servera(OK/NOK)
    n = write(sockfd, "OK", 2);
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani");
        return 0;
    } else {
        if (msg == "NOK") {
            printf("Prihlasenie neuspesne, zle prihlasovacie udaje!");
            return 0;
        } else {
            if (msg == "DUP") {
                printf("Uzivatel s danym nickom uz existuje!");
                return 0;
            } else {
                printf("Registracia uspesna!");
                return 1;
            }
        }
    }
}

// Po prihlaseni
int hlavneMenu() {
    char moznost_c[1];
    int moznost = 0;
    printf("1. Sprava\n");
    printf("2. Pridat priatela\n");
    printf("3. Zrusit priatelstvo\n");
    printf("4. Zrusenie uctu\n");
    printf("5. Odhlasit sa\n");
    while (moznost < 1 || moznost > 5) { 
        printf("Zvolte cislo moznosti: ");
        scanf("%c", moznost_c);
        moznost = strtol(moznost_c, NULL, 10);
    }
    return moznost;
}

int zrusUcet(int sockfd, struct sockaddr_in serv_addr) {
    char msg[22], res[1];
    printf("Naozaj chcete zrusit Vas ucet? Navratenie do povodneho stavu nie je mozne\n");
    printf("(Y/N): ");
    scanf("%c", res);
    strcpy(msg, "V");       // vymazanie
    strcat(msg, "|");
    strcat(msg, prihlasenyNick);
    
    // Poslem udaje
    int n = write(sockfd, msg, 22);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri zruseni uctu.");
        return 0;
    }

    bzero(msg,22);
    n = read(sockfd, msg, 3);     // Odpoved od servera(OK/NOK)
    n = write(sockfd, "OK", 2);
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani.");
        return 0;
    } else {
        if (msg == "NOK") {
            printf("Zrusenise NEUSPESNE!\n");
            return 0;
        } else {
            printf("Ucet uspesne zruseny. RIP.\n");
            return 1;
        }
    }
}

int odhlasenie(int sockfd, struct sockaddr_in serv_addr) {
    char msg[22];
    strcpy(msg, "O");       // vymazanie
    strcat(msg, "|");
    strcat(msg, prihlasenyNick);
    
    // Poslem udaje
    int n = write(sockfd, msg, 22);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri zruseni uctu.");
        return 0;
    }

    bzero(msg,22);
    n = read(sockfd, msg, 3);     // Odpoved od servera(OK/NOK)
    n = write(sockfd, "OK", 2);
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani.");
        return 0;
    } else {
        if (msg == "NOK") {
            printf("Odhlasenie neuspesne!\n");
            return 0;
        } else {
            printf("Boli ste odhlaseny.\n");
            bzero(prihlasenyNick, 20);
            return 1;
        }
    }
}