#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h> 
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "user_reader.h"
#include "udp_tx.h"
#include "list.h"
#include "stalk.h"

// Loop that thread runs till cancled
static void* upd_transmit_loop(void* arg);

static pthread_t upd_tx_pid;

// Socket that would send the packets recieved from a user's input
static int tx_socket_desc;

static int port;

static struct sockaddr_in sin;

#ifdef DEBUG
    // Variables for debugging purposes, unused otherwise
    // Log file to print log messages to
    static char* udp_tx_log_file_name = "./log/udp_tx_log.log";
    static FILE* udp_tx_log;
    // Start time fetched in init function
    static struct tm udp_tx_start_time;
    // gets the current time relative to the start time
    static struct tm udp_tx_current_time(){
        time_t time_temp;
        time(&time_temp);
        struct tm time_now = *localtime(&time_temp);
        // only minutes and seconds are used for printout
        time_now.tm_min -= udp_tx_start_time.tm_min;
        time_now.tm_sec -= udp_tx_start_time.tm_sec;
        return time_now;
    }
    #define UDP_TX_LOG(_message) STALK_LOG(udp_tx_log, _message, udp_tx_current_time())
#else
    #define UDP_TX_LOG(_message) ;
#endif

void udp_tx_init(char* tx_machine, char* tx_port){
    #ifdef DEBUG
        // fetch start time, bust be done before thread as not threadsafe
        udp_tx_start_time = get_start_time();
        // Open log file
        udp_tx_log = fopen(udp_tx_log_file_name, "w");
        if(udp_tx_log == NULL){
            fprintf(stderr, "Invalid File name %s, %i", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
    #endif
    // TODO - Create UPD socket and bind it to the port
    
    // We begin by retrieving the port from the input to our function
    char* tempPtr;
    port = strtol(tx_port, &tempPtr, 10);
    if (port == 0)
    {
        fprintf(stderr, "Error in TX port number: %s", tx_port);
    }
    printf("Sending Data through UDP on port: %s\n", tx_port);

    // We continue by using the getaddrinfo function to connect the hostname to an 
    // IP address
    // struct addrinfo hints;
    // struct addrinfo *result, *temp;
    struct hostent* ht;
    // int sockfd;

    // memset(&hints, 0, sizeof(struct addrinfo));
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_DGRAM;
    // // I can also test removing the two lines below
    // hints.ai_flags = 0;
    // hints.ai_protocol = 0;

    // printf("Okay, here\n");

    ht = gethostbyname(tx_machine);

    if (ht == 0)
    {
        printf("Unknown Host\n");
    }

    bcopy((char *)ht->h_addr, (char*)&sin.sin_addr, ht->h_length);

    // int value = getaddrinfo(tx_machine, tx_port, &hints, &result);
    // if (value != 0)
    // {
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(value));
    //     exit(EXIT_FAILURE);
    // }

    // // Try each address structures until one of it is successful 
    // for (temp = result; temp != NULL; temp = temp->ai_next)
    // {
    //     sockfd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);

    //     if (sockfd == -1) 
    //         continue; 

    //     if (connect(sockfd, temp->ai_addr, temp->ai_addrlen) != -1)
    //         break; // Sucess in connecting 
        
    //     close(sockfd);
    // }   

    // if (temp == NULL) // No address succeeded in connecting 
    // {
    //     fprintf(stderr, "Could not establish a successful conection to the host provided\n");
    //     exit(EXIT_FAILURE);
    // }

    // freeaddrinfo(result);

    sin.sin_family = AF_INET;
    sin.sin_port = port;

    // Creating the socket
    tx_socket_desc = socket(PF_INET, SOCK_DGRAM, 0);
    if (tx_socket_desc == -1)
    {
        UDP_TX_LOG("ERROR: invalid socket descriptor in udp_tx\n");
    }

    pthread_create(&upd_tx_pid, NULL, upd_transmit_loop, NULL);
}

void udp_tx_destroy(){
    // Stops the thread
    pthread_cancel(upd_tx_pid);

    #ifdef DEBUG
        // close logging file
        fclose(udp_tx_log);
    #endif

    // TODO - Close the UPD socket
    close(tx_socket_desc);

    // Waits until thread finishes before continuing 
    pthread_join(upd_tx_pid, NULL);
    printf("Fished UPD Transmitor\n");
}

static void* upd_transmit_loop(void* arg){
    printf("Started UDP Transmitor on UDP_TX\n");
    UDP_TX_LOG("Started UDP TX Loop\n");
    // TODO - coninually wait for tx_list to have messages to send to other s-talk instance
    char* msg = NULL;
    while(1){
        // pthread_testcancel();     
        if(user_reader_txList_getNext(&msg))
        {
            UDP_TX_LOG("ERROR: Cannot retrieve the next message from the list.\n");
            // TODO: ERROR HANDLING
        }

        int bytesTx = sendto(tx_socket_desc, msg, MAX_MESSAGE_SIZE, 0, (struct sockaddr*) &sin, sizeof(struct sockaddr_in));

        if(bytesTx == 0){
            UDP_TX_LOG("ERROR: Sent 0 bytes \n");
            // TODO 
        }
        else if(bytesTx == -1){
            UDP_TX_LOG("ERROR: Sent Invalid Message\n");
            // TODO: error recieving message
        }
        else
        {
            printf("First is here\n");
            UDP_TX_LOG("Message Sent Successfully\n");  
            // Question: Why do we have to add each message to a list and then output 
            // the messages added to the list for the assignment?
            if (strcmp(msg, "!\n\0") == 0)
            {
                UDP_TX_LOG("Received Termination Request; now preparing for termination\n");
                stalk_initiateShutdown();
                return NULL;
            }
        }
    }
    return NULL;
}
