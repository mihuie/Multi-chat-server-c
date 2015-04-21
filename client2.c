#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */



#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000


void peerChat(int i, int sock)
{
    char peer_ip[10], peer_id[10];
    char buf[BUF_SIZE];
    int byte_recvd;

    if (i == 0){

        fgets(buf, BUF_SIZE, stdin);
        if (strcmp(buf, "\\c\n") == 0){
            write(sock, buf, 10);
        }else if (strcmp(buf, "\\q\n") == 0){
            write(sock, buf, BUF_SIZE);
            close(sock);
            printf("\nbye!");
        }else {
            write(sock, buf, BUF_SIZE);
        } 
    } else {
        byte_recvd = recv(sock, buf, sizeof(buf), 0);
        buf[byte_recvd] = '\0';

        if (strcmp(buf, "r@") == 0){    
            printf("server>  Enter a username? :");
            memset(buf, BUF_SIZE, 0);
            scanf("%s", buf);
            send(sock, buf, BUF_SIZE, 0);
        }else {
            printf("%s\n" , buf);
            printf("\n>>> ");
            fflush(stdout);
        }
    }    

}

void ServerConnect()
{
    int sock;
    struct sockaddr_in  addr_server;
    int addr_size, no_clients, i;
    fd_set master;
    fd_set read_fds;

    // create socket for sending data 
    sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0){
        printf("socket() failed\n");
        exit(0);
    }

    // fill the address structure for sending data 
    memset(&addr_server, 0, sizeof(addr_server));  // zero out structure 
    addr_server.sin_family = AF_INET;  // address family 
    addr_server.sin_addr.s_addr = inet_addr(SERVER_IP);
    addr_server.sin_port = htons((unsigned short)SERVER_PORT);

    if(connect(sock, (struct sockaddr *) &addr_server, sizeof(addr_server)) == -1) {
        perror("connect");
        exit(1);
    } 

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(0, &master);
    FD_SET(sock, &master);
    
    no_clients = sock;
    
    while(1){
        read_fds = master;
        if(select(no_clients+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(4);
        }
        
        for(i=0; i <= no_clients; i++ )
            if(FD_ISSET(i, &read_fds)){
                peerChat(i, sock);
                fflush(stdout);
            }
    }

    close(sock);
    
}

int main(int argc, char *argv[]){
    

    int option;

    while(1){

        printf("\n1: Group Chat \n2: Availble Peers\n>>> ");
        scanf(" %d", &option);
        printf("\n\n");

        if (option == 2){ 
            ServerConnect();
        } else if (option == 1) {
            printf(" Not setup up yet.... \n");
        }
    }
}