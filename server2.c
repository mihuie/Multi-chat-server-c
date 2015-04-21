#include <stdio.h>
#include <sys/types.h>	/* system type defintions */
#include <sys/socket.h>	/* network system functions */
#include <netinet/in.h>	/* protocol & struct definitions */
#include <stdlib.h>	/* exit() warnings */
#include <string.h>	/* memset warnings */
#include <arpa/inet.h>

#define BUF_SIZE	1024
#define LISTEN_PORT	60000
#define MAX_USERS 100
#define USER_INFO 1024

int main(int argc, char *argv[]){
    int sock_recv;
    struct sockaddr_in	my_addr;
    int	i, n, no_clients, newfd;
    fd_set readfds, active_fd_set;//,read_fd_set;
    int	incoming_len;
    struct sockaddr_in remote_addr;
    int	recv_msg_size;
    char buf[BUF_SIZE];
    int	select_ret;
    char format[10];

    // int n_users = 0;
    char str[BUF_SIZE], temp[10];
    char users[MAX_USERS][USER_INFO] = {{""}};
    char who_str[] = "\nWhat's your name? :";

    FD_ZERO(&readfds);
    FD_ZERO(&active_fd_set);/* zero out socket set */

    /* create socket for receiving */
    sock_recv=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_recv < 0){
        printf("socket() failed\n");
        exit(0);
    }
    
    /* make local address structure */
    memset(&my_addr.sin_zero, 0, sizeof(my_addr));	/* zero out structure */
    my_addr.sin_family = AF_INET;	/* address family */
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);  /* current machine IP */
    my_addr.sin_port = htons((unsigned short)LISTEN_PORT);
    
    /* bind socket to the local address */
    i=bind(sock_recv, (struct sockaddr *) &my_addr, sizeof (my_addr));
    if (i < 0){
        printf("bind() failed\n");
        exit(0);
    }

    // listen
    if (listen(sock_recv, 10) == -1) {
        perror("listen");
        exit(1);
    }

    FD_SET(sock_recv, &active_fd_set);	/* add socket to listen to */
    
    no_clients = sock_recv;

    while (1){
        readfds = active_fd_set;
        if (select(no_clients+1,&readfds,NULL,NULL,NULL) == -1){
            exit(4);
        }	

        for (i = 0; i<= no_clients; i++){

            if (FD_ISSET(i, &readfds)){
                if (i == sock_recv){
                    incoming_len = sizeof(remote_addr);   /* who sent to us? */
                    if (!((newfd = accept(sock_recv, (struct sockaddr *)&remote_addr, &incoming_len)) == -1)){
                        /*puts("After receiving...");*/
                        FD_SET(newfd, &active_fd_set);
                        if (newfd > no_clients){
                            no_clients = newfd;
                        }

                        // send(i, "who_str", 20, 0);
                        // recv(i, buf, sizeof(buf), 0);

                        printf("New connection from %s:%d\n",inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));

                        // creating a list of peers to return to
                        memset(str, 0, BUF_SIZE);
                        strcpy(str, "\nList of peers:\n");
                
                        for (n = 0; n<= no_clients; n++){
                            if (FD_ISSET(n, &active_fd_set)) {
                                // except the listener and ourselves
                                if (n != sock_recv && n != i){
                                    getpeername(n, (struct sockaddr*)&remote_addr, &incoming_len);
                                    sprintf(temp, "%d", n);
                                    strcat(str, temp);
                                    strcat(str, ":\t");
                                    strcat(str, inet_ntoa(remote_addr.sin_addr));
                                    strcat(str, ":");
                                    sprintf(temp, "%d", ntohs(remote_addr.sin_port));
                                    strcat(str, temp);
                                    strcat(str, "\n");
                                }
                            }
                        }

                        for (n = 0; n<= no_clients; n++){
                            if (FD_ISSET(n, &active_fd_set)) {
                                if (n != sock_recv && n != i)
                                    send(n, str, BUF_SIZE, 0);
                            }
                        }

                    }
                } else {

                    recv_msg_size=recv(i, buf, sizeof(buf), 0);//,(struct sockaddr *)&remote_addr,&incoming_len);
                    buf[recv_msg_size] = '\0';
                    sprintf(format, "%%%u[^\n]", recv_msg_size);

                    printf("%s\n", buf);

                    if (recv_msg_size <= 0){ /* what was sent? */
                        if (recv_msg_size == 0){ 
                            printf("socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &active_fd_set);

                        memset(str, 0, BUF_SIZE);
                        strcpy(str, "\nList of peers:\n");

                        for (n = 0; n<= no_clients; n++){
                            if (FD_ISSET(n, &active_fd_set)) {
                                // except the listener and ourselves
                                if (n != sock_recv && n != i){
                                    getpeername(n, (struct sockaddr*)&remote_addr, &incoming_len);
                                    strcat(str, inet_ntoa(remote_addr.sin_addr));
                                    strcat(str, ":");
                                    sprintf(temp, "%d", ntohs(remote_addr.sin_port));
                                    strcat(str, temp);
                                    strcat(str, "\n");
                                }
                            }
                        }

                        for (n = 0; n<= no_clients; n++){
                            if (FD_ISSET(n, &active_fd_set)) {
                                if (n != sock_recv && n != i)
                                    send(n, str, BUF_SIZE, 0);
                            }
                        }

                    } else if (FD_ISSET(atoi(buf), &active_fd_set)){
                        send(atoi(buf), "Incoming connection request...  Enter '-Y' to accept, '-N' to decline\n", BUF_SIZE, 0);
                    } else if (strcmp(buf, "yes") == 0){
                        close(i); // bye!
                        FD_CLR(i, &active_fd_set);
                    }


                }
            }
        }
    }
}


