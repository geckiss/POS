/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Martin Gregorík
 * Created on 18. Decembra 2018, 12:32
 */

/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
*/

//#define PORT 11111

/*
 * Kód pre server
 */

/*
int main(int argc, char** argv) {
    int server_fd, newSocket_fd;
    int* new_sockets = (int*)malloc(sizeof(int) * 255);
    int socketCount = 0;
    int opt = 1;
    struct sockaddr_in address;
    int addrLen = sizeof(address);
    char sprava[256];
    int n;
    
    // Vytvorim socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Error creating socket");
        return 1;
    }
    // Optional
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    // Vynulujem adresu
    bzero((char*)&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    // Nastavim mu adresu a port podla hodnot v address
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    
    // Pasive - necham ho pocuvat
    listen(server_fd, 20);
    
    // Prijimem spojenie
    newSocket_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrLen);
    if (newSocket_fd < 0)
    {
	perror("ERROR on accept");
	return 3;
    }
   
    bzero(sprava, 256);
    n = read(newSocket_fd, sprava, 255);
    
    printf("Sprava: %s\n", sprava);
    
    close(newSocket_fd);
    close(server_fd);
    return (EXIT_SUCCESS);
}

*/