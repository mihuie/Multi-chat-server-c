// 03025377 Michael Huie

#include <stdio.h>
#include <sys/types.h>  /* system type defintions */
#include <sys/socket.h> /* network system functions */
#include <netinet/in.h> /* protocol & struct definitions */
#include <stdlib.h> /* exit() warnings */
#include <string.h> /* memset warnings */
#include <arpa/inet.h>

#define BUF_SIZE    1024
#define LISTEN_PORT 60000
#define MAX_USERS 100
#define USER_INFO 1024


int main(int argc, char *argv[]){
    int sock_recv;
    struct sockaddr_in  my_addr;
    int i, n, m, no_clients, newfd;
    fd_set readfds, active_fd_set;//,read_fd_set;
    int incoming_len;
    struct sockaddr_in remote_addr;
    int recv_msg_size;
    char buf[BUF_SIZE];
    int select_ret;
    char format[10];

    int fun[20] = {0}; // holds list of persons in fun group
    int fun_count = 0; // counts number of persons in fun group
    int wrk[20] = {0}; // holds list of persons in work group
    int wrk_count = 0; // counts number of persons in work group
    int loop, user_present, in_fun_gp, in_wrk_gp;
    char str[BUF_SIZE], temp[10];
    char users[MAX_USERS][USER_INFO] = {{""}};
    char who_str[] = "r@";
    char port[8];
    int user_avl[MAX_USERS] = {0};
    char clear[] = "\\clr";// clears client screen
    
    FD_ZERO(&readfds);
    FD_ZERO(&active_fd_set);/* zero out socket set */


    /* create socket for receiving */
    sock_recv=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_recv < 0){
        printf("socket() failed\n");
        exit(0);
    }
    
    /* make local address structure */
    memset(&my_addr.sin_zero, 0, sizeof(my_addr));  /* zero out structure */
    my_addr.sin_family = AF_INET;   /* address family */
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

    system("clear");
    printf("Listening on port 60000...... \n");

    FD_SET(sock_recv, &active_fd_set);  /* add socket to listen to */
    
    no_clients = sock_recv;

    while (1){
        readfds = active_fd_set;
        if (select(no_clients+1,&readfds,NULL,NULL,NULL) == -1){
            exit(4);
        }   

        for (i = 0; i<= no_clients; i++){
            if (FD_ISSET(i, &readfds)){
    //############################## 
    //      NEW USER
    //##############################         
                if (i == sock_recv){ 
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
                        strcat(str, "\n");
                        write(newfd, str, BUF_SIZE);
                    }
                } 
    //############################## 
    //      EXISTING USERS
    //##############################        
                else {
                    recv_msg_size=recv(i, buf, sizeof(buf), 0);
                    buf[recv_msg_size] = '\0';

                    // if user disconnects
                    if (recv_msg_size <= 0){ 
                        printf("%s went offline\n", users[i]);                    
                        close(i); 
                        FD_CLR(i, &active_fd_set);
                    }                     
                    else {        
        //############################## 
        //      USER LIST
        //############################## 
                        if(strcmp("\\l\n", buf) == 0){
                            // creating a list of peers to return to
                            memset(str, 0, BUF_SIZE);
                            strcat(str, "\n\tAvailable users:\n");
                            // creating list of available users
                            for (n = 0; n<= no_clients; n++){
                                if (FD_ISSET(n, &active_fd_set)) {
                                    // except the listener and ourselves
                                    if (n != sock_recv && n != i && (user_avl[n] == 0)){
                                        strcat(str, users[n]);
                                        strcat(str, "\n");
                                    }
                                }
                            }
                            if (user_avl[i] == 0){
                                strcat(str, users[i]);
                                strcat(str, " ***me\n");
                            }                            
                            write(i, str, BUF_SIZE);
                        } 
        //############################## 
        //      FUN GROUP
        //##############################                 
                        else if(strcmp("\\f\n", buf) == 0){
                            if (user_avl[i] == 1){
                                write(i, "You are unavailable... Exit group or P2P to join FUN group. \n", BUF_SIZE);
                            }
                            else{
                                memset(str, 0, BUF_SIZE);
                                // write(i, clear, strlen(clear));
                                write(i, "\n\t=====================================\n\tWelcome to the Fun Group - Exit(\\x)\n\t=====================================\n", BUF_SIZE);

                                fun[fun_count] = i;
                                fun_count = fun_count + 1;
                                user_avl[i] = 1;

                                strcat(str, users[i]);
                                strcat(str, " has joined group");
                                // announce user has joined group
                                for (n = 0; n<= fun_count; n++){
                                    if (FD_ISSET(fun[n], &active_fd_set)) {
                                        // except the listener and ourselves
                                        if (fun[n] != sock_recv && fun[n] != i )
                                            write(fun[n], str, BUF_SIZE);
                                    }
                                }
                            }
                            
                        }
        //############################## 
        //      WORK GROUP
        //##############################      
                        else if(strcmp("\\w\n", buf) == 0){
                            if (user_avl[i] == 1){
                                write(i, "You are unavailable... Exit group or P2P to join WORK group. \n", BUF_SIZE);
                            }
                            else{
                                memset(str, 0, BUF_SIZE);
                                write(i, "\n\t=====================================\n\tWelcome to the Work Group - Exit(\\x)\n\t=====================================\n", BUF_SIZE);
                                
                                wrk[wrk_count] = i;
                                wrk_count = wrk_count + 1;
                                user_avl[i] = 1;

                                strcat(str, users[i]);
                                strcat(str, " has joined group");
                                // announce user has joined group
                                for (n = 0; n<= wrk_count; n++){
                                    if (FD_ISSET(wrk[n], &active_fd_set)) {
                                        // except the listener and ourselves
                                        if (wrk[n] != sock_recv && wrk[n] != i )
                                            write(wrk[n], str, BUF_SIZE);
                                    }
                                }
                            }
                        } 
        //############################## 
        // EXIT ACTIVITY => GO AVAILABLE
        //##############################      
                        else if (strcmp("\\x\n", buf) == 0){
                            
                            // write(i, clear, strlen(clear));
                            // sleep(2);
                            write(i, "\nYou and now available\n", BUF_SIZE);
                            user_avl[i] = 0;

                            // checking if user is in wrk group
                            for (n = 0; n <= fun_count; n++){
                                if(fun[n] == i){
                                    fun[n] = '\0';

                                    strcpy(str, users[i]);
                                    strcat(str, " has left\n");
                                    // announce user has left
                                    for (n = 0; n<= fun_count; n++){
                                        if (FD_ISSET(fun[n], &active_fd_set)) {
                                            // except the listener and ourselves
                                            if (fun[n] != sock_recv && fun[n] != i )
                                                write(fun[n], str, BUF_SIZE);
                                        }
                                    }
                                    
                                }
                            }

                            // checking if user is in wrk group
                            for (n = 0; n <= wrk_count; n++){
                                if(wrk[n] == i){
                                    wrk[n] = '\0';

                                    strcpy(str, users[i]);
                                    strcat(str, " has left\n");
                                    // announce user has left
                                    for (n = 0; n<= wrk_count; n++){
                                        if (FD_ISSET(wrk[n], &active_fd_set)) {
                                            // except the listener and ourselves
                                            if (wrk[n] != sock_recv && wrk[n] != i )
                                                write(wrk[n], str, BUF_SIZE);
                                        }
                                    }
                                    
                                }
                            } 
                           
                        } 
        //############################## 
        //      CHAT REQUEST
        //############################## 
                        else if(strcmp("@", buf) == 0){
                            
                            memset(str, 0, BUF_SIZE);
                            user_present = 0;

                            for (n = 1; n <= recv_msg_size; n++)
                                str[n-1] = buf[n];
                            
                            for (n = 0; n<= no_clients; n++){                                
                                if (strcmp(users[n], str) == 0)
                                    user_present++;
                            }

                            if (user_present == 0)
                                write(i, "\nInvalid username!\n", BUF_SIZE); 
                            else if (user_present == 1){
                                strcpy(str, "@");
                                strcat(str, users[i]);
                                getpeername(sock_recv , (struct sockaddr*)&remote_addr , (socklen_t*)&incoming_len);
                                strcat(str, ",");
                                strcat(str, inet_ntoa(remote_addr.sin_addr));
                                strcat(str, ",");
                                sprintf(port, "%d", ntohs(remote_addr.sin_port));
                                strcat(str, port); 
                                write(n, str, BUF_SIZE);
                            }                    
                        }                   
        //############################## 
        //      SEND MSG TO GROUPS
        //##############################           
                        else {
                            // reseting variables
                            memset(str, 0, BUF_SIZE);
                            in_fun_gp = 0;
                            in_wrk_gp = 0;

                            // checking if user is in fun group
                            for (n = 0; n <= fun_count; n++){
                                if(fun[n] == i){
                                    in_fun_gp = 1;
                                    break;
                                }
                            }

                            // checking if user is in wrk group
                            for (n = 0; n <= wrk_count; n++){
                                if(wrk[n] == i){
                                    in_wrk_gp = 1;
                                    break;
                                }
                            }  

                            // sending msg to fun group
                            if (in_fun_gp == 1){
                                for (n = 0; n<= fun_count; n++){
                                    if (FD_ISSET(fun[n], &active_fd_set)) {
                                        // except the listener and ourselves
                                        if (fun[n] != sock_recv && fun[n] != i ){
                                            strcat(str, "\t\t\t\t<");
                                            strcat(str, users[i]);
                                            strcat(str, "> ");
                                            strcat(str, buf);
                                            write(fun[n], str, BUF_SIZE);
                                        }
                                    }
                                }
                            } 
                            // sending msg to work group
                            else if (in_wrk_gp == 1){
                                for (n = 0; n<= wrk_count; n++){
                                    if (FD_ISSET(wrk[n], &active_fd_set)) {
                                        // except the listener and ourselves
                                        if (wrk[n] != sock_recv && wrk[n] != i ){
                                            strcat(str, "\t\t\t\t<");
                                            strcat(str, users[i]);
                                            strcat(str, "> ");
                                            strcat(str, buf);
                                            write(wrk[n], str, BUF_SIZE);
                                        }
                                    }
                                }
                            }  
                        }//// end of work group
                                     
                    }
                }
            }
        }// end of for loop goes through connects
    }// end while to listen and send
}// end of main


