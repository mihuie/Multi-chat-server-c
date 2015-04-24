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
    int sock, sock_toPeer, sock_in, sock_PeerIn, sock_listen, sock_recv;
    struct sockaddr_in  addr_server, my_addr, peer_addr, recv_addr;
    int addr_size, no_clients, i;
    fd_set master;
    fd_set read_fds;

    char buf[BUF_SIZE], my_buf[BUF_SIZE], str[20];
    int byte_recvd, bytes_received;
    char menu[] = "\n\t\tMENU\n\t\\h - Help\n\t\\c - Chat with user\n\t\\l - List online users\n\t\\f - Fun Group\n\t\\w - Work Group\n\t\\x - Exit Group\n\t\\q - Quit\n";
    char user[30];
    char ip[30];
    char port[10];
    int group = 0;
    int myPort = (rand() % 1000 ) + 60001;
    char recv_buf[BUF_SIZE];
    char hostname[128];


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
                        strcat(str, ",");
                        sprintf(port, "%d", myPort);
                        strcat(str, port);
                        write(sock, str, BUF_SIZE); 

                        printf("listening on port %s........\n", port);


                        // listening for connections
                        sock_listen = socket(PF_INET, SOCK_STREAM, 0);
                        if (sock_listen < 0){
                            printf("socket() failed\n");
                            exit(0);
                        }
                         
                        memset(&my_addr, 0, sizeof (my_addr));  
                        my_addr.sin_family = AF_INET;   
                        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
                        my_addr.sin_port = htons((unsigned short)myPort);
                            
                        i=bind(sock_listen, (struct sockaddr *) &my_addr, sizeof (my_addr));
                        if (i < 0){
                            printf("bind() failed\n");
                            exit(0);
                        }
                           
                        i=listen(sock_listen, 5);
                        if (i < 0){
                            printf("listen() failed\n");
                            exit(0);
                        }

                        printf("\nConnected\n\n");
                        int count;
                        while (1){
                            /* if connection is established, communicate */
                            bzero(my_buf, BUF_SIZE);
                            count = read( sock_recv, my_buf, BUF_SIZE);
                            if(count < 0)
                            {
                              perror("Error reading from socket\n");
                              exit(1);
                            }
                            printf("remote user > %s\n", my_buf);

                            /* Decode HTTP header */

                            /* write a response to the client */
                            printf("me >");
                            bzero(my_buf, BUF_SIZE);
                            fgets(my_buf, BUF_SIZE, stdin);
                            count = write(sock_recv, my_buf, strlen(my_buf));
                            if(count < 0)
                            {
                              perror("Error writing to socket\n");
                              exit(1);
                            }

                        }

                        close(sock_recv);
                        close(sock_listen);


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
                        // fgets(buf, BUF_SIZE, stdin);
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
                            port[j] = buf[i];
                            i++;
                            j++;
                        }
                        i++;

                        int h=0;
                        while(buf[i] != '\0')
                        {
                            ip[h] = buf[i];
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
                            printf("\nSetting up connection\n\n");
                            
                            if((sock_toPeer = socket(AF_INET, SOCK_STREAM, 0)) == -1)
                            {
                                perror("socket");
                            }

                            peer_addr.sin_family = AF_INET;
                            peer_addr.sin_addr.s_addr = inet_addr(ip);
                            peer_addr.sin_port = htons( atoi(port) );

                            sock_toPeer = connect(sock_toPeer,(struct sockaddr *) &peer_addr, sizeof(peer_addr));
                            if(sock_toPeer<0)
                            {
                                perror("Connect");
                                break;
                            }
                            printf("\n Go ahead, start chatting with %s\n", user);

                            while (1){

                                memset(recv_buf,'\0', BUF_SIZE);
                                memset(my_buf,'\0', BUF_SIZE);

                                if(recv(sock_toPeer, recv_buf, BUF_SIZE, 0) > 0)
                                    printf("\t\t<%s>: %s\n", user, recv_buf);
                                else if (fgets(my_buf, BUF_SIZE, stdin) != NULL){
                                    write(sock_toPeer, my_buf, BUF_SIZE);
                                    memset(my_buf,'\0', BUF_SIZE);
                                    fflush(stdout);
                                }
                                
                                
                            }
                            close(sock_toPeer);   
                        }
                    } 
                    else {
                        printf("%s\n" , buf);
                        fflush(stdout);
                    }
                }  
                fflush(stdout);
            }
    }
    close(sock);    
}

