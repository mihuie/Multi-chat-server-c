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

    int fun[20] = {0}; // holds list of persons in fun group
    int fun_count = 0; // counts number of persons in fun group
    int loop, user_present;
    char str[BUF_SIZE], temp[10];
    char users[MAX_USERS][USER_INFO] = {{""}};
    char who_str[] = "r@";
    char menu[] = "\n\tMENU\n\\c - Chat with user\n\\l - List online users\n\\f - Fun Group\n\\w - Work Group\n\\q - Quit";
    



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
new_user:       if (i == sock_recv){
                    incoming_len = sizeof(remote_addr);   /* who sent to us? */
                    if (!((newfd = accept(sock_recv, (struct sockaddr *)&remote_addr, &incoming_len)) == -1)){
                        FD_SET(newfd, &active_fd_set);
                        if (newfd > no_clients){
                            // check if username already exist
                            loop = 1;
                            while(loop){
                                loop = 0;
                                write(newfd, who_str, 20);
                                recv(newfd, buf, sizeof(buf), 0);                            
                                for (n = 0; n<= no_clients; n++){
                                    if (strcmp(users[n], buf) == 0){
                                        write(newfd, "server>  Username unavailable!", 35);
                                        loop = 1;
                                    }
                                }
                            }
                            // register user
                            strcpy(users[newfd], buf);
                            no_clients = newfd;
                        }

                        memset(str, 0, BUF_SIZE);
                        strcpy(str, "\nYou are now online as ");
                        strcat(str, users[newfd]);
                        write(newfd, str, BUF_SIZE);
                        write(newfd, menu, BUF_SIZE);
                    }

                } else {
exsiting_user:
                    recv_msg_size=recv(i, buf, sizeof(buf), 0);
                    buf[recv_msg_size] = '\0';

                    // if user disconnects
                    if (recv_msg_size <= 0){ 
                        if (recv_msg_size == 0)
                            printf("%s went offline", users[i]);
                        else 
                            perror("recv");
                        
                        close(i); // bye!
                        FD_CLR(i, &active_fd_set);

                    } else {
chat:                   // checking if  
                        if(strcmp("\\c\n", buf) == 0){
                            write(i, "\nWho do you want to chat with? : ", BUF_SIZE);
                            recv(i, buf, sizeof(buf), 0);

                            // searching if username listed
                            user_present = 0;
                            for (n = 0; n<= no_clients; n++){
                                if (strcmp(users[n], buf) == 0)
                                    user_present = 1;
                            }

                            if(user_present == 0)
                                write(i, "invalid username! \n", BUF_SIZE);
                            else{
                                printf("Yes\n");
                            }
                        } 
                        // list
                        if(strcmp("\\l\n", buf) == 0){
userlist: 
                            // creating a list of peers to return to
                            memset(str, 0, BUF_SIZE);
                            strcat(str, "\nAvailable users:\n");

                            // creating list of available users
                            for (n = 0; n<= no_clients; n++){
                                if (FD_ISSET(n, &active_fd_set)) {
                                    // except the listener and ourselves
                                    if (n != sock_recv && n != i){
                                        strcat(str, users[n]);
                                        strcat(str, "\n");
                                    }
                                }
                            }
                            strcat(str, users[i]);
                            strcat(str, " ***me");
                            write(i, str, BUF_SIZE);
                        } 
                        if(strcmp("\\f\n", buf) == 0){
fun_group: 
                            memset(str, 0, BUF_SIZE);
                            write(i, "\nWelcome to the Fun Group\n", BUF_SIZE);

                            fun[fun_count] = i;
                            fun_count = fun_count + 1;
                            strcat(str, users[i]);
                            strcat(str, " has joined group");
                            // write(i, str, BUF_SIZE);

                            for (n = 0; n<= no_clients; n++){
                                // if (FD_ISSET(n, &active_fd_set)) {
                                    // except the listener and ourselves
                                    if (n != sock_recv && n != i )
                                        write(fun[n], str, BUF_SIZE);
                                // }
                            }

                            
                        } 

                    }// end else --- checking user responses
                }
            }
        }
    }
}


