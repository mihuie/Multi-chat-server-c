// 03025377 Michael Huie

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */


#define BUF_SIZE	1024
#define SERVER_IP	"127.0.0.1"
#define	SERVER_PORT	60000



int main(int argc, char *argv[])
{
    int sock, sock_toPeer, sock_in, sock_PeerIn;
    struct sockaddr_in  addr_server, my_addr, peer_addr;
    int addr_size, no_clients, i;
    fd_set master;
    fd_set read_fds;

    char buf[BUF_SIZE], str[20];
    int byte_recvd;
    char menu[] = "\n\t\tMENU\n\t\\h - Help\n\t\\c - Chat with user\n\t\\l - List online users\n\t\\f - Fun Group\n\t\\w - Work Group\n\t\\x - Exit Group\n\t\\q - Quit\n";
    char user[30];
    char ip[30];
    char port[10];
    int group = 0;
    int myPort = (rand() % 1000 ) + 60001;


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

    system("clear");
    printf("\n\tPlease enter '\\h' for MENU\n\n");
    
    while(1){
        read_fds = master;
        if(select(no_clients+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            exit(4);
        }
        
        for(i=0; i <= no_clients; i++ )
            if(FD_ISSET(i, &read_fds)){
                // in from keyboard being sent to server
                if (i == 0){
                    fgets(buf, BUF_SIZE, stdin);

                    if (strcmp(buf, "\\q\n") == 0){
                        printf("\nbye!");
                        close(sock);

                    } else if (strcmp(buf, "\\c\n") == 0){  
                        memset(buf, '\0', BUF_SIZE);
                        memset(str, '\0', BUF_SIZE);  
                        
                        system("clear");
                        printf("Who do you want to chat with? :");
                        scanf("%s", buf);
                        
                        strcat(str, "@");
                        strcat(str, buf);
                        write(sock, str, BUF_SIZE); 

                    } else if(strcmp(buf, "\\h\n") == 0){
                        system("clear");
                        printf("%s\n", menu);

                    } else if(strcmp(buf, "\\w\n") == 0){
                        system("clear");                        
                        write(sock, buf, BUF_SIZE);
                    } else if(strcmp(buf, "\\f\n") == 0){
                        system("clear");                        
                        write(sock, buf, BUF_SIZE);
                    } else if(strcmp(buf, "\\x\n") == 0){
                        printf("Bye.... \n");
                        sleep(1);
                        system("clear");                        
                        printf("\n\tPlease enter '\\h' for MENU\n\n");
                        write(sock, buf, BUF_SIZE);
                    } else                        
                        write(sock, buf, BUF_SIZE);
                } 
                // from server 
                else {
                    byte_recvd = recv(sock, buf, sizeof(buf), 0);
                    buf[byte_recvd] = '\0';

                    if (byte_recvd <= 0){ 
                        if (byte_recvd == 0)
                            printf("Server offline\n");
                        else 
                            perror("recv");
                        
                        close(i); // bye!
                        FD_CLR(i, &read_fds);

                    }else if (strcmp(buf, "r@") == 0){ 

                        printf("server>  Enter a username? :");
                        memset(buf, BUF_SIZE, 0);
                        scanf("%s", buf);
                        write(sock, buf, BUF_SIZE);

                    } 
                    // peer to peer chat
                    else if (strncmp("@", buf, 1) == 0){

                        memset(user,'\0',30);
                        memset(ip,'\0',30);
                        memset(port,'\0', 10);

                        int i=1;

                        while(buf[i] != ',')
                        {
                            user[i-1] = buf[i];
                            i++;
                        }
                        i++;

                        int j=0;
                        while(buf[i] != ',')
                        {
                            ip[j] = buf[i];
                            i++;
                            j++;
                        }
                        i++;

                        int h=0;
                        while(buf[i] != '\0')
                        {
                            port[h] = buf[i];
                            i++;
                            h++;
                        }

                        // printf("%s %s %s \n", user, ip, port);
                        printf("\nIncoming connect request from %s. \nWould like to connect? (y/n): ", user);
                        char choice[5];
                        memset(choice,'\0', 5);
                        // fgets(choice, BUF_SIZE, stdin);
                        scanf(" %s", choice);

                        /// set up connect request to client here
                        if (strcmp(choice, "y") == 0){
                            printf("\nSetting up connection\n");
                            /*

                            if(socket(AF_INET , SOCK_STREAM , 0) < 0)
                            {
                                perror("socket");
                            }

                            addr_client.sin_addr.s_addr = (long int)atoi(ip);
                            addr_client.sin_family = AF_INET;
                            addr_client.sin_port = htons( atoi(port) );

                            sock_toPeer = connect(sock_toPeer,(struct sockaddr *) &addr_client, sizeof(addr_client));
                            if(sock_toPeer<0)
                            {
                                perror("Connect");
                                return 1;
                            }
                            /// incomplete
                            */
                        }
                    } else {
                        printf("%s\n" , buf);
                        fflush(stdout);
                    }
                }  
                fflush(stdout);
            }
    }
    close(sock);    
}

