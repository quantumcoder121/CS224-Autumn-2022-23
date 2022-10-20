// including header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

// macros
#define MAXINACTIVETIME 10 // maximum time (in minutes) of inactivity after which program will terminate
#define MAXMSGLEN 100  // maximum length of message in terms of characters

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
    socklen_t addr_len;
    int seq_no = 1;
    int rec_no;
    clock_t start_time;
    FILE* output_file = fopen("receiver.txt", "w");

    // detecting command line errors
    if (argc != 4) {
        fprintf(stderr, "usage: ./receiver <receiver_port> <sender_port> <drop_probability>\n");
        exit(1);
    }

    // check if output file exists
    if (output_file == NULL){
        fprintf(stderr, "file receiver.txt does not exist\n");
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
        this part will wait for a message from the sender port
        */

        // initializing variables
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // IP version compatible
        hints.ai_socktype = SOCK_DGRAM; // Datagram socket
        hints.ai_flags = AI_PASSIVE; // localhost

        // getting metadata for the receiver port
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
            fprintf(stderr, "socket error: failed to make a socket 1\n");
            return 2;
        }

        addr_len = sizeof their_addr;

        // receiving the message
        numbytes = recvfrom(sockfd, buf, MAXMSGLEN - 1 , 0, (struct sockaddr *) &their_addr, &addr_len);

        // analyzing the message
        if (numbytes != -1) {
            char* msg = strtok(buf, ":");
            msg = strtok(NULL, ":");
            rec_no = atoi(msg);
            start_time = clock();
            if (rec_no == seq_no){
                float rand_var = ((float) rand()) / ((float) RAND_MAX);
                if (rand_var < atof(argv[3])) {
                    printf("packet dropped\n");
                    fprintf(output_file, "packet dropped\n");
                    freeaddrinfo(servinfo);
                    close(sockfd);
                    continue;
                }
                seq_no = seq_no + 1;
            }
        }

        // no acknowledgement will be transmitted in case of any errors in receiving the message
        else {
            freeaddrinfo(servinfo);
            close(sockfd);
            continue;
        }

        // closing the socket
        freeaddrinfo(servinfo);
        close(sockfd);

        /*
        this part will send an acknowledgement to the sender port
        */

        // initializing variables
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;  // IP version compatible
        hints.ai_socktype = SOCK_DGRAM; // Datagram socket
        
        // getting metadata for the sender port
        if ((rv = getaddrinfo("localhost", argv[2], &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and make a socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
            break;
        }

        // detecting socket errors
        if (p == NULL) {
            fprintf(stderr, "socket error: failed to make a socket 2\n");
            return 2;
        }

        // making the acknowledgement (with dynamic memory allocation)
        int str_len = snprintf(NULL, 0, "Acknowledgement:%d", seq_no);
        char* ack = malloc(str_len + 1);
        snprintf(ack, str_len + 1, "Acknowledgement:%d", seq_no);

        // sending the acknowledgement
        if ((numbytes = sendto(sockfd, ack, strlen(ack), 0, p->ai_addr, p->ai_addrlen)) == -1) {
            fprintf(stderr, "sendto error\n");
            exit(1);
        }

        // printing the sent message
        printf("sent \"%s\"\n", ack);
        fprintf(output_file, "sent \"%s\"\n", ack);

        // closing the socket
        freeaddrinfo(servinfo);
        close(sockfd);
    }

    return 0; // program ends
}
