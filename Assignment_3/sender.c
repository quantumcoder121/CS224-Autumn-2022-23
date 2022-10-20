// including header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

// macros
#define MAXINACTIVETIME 10 // maximum time (in minutes) of inactivity after which program will terminate
#define MAXMSGLEN 100 // maximum length of message in terms of characters

// main function with command line arguments
int main(int argc, char* argv[]){
    
    // declaring variables
    int sockfd;
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXMSGLEN];
    socklen_t addr_len = sizeof their_addr;
    int count = 0;
    clock_t start_time;
    FILE* output_file = fopen("sender.txt", "w");
    
    // detecting command line errors
    if (argc != 5) {
        fprintf(stderr, "usage: ./sender <sender_port> <receiver_port> <timeout_value> <packet_count>\n");
        exit(1);
    }

    // check if output file exists
    if (output_file == NULL){
        fprintf(stderr, "file sender.txt does not exist\n");
        exit(1);
    }

    start_time = clock(); // starting the clock
    
    // main while loop
    while(1){

        // checking for inactivity
        if (clock() > start_time + 60000 * MAXINACTIVETIME){
            printf("terminating program due to inactivity\n");
            break;
        }

        /*
        this part will send a packet to the receiver port
        */

        // initializing variables
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // IP version compatible
        hints.ai_socktype = SOCK_DGRAM; // Datagram socket

        // getting metadata for the receiver port
        if ((rv = getaddrinfo("localhost", argv[2], &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
            return 1;
        }

        // looping through all the results in the metadata and making a socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
            break;
        }

        // detecting socket errors
        if (p == NULL) {
            fprintf(stderr, "socket error: failed to make a socket\n");
            return 2;
        }

        // making the message (with dynamic memory allocation)
        int str_len = snprintf(NULL, 0, "Packet:%d", count + 1);
        char* msg = malloc(str_len + 1);
        snprintf(msg, str_len + 1, "Packet:%d", count + 1);

        // sending the message
        if ((numbytes = sendto(sockfd, msg, strlen(msg), 0, p->ai_addr, p->ai_addrlen)) == -1) {
            fprintf(stderr, "sendto error\n");
            continue;
        }

        // printing the sent message
        printf("sent \"%s\"\n", msg);
        fprintf(output_file, "sent \"%s\"\n", msg);

        // closing the socket
        freeaddrinfo(servinfo);
        close(sockfd);

        /*
        this part will wait for an acknowledgement from the receiver port
        */

        // initializing variables
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // IP version compatible
        hints.ai_socktype = SOCK_DGRAM; // Datagram socket
        hints.ai_flags = AI_PASSIVE; // localhost

        // getting metadata for the sender port
        if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
            return 1;
        }

        // looping through all the results in the metadata and making a socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                continue;
            }
            break;
        }

        // detecting socket errors
        if (p == NULL) {
            fprintf(stderr, "socket error: failed to make a socket\n");
            return 2;
        }

        /*
        for the timeout functionality we will be using the select() system call that waits for a certain duration until a process terminates
        */

        // declaring variables for select() system call
        fd_set fds;
        int n;
        struct timeval tv;

        // initializing these variables
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        tv.tv_sec = atoi(argv[3]);
        tv.tv_usec = 0;

        n = select(sockfd + 1, &fds, NULL, NULL, &tv); // select() system call
        
        // loop will be continued if the retransmission timer expires before receiving an acknowledgement
        if (n == 0){
            printf("retransmission timer expired\n");
            fprintf(output_file, "retransmission timer expired\n");
            freeaddrinfo(servinfo);
            close(sockfd);
            continue;
        }

        // receiving the acknowledgement
        numbytes = recvfrom(sockfd, buf, MAXMSGLEN - 1 , 0, (struct sockaddr *) &their_addr, &addr_len);

        // analyzing the acknowledgement
        if (numbytes != -1) {
            char* ack = strtok(buf, ":");
            ack = strtok(NULL, ":");
            count = atoi(ack) - 1;
            if (count == atoi(argv[4])){
                printf("sent all packets successfully\n");
                fprintf(output_file, "sent all packets successfully\n");
                break;
            }
            start_time = clock();
            freeaddrinfo(servinfo);
            close(sockfd);
        }

        // packet will be retransmitted in case of any errors in receiving an acknowledgement
        else {
            freeaddrinfo(servinfo);
            close(sockfd);
            continue;
        }
    }

    return 0; // program ends
}