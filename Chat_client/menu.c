/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "menu.h"


char prihlasenyNick[255];

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

int prihlasenie(int sockfd, struct sockaddr_in serv_addr) {
    
    char nick[20], pass[20], msg[255], buff[255];
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
    printf("Poslielam udaje. \n");
    int n = write(sockfd, msg, 255);
    printf("Poslal som prihlasovacie udaje. \n");
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri prihlasovani. \n");
        return 0;
    }

    bzero(msg, 255);
    printf("Cakam na odpoved. \n");
    n = read(sockfd, msg, 255);     // Odpoved od servera, prihlasil/ziadosti
    
  
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani");
        return 0;
    } else {
        // Zistim, ci som sa prihlasil
        if (strcmp(msg, "NOK") == 0) {
            printf("Prihlasenie neuspesne, zle prihlasovacie udaje!\n");
            return 0;
        } else {
            //n = write(sockfd, "OK", 255);
            printf("Prihlasenie uspesne!\n");  
            strcpy(prihlasenyNick, nick);
            printf("Prihlaseny nick: %s!\n", prihlasenyNick); 
                     
              
            char *mamZiadosti;
            printf("Cakam na ziadosti.\n");
            //-----------------
            
           
            //-----------------
            
            n = read(sockfd, msg, 21);
            printf("Precital som, ci mam ziadosti.\n");
 
            //
            mamZiadosti = strtok(msg, "|");
            printf("Ci mam ziadosti. %s \n", mamZiadosti);
            //
            if (strcmp(mamZiadosti, "Z")) {
                char res;   //result
                long int pocetZiadosti = strtol(msg, NULL, 10);
                for (long i = 0; i < pocetZiadosti; i++) {
                    bzero(msg,255);
                    bzero(res, 1);
                    n = read(sockfd, msg, 255);
                    printf(msg);
                    printf("Prijimate? (Y/N)\n");
                    scanf("%c", res);
                    n = write(sockfd, res, 2);      // alebo 2 ?
                }
                return 1;
            } else {
                printf("Nemam ziadne nove ziadosti.");
                return 1;
            }
             
        }
    }
}

int registracia(int sockfd, struct sockaddr_in serv_addr) {
    char nick[255], pass[255], msg[255];
    printf("Zvolte si nick(maximalne 20 znakov): ");
    scanf("%s", nick);
    printf("Zvolte si heslo(maximalne 20 znakov): ");
    scanf("%s", pass);
    strcpy(msg, "R");
    strcat(msg, "|");
    strcat(msg, nick);
    strcat(msg, "|");
    strcat(msg, pass);
    
    // Poslem registracne udaje
    int n = write(sockfd, msg, 255);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri registracii");
        return 0;
    }

    bzero(msg,255);
    n = read(sockfd, msg, 255);     // Odpoved od servera(OK/DUP/NOK)
    //n = write(sockfd, "OK", 255);
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani");
        return 0;
    } else {
        if (strcmp(msg, "NOK") == 0) {
            printf("Prihlasenie neuspesne, zle prihlasovacie udaje!");
            return 0;
        } else {
            if (strcmp(msg, "DUP") == 0) {
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
    while (dobryNick != 1) {
    printf("\n");
    printf("Zvolte nick, ktoremu chcete napisat spravu:\n");
    printf("Nick: ");
    scanf("%s", priatel);
    printf("\n");
    
    if (strcmp(priatel, "ESC")) {
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
    int n = write(sockfd, msg, 255);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri registracii.");
        return 0;
    }

    bzero(msg,255);
    n = read(sockfd, msg, 255);     // Odpoved od servera(OK/NOK)
    n = write(sockfd, "OK", 255);
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani");
        return 0;
    } else {
        if (strcmp(msg, "NOK")) {
            printf("Prihlasenie neuspesne, zle prihlasovacie udaje!");
            return 0;
        } else {
            if (strcmp(msg, "DUP")) {
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
    printf("2. Sprava priatelov\n");
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
    char msg[24];
    printf("Naozaj chcete zrusit Vas ucet? Navratenie do povodneho stavu nie je mozne\n");
    /*
    printf("(Y): ");
    scanf("%c", res);
     */
    strcpy(msg, "V");       // vymazanie
    strcat(msg, "|");
    strcat(msg, prihlasenyNick);
    
    // Poslem udaje
    int n = write(sockfd, msg, 24);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri zruseni uctu.");
        return 0;
    }

    bzero(msg,24);
    n = read(sockfd, msg, 22);     // Odpoved od servera(OK/NOK)

    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani.");
        return 0;
    } else {
        if (strcmp(msg, "NOK") == 0) {
            printf("Zrusenise NEUSPESNE!\n");
            return 0;
        } else {
            printf("Ucet uspesne zruseny. RIP.\n");
            return 1;
        }
    }
}

int odhlasenie(int sockfd, struct sockaddr_in serv_addr) {
    char msg[40];
    strcpy(msg, "O");       // vymazanie
    strcat(msg, "|");
    strcat(msg, prihlasenyNick);
    
    // Poslem udaje
    int n = write(sockfd, msg, 40);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri zruseni uctu.");
        return 0;
    }

    bzero(msg,40);
    n = read(sockfd, msg, 40);     // Odpoved od servera(OK/NOK)
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani.");
        return 0;
    } else {
        if (strcmp(msg, "NOK") == 0) {
            printf("Odhlasenie neuspesne!\n");
            return 0;
        } else {
            printf("Boli ste odhlaseny.\n");
            bzero(prihlasenyNick, 255);
            return 1;
        }
    }
    
}

int pridajPriatela(int sockfd, struct sockaddr_in serv_addr, friends* pPriatelia, int pPocetPriatelov) {  
    
    char msg[40];
    char koho[20];
    char suhlas;
    int pocetRegistrovanych = 0;
    int pocetZiadosti = 0;
    int pocetAktualnychPriatelov = 0;
    
    int n = write(sockfd, "A", 2);
    printf("Stahujem nove ziadosti.\n");
    
    n = read(sockfd, msg, 40);
    pocetZiadosti = strtol(msg, NULL, 10);
    bzero(msg,40);
    printf("Pocet novych ziadosti je %d.\n", pocetZiadosti);
    
    for (int i = 0; i < pocetZiadosti; i++) {
        n = read(sockfd, msg, 40);
        printf("** %s ** si ta chce pridat do priatelov. Suhlasis Y/N\n", msg);
        scanf("%c", suhlas);
        if (strcmp(suhlas, "Y") == 0) {
            n = write(sockfd, "Y", 2);
            if (pocetAktualnychPriatelov == 1) {
                pPocetPriatelov++;
                pocetAktualnychPriatelov = 0;
            }
            for (int y = 0; y < pPocetPriatelov; y++) {
                if (pPriatelia->priatelia[pPocetPriatelov] == NULL) {
                    strcpy(pPriatelia->priatelia[pPocetPriatelov], msg);
                    pocetAktualnychPriatelov = 1;
                    break;
                }
            }
            bzero(msg,40);
        } else {
            n = write(sockfd, "N", 2);
            bzero(msg,40);
        }
    }
    
    pPocetPriatelov = pocetAktualnychPriatelov;    
    n = read(sockfd, msg, 40);
    pocetRegistrovanych = strtol(msg, NULL, 10);
    
    printf("Pocet ludi je %d.\n", pocetRegistrovanych);
         
    for (int i = 0; i < pocetRegistrovanych; i++) {
        bzero(msg,40);
        n = read(sockfd, msg, 40); 
        printf(">>  %s  <<\n", msg);
    }
    
    printf("Zadaj meno, koho si chces pridat: \n");
    scanf("%s", koho);
    
    bzero(msg,40);
    strcpy(msg, prihlasenyNick);
    strcat(msg, "|");
    strcat(msg, koho);
    
    n = write(sockfd, msg, 40);
    if (n < 0) {
        perror("Chyba pri posielani socketu na server pri zruseni uctu.");
        return 0;
    }

    bzero(msg,40);
    n = read(sockfd, msg, 40);     // Odpoved od servera(OK/NOK)
    if (n < 0) {
        perror("Chyba pri citani odpovede servera pri prihlasovani.");
        return 0;
    } else {
        if (strcmp(msg, "NOK") == 0) {
            printf("Odoslanie ziadosti neuspesne.\n");
            return 0;
        } else {
            printf("Odoslanie ziadosti uspesne.\n");
            return 1;
        }
    }
}