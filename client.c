#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>



bool zadajTvojTip(char* buffer, int sockfd)
{
    int n;
    n = read(sockfd, buffer, 255);
    if (n < 0)
    {
        perror("Error reading from socket");
        return false;
    }
    //Vypíšeme odpoveď z buffra na štandardný výstup.
    printf("%s",buffer);
    //Načítame správu od používateľa zo štandardného vstupu do buffra.
    fgets(buffer, 255, stdin);
    //Pošleme správu cez socket servru.

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("Error writing to socket");
        return false;
    }

    if (strcmp(buffer, "Koncim\n")==0) {
        printf("Odpojeny zo servera.\n");
        return false;
    }

    bzero(buffer,256);
    //Načítame odpoveď od servra do buffra.
    n = read(sockfd, buffer, 255);
    if (n < 0)
    {
        perror("Error reading from socket");
        return false;
    }
    //Vypíšeme odpoveď z buffra na štandardný výstup.
    printf("%s ",buffer);

    bzero(buffer,256);
    //Načítame správu od používateľa zo štandardného vstupu do buffra.
    fgets(buffer, 255, stdin);
    //Pošleme správu cez socket servru.
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("Error writing to socket");
        return false;
    }
    if (strcmp(buffer, "Koncim\n")==0) {
        printf("Odpojeny zo servera.\n");
        return false;
    }
    bzero(buffer, 256);
    return true;
}

void cakajVysledok(char* buffer, int sockfd)
{
    int n;

    bzero(buffer,256);
    //Načítame odpoveď od servra do buffra.
    n = recv(sockfd, buffer, 256, 0);
    if (n < 0)
    {
        perror("Error reading from socket");
        return;
    }
    //Vypíšeme odpoveď z buffra na štandardný výstup.

    printf("Moje skore: %s", buffer);

}

void vypisHraciuPlochu2(int * stavy, int * pary, int dlzka) {
    for (int i = 0; i < dlzka; ++i) {
        for (int j = 0; j < dlzka; ++j) {
            int index = j + (i * dlzka);
            if (stavy[index] == 0) {            //este neotocena karta
                printf("X   ");
            } else if (stavy[index] == 1) {     //otocena karta
                printf("%d   ", pary[index]);
            } else if (stavy[index] == -1) {     //otocena karta
                printf("-   ");
            }
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    char buffer[256];

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }

    printf("Vitaj, a drzim palce.\n");

    int pocetParov;
    recv(sockfd, &pocetParov, sizeof(pocetParov), 0);
    int pary[pocetParov*pocetParov];
    int stavy[pocetParov*pocetParov];
    recv(sockfd, pary, sizeof(int)*pocetParov*pocetParov, 0);
    recv(sockfd, stavy, sizeof(int)*pocetParov*pocetParov, 0);

    vypisHraciuPlochu2(stavy, pary, pocetParov);


    while(1) {
        recv(sockfd, buffer, 255, 0);
        printf("%s\n", buffer);

        if (strcmp(buffer, "$S: Si na rade")==0 || strcmp(buffer, "$S: Chces otocit uz otocenu karticku")==0) {
            //zadajTvojTip(buffer,sockfd);
                if (!zadajTvojTip(buffer,sockfd)) {
                    break;
                }
        }else if (strcmp(buffer, "$S: Mas moznost opravit index vacsi ako pocetParov")==0 || (strcmp(buffer,"$S: Zadal si dve rovnake karty. Naprav svoj pokus"))==0) {
            if (!zadajTvojTip(buffer,sockfd)) {
                break;
            }
        } else if (strcmp(buffer, "$S: Pokracuj")==0) {
            if (!zadajTvojTip(buffer,sockfd)) {
                break;
            }
        } else if (strcmp(buffer, "$S: Koniec\n")==0) {
            cakajVysledok(buffer,sockfd);
            break;
        } else if (strcmp(buffer, "$S: Zmena")==0 || strcmp(buffer, "$S: Karty su zhodne. Pokracujes.")==0 || strcmp(buffer, "$S: Karty nie su zhodne. Pokracuje dalsi hrac.")==0) {
            int * index=stavy;
            recv(sockfd, index, sizeof(int)*pocetParov*pocetParov, 0);
            vypisHraciuPlochu2(index, pary, pocetParov);
        }
    }

    //cakajVysledok(buffer,sockfd);
    close(sockfd);
    return 0;
}