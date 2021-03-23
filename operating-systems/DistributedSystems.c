//
//  DistributedSystems.c
//  OSTEP
//
//  Created by Annalise Tarhan on 3/19/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/fcntl.h>

#define SERVER_PORT 10000
#define CLIENT_PORT 20000
#define LOCAL_HOST_NAME "Annalises-MacBook-Pro-2.local"

//#define MAX_UDP 65507     // Realistic number
#define MAX_UDP 655         // Usable number

#define TIMEOUT 4
#define MICROSECS_IN_SECONDS 1000000

char *short_message = "Hello world!";
char *long_message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras tempor leo sit amet nibh tincidunt varius. Maecenas nec pulvinar odio. Nulla eu dolor urna. Duis eu purus semper orci tempus volutpat tempor et neque. Proin at tincidunt leo. Etiam eu felis ac risus ultrices mollis vel vel nunc. Aliquam eget faucibus felis.Nunc commodo augue ut dui aliquam, sit amet malesuada orci lobortis. Donec imperdiet, urna nec porttitor fermentum, ex lectus viverra libero, et suscipit leo neque nec lorem. Praesent id lacus elementum, rutrum metus ac, sagittis enim. Sed eu euismod urna. Aenean ultricies id sem eget pharetra. In tempus venenatis nulla. Quisque consequat non felis at elementum. Phasellus a elementum ante. Etiam sollicitudin neque nunc, et blandit magna placerat sed. Sed sit amet auctor massa, sed semper est. Aenean feugiat augue ut eros bibendum consequat. Praesent rhoncus aliquet commodo. Quisque hendrerit, erat et rutrum porta, enim turpis faucibus libero, at faucibus tellus ligula quis arcu. Aenean id interdum metus. Nullam sem lorem, vehicula sed mattis sit amet, luctus vehicula orci. Integer eget molestie magna. Quisque pulvinar sapien nisi, nec iaculis nisi vehicula sit amet. Nullam placerat, magna sed porttitor elementum, enim ante vestibulum nunc, non fringilla tellus tortor quis tortor. In tristique vulputate est volutpat pharetra. Mauris sollicitudin lacus non mauris rutrum tempus. Pellentesque sit amet enim mattis, tempus lacus ac, pellentesque lectus. Vivamus quis hendrerit lorem, dapibus tempor neque. Aliquam ut pharetra mauris. Nullam aliquam dui vel libero feugiat, ac aliquet felis dignissim. Curabitur vestibulum non mi ultricies congue. Mauris pretium leo tellus, in condimentum velit elementum at. Fusce sed odio a massa vehicula fringilla sed nec quam. Duis consectetur non ligula ut dignissim. Nulla placerat est sem, eu viverra nisl cursus sed. Morbi porta, erat in auctor iaculis, diam enim eleifend ligula, id rhoncus arcu libero quis neque. Cras sollicitudin arcu a eros blandit iaculis. Nunc faucibus, lorem et tincidunt laoreet, nisl lectus mattis eros, ut condimentum metus leo a ante. Nunc ut urna sed velit feugiat gravida vel sed tortor. Integer dictum diam eu leo egestas, nec posuere risus luctus. Pellentesque feugiat eu velit vel ullamcorper. Fusce dictum tristique nibh. Aenean non accumsan urna. Vestibulum faucibus posuere vehicula. Maecenas et metus odio. Donec pretium arcu id ipsum ullamcorper, eget condimentum mauris blandit. Ut eget dui tincidunt lacus sollicitudin sodales. Integer purus nisi, euismod dignissim mauris in, iaculis fringilla velit. Mauris blandit orci eu est auctor, id bibendum urna cursus. Proin mollis orci vel arcu semper faucibus. Nunc quis quam laoreet, accumsan quam mattis, rhoncus erat. Duis vel neque posuere, tempor tellus in, fermentum mi. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Phasellus malesuada vehicula turpis vitae laoreet.";


/* Utility functions */
int UDP_Open(int port) {
    int sd;
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        return -1;
    }
    struct sockaddr_in myaddr;
    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(port);      // Deals with byte ordering
    myaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        close(sd);
        return -1;
    }
    return sd;
}

void UDP_FillSockAddr(struct sockaddr_in *addr, int port) {
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_aton(LOCAL_HOST_NAME, &addr->sin_addr);
}

int UDP_Write(int sd, struct sockaddr_in *addr, char *buffer, int n) {
    int addr_len = sizeof(struct sockaddr_in);
    return (int) sendto(sd, buffer, n, 0, (struct sockaddr *) addr, addr_len);
}

int UDP_Read(int sd, struct sockaddr_in *addr, char *buffer, int n) {
    int len = sizeof(struct sockaddr_in);
    return (int) recvfrom(sd, buffer, n, 0, (struct sockaddr *) addr, (socklen_t *) &len);
}




/*
 Question 1
 
 client() - sends a default message
 server() - sends a default response
 */

void *server() {
    int sd = UDP_Open(SERVER_PORT);
    assert(sd > -1);
    while (1) {
        struct sockaddr_in addr;
        char message[BUFSIZ];
        int rc = UDP_Read(sd, &addr, message, BUFSIZ);
        printf("Server received: %s\n", message);
        if (rc > 0) {
            char reply[BUFSIZ];
            sprintf(reply, "acknowledged");
            rc = UDP_Write(sd, &addr, reply, BUFSIZ);
        }
    }
}

void client() {
    int sd = UDP_Open(CLIENT_PORT);
    assert(sd > -1);
    
    struct sockaddr_in sendAddr, rcvAddr;
    UDP_FillSockAddr(&sendAddr, SERVER_PORT);
    
    char message[BUFSIZ];
    sprintf(message, "hello world");
    int rc = UDP_Write(sd, &sendAddr, message, BUFSIZ);
    
    if (rc > 0) {
        UDP_Read(sd, &rcvAddr, message, BUFSIZ);
        printf("Client received: %s\n", message);
    } else {
        perror("client");
        exit(EXIT_FAILURE);
    }
    close(sd);
}

void run() {
    pthread_t thread;
    pthread_create(&thread, NULL, &server, NULL);
    sleep(1);                       // Give server a chance to start
    client();
    pthread_join(thread, NULL);     // Will never return, server is while (1)
}





/*
 Question 2
 
 Server:
 * struct server_connection
 
 * struct server_connection *start_server();
 * void receive_message(struct server_connection c, struct sockaddr_in *addr, char *message);
 * void send_response(struct server_connection c, struct sockaddr_in *addr);
 
 Client:
 * struct client_connection
 
 * struct client_connection *open_client_port();
 * void send_message(struct client_connection c, char *message);
 * void await_response(struct client_connection *c);
 
 */

struct server_connection {
    int sd;
    char *response;
} server_connection;

struct client_connection {
    int sd;
    struct sockaddr_in clientAddr, serverAddr;
    char response[BUFSIZ];
} client_connection;

struct server_connection *start_server() {
    int sd = UDP_Open(SERVER_PORT);
    if (sd < 0) {
        perror("Error starting server");
        exit(EXIT_FAILURE);
    }
    struct server_connection *c = malloc(sizeof(server_connection));
    c->sd = sd;
    c->response = "Goodbye world";
    return c;
}

void receive_message(struct server_connection c, struct sockaddr_in *addr, char *message) {
    bzero(message, BUFSIZ);
    int rc = UDP_Read(c.sd, addr, message, BUFSIZ);
    if (rc < 0) {
        perror("Error receiving message");
    }
}

void send_response(struct server_connection c, struct sockaddr_in *addr) {
    int rc = UDP_Write(c.sd, addr, c.response, BUFSIZ);
    if (rc < 0) {
        perror("Error sending response");
    }
}

struct client_connection *open_client_port() {
    int sd = UDP_Open(CLIENT_PORT);
    if (sd < 0) {
        perror("Error opening client port");
        exit(EXIT_FAILURE);
    }
    struct client_connection *c = malloc(sizeof(client_connection));
    c->sd = sd;
    UDP_FillSockAddr(&c->serverAddr, SERVER_PORT);
    return c;
}

void send_message(struct client_connection c, char *message) {
    int rc = UDP_Write(c.sd, &c.serverAddr, message, BUFSIZ);
    if (rc < 0) {
        perror("Error sending client message");
        exit(EXIT_FAILURE);
    }
}

void await_response(struct client_connection *c) {
    bzero(c->response, BUFSIZ);
    int rc = UDP_Read(c->sd, &c->clientAddr, c->response, BUFSIZ);
    if (rc < 0) {
        perror("Error reading message");
        exit(EXIT_FAILURE);
    }
}

void *server2() {
    struct server_connection *c = start_server();
    char message[BUFSIZ];
    printf("Server started\n");
    while (1) {
        struct sockaddr_in addr;
        receive_message(*c, &addr, message);
        printf("Server received: %s\n", message);
        send_response(*c, &addr);
    }
}

void client2() {
    struct client_connection *c = open_client_port();
    char *message = "Hello world!";
    send_message(*c, message);
    await_response(c);
    printf("Client received: %s\n", c->response);
    close(c->sd);
    free(c);
}

void run2() {
    pthread_t thread;
    pthread_create(&thread, NULL, &server2, NULL);
    sleep(1);                       // Give server a chance to start
    client2();
    pthread_join(thread, NULL);     // Will never return
}





/*
 Question 3
 
 In this model, communication between client and server is mostly one-way.
 The client sends messages and receives acknowledgements in returns, nothing more.
 
 Sample output, where server sleeps for TIMEOUT+1 seconds before responding:
 
 Server started
 Sending message...
 Server received: Hello world!
 Timeout
 Sending message...
 Server received: Hello world!
 Client received: Acknowledged
 
  */

/* SERVER CODE */
struct server_connection *start_server3() {
    int sd = UDP_Open(SERVER_PORT);
    if (sd < 0) {
        perror("Error starting server");
        exit(EXIT_FAILURE);
    }
    
    /* Initialize connection struct */
    struct server_connection *c = malloc(sizeof(server_connection));
    c->sd = sd;
    c->response = "Acknowledged";
    return c;
}

void *server3() {
    struct server_connection *c = start_server3();
    char message[MAX_UDP];
    
    printf("Server started\n");
    while (1) {
        struct sockaddr_in addr;
        receive_message(*c, &addr, message);
        printf("Server received: %s\n", message);
        sleep(TIMEOUT + 1);
        send_response(*c, &addr);
    }
}

/* CLIENT CODE */
struct client_connection3 {
    int sd, done, done_or_timeout;
    struct sockaddr_in clientAddr, serverAddr;
    char message[MAX_UDP];
    char response[MAX_UDP];
    pthread_mutex_t lock;
    pthread_cond_t condition;
} client_connection3;

struct client_connection3 *open_client_port3(char *message) {
    /* Open port */
    int sd = UDP_Open(CLIENT_PORT);
    if (sd < 0) {
        perror("Error opening client port");
        exit(EXIT_FAILURE);
    }
    
    /* Configure socket */
    int buffer_size = MAX_UDP;     // Buffer needs to be bigger than the default to accommodate a full UDP datagram
    int rc = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    if (rc < 0) {
        perror("Error setting sockopt");
        exit(EXIT_FAILURE);
    }
    
    /* Initialize connection struct */
    struct client_connection3 *c = malloc(sizeof(client_connection3));
    c->sd = sd;
    c->done = 0;
    c->done_or_timeout = 0;
    UDP_FillSockAddr(&c->serverAddr, SERVER_PORT);
    
    /* Initialize condition variable */
    rc = pthread_mutex_init(&c->lock, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&c->condition, NULL);
    assert(rc == 0);
    
    /* Set message */
    int i = -1;
    do {
        i++;
        c->message[i] = message[i];
    } while (message[i] != '\0');

    return c;
}

void *timer_function(void *client) {
    struct client_connection3 *c = (struct client_connection3 *) client;
    sleep(TIMEOUT);
    
    pthread_mutex_lock(&c->lock);

    printf("Timeout\n");
    c->done_or_timeout = 1;
    int rc = pthread_cond_signal(&c->condition);
    assert(rc == 0);
    rc = pthread_mutex_unlock(&c->lock);
    assert(rc == 0);
    
    return NULL;
}

void *send_function(void *client) {
    struct client_connection3 *c = (struct client_connection3 *) client;

    printf("Sending message...\n");
    int rc = UDP_Write(c->sd, &c->serverAddr, c->message, MAX_UDP);
    if (rc < 0) {
        perror("Error sending message");
        exit(EXIT_FAILURE);
    }
    
    /* Wait for response. Don't lock until after UDP_Read returns. */
    rc = UDP_Read(c->sd, &c->clientAddr, c->response, MAX_UDP);
    pthread_mutex_lock(&c->lock);
    
    /* In case of failure, act like a timeout */
    if (rc < 0) {
        perror("Error sending message");
        
    /* Otherwise, mark network call complete. */
    } else {
        c->done = 1;
    }
    
    /* In either case, signal calling function. */
    c->done_or_timeout = 1;
    rc = pthread_cond_signal(&c->condition);
    assert(rc == 0);
    rc = pthread_mutex_unlock(&c->lock);
    assert(rc == 0);
    return NULL;
}

void send_message3(struct client_connection3 *c) {
    pthread_t timer, sender;
    
    while (!c->done) {
        /* Reset done_or_timeout and launch new timer and sender threads. */
        c->done_or_timeout = 0;
        pthread_create(&timer, NULL, timer_function, c);
        pthread_create(&sender, NULL, send_function, c);
                
        /* Standard condition variable semantics */
        pthread_mutex_lock(&c->lock);
        while (!c->done_or_timeout) {
            pthread_cond_wait(&c->condition, &c->lock);
        }
        pthread_mutex_unlock(&c->lock);
        
        // Decided not to worry about killing pthreads. If it's the timer that's still going, no big deal, I'll never be looking at the done variable again anyway, since I've already gotten the ACK. If the network call is hung, it's a waste of resources but doesn't really matter. Resources will be reclaimed after the client eventually receives a response and exits. Admittedly, it would be bad if two network calls completed at the same time and corrupted the response buffer. Not sure how to deal with that, since wrapping the UDP_Write in a mutex would lock it permanently if it hangs.
    }
}

// Client is brittle. Can only send one pre-set message one time.
void client3(char *message) {
    struct client_connection3 *c = open_client_port3(message);
    send_message3(c);
    printf("Client received: %s\n", c->response);
    close(c->sd);
    free(c);
}


void run3() {
    pthread_t thread;
    pthread_create(&thread, NULL, &server3, NULL);
    sleep(1);
    char *message = "Hello world!";
    client3(message);
}





/*
 Question 4
 
 This protocol rather inelegantly sends an empty message to tell server that the entire message has been sent.
 */

/* SERVER CODE */
char *receive_message4(struct server_connection c, struct sockaddr_in *addr) {
    char *message = malloc(0);      // Message will accumulate here
    char buf[MAX_UDP];              // Temporary buffer to hold new part of message
    int rc, num_parts = 1;
    do {
        message = realloc(message, MAX_UDP * num_parts);    // Resize message to be big enough for new part
        bzero(buf, MAX_UDP);                                // Clear temporary buffer
        rc = UDP_Read(c.sd, addr, buf, MAX_UDP);            // Read message into temporary buffer
        if (rc < 0) {
            perror("Error receiving message");
        }
        strcat(message, buf);                               // Add temporary buffer contents to message
        num_parts++;
    } while (rc > 0);                                       // Keep reading until an empty message is received
    return message;
}

void *server4() {
    struct server_connection *c = start_server3();
    
    printf("Server started\n\n");
    while (1) {
        struct sockaddr_in addr;
        char *message = receive_message4(*c, &addr);
        printf("Server received: %s\n", message);
        assert(strcmp(message, long_message) == 0);
        free(message);
        send_response(*c, &addr);
    }
}

/* CLIENT CODE */
struct client_connection4 {
    int sd, done, done_or_timeout;
    struct sockaddr_in clientAddr, serverAddr;
    char *message;
    char response[MAX_UDP];
    pthread_mutex_t lock;
    pthread_cond_t condition;
} client_connection4;

struct client_connection4 *open_client_port4(char *message) {
    /* Open port */
    int sd = UDP_Open(CLIENT_PORT);
    if (sd < 0) {
        perror("Error opening client port");
        exit(EXIT_FAILURE);
    }
    
    /* Configure socket */
    int buffer_size = MAX_UDP;     // Buffer needs to be bigger than the default to accommodate a full UDP datagram
    int rc = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    if (rc < 0) {
        perror("Error setting sockopt");
        exit(EXIT_FAILURE);
    }
    
    /* Initialize connection struct */
    struct client_connection4 *c = malloc(sizeof(client_connection4));
    c->sd = sd;
    c->done = 0;
    c->done_or_timeout = 0;
    UDP_FillSockAddr(&c->serverAddr, SERVER_PORT);
    
    /* Initialize condition variable */
    rc = pthread_mutex_init(&c->lock, NULL);
    assert(rc == 0);
    rc = pthread_cond_init(&c->condition, NULL);
    assert(rc == 0);
    
    /* Set message */
    c->message = message;

    return c;
}

void *send_function4(void *client) {
    struct client_connection4 *c = (struct client_connection4 *) client;

    printf("Sending message...\n");
        
    int index = 0, part = 1, rc;
    unsigned long msg_length = strlen(c->message);
    char buf[MAX_UDP];
    
    while (index < msg_length) {
        bzero(buf, MAX_UDP);
        int i = 0;
        while (i < MAX_UDP && index < msg_length) {
            buf[i] = c->message[index];
            i++;
            index++;
        }
        rc = UDP_Write(c->sd, &c->serverAddr, buf, MAX_UDP);
        printf("Sent part %i of the message.\n", part);
        if (rc < 0) {
            perror("Error sending message");
            exit(EXIT_FAILURE);
        }
        part++;
    }
    
    /* Send empty message to mark end of stream */
    rc = UDP_Write(c->sd, &c->serverAddr, buf, 0);
    if (rc < 0) {
        perror("Error sending message");
        exit(EXIT_FAILURE);
    }
    
    printf("Finished sending message\n\n");
    
    /* Wait for response */
    rc = UDP_Read(c->sd, &c->clientAddr, c->response, MAX_UDP);
    pthread_mutex_lock(&c->lock);
    if (rc < 0) {
        perror("Error sending message");
    } else {
        c->done = 1;
    }
    
    /* Let main function know read is complete */
    c->done_or_timeout = 1;
    rc = pthread_cond_signal(&c->condition);
    assert(rc == 0);
    rc = pthread_mutex_unlock(&c->lock);
    assert(rc == 0);
    
    return NULL;
}

void *timer_function4(void *client) {
    struct client_connection4 *c = (struct client_connection4 *) client;
    sleep(TIMEOUT);
    
    pthread_mutex_lock(&c->lock);

    printf("Timeout\n");
    c->done_or_timeout = 1;
    int rc = pthread_cond_signal(&c->condition);
    assert(rc == 0);
    rc = pthread_mutex_unlock(&c->lock);
    assert(rc == 0);
    
    return NULL;
}

void send_message4(struct client_connection4 *c) {
    pthread_t timer, sender;
    
    while (!c->done) {
        c->done_or_timeout = 0;
        pthread_create(&timer, NULL, timer_function4, c);
        pthread_create(&sender, NULL, send_function4, c);
                
        pthread_mutex_lock(&c->lock);
        while (!c->done_or_timeout) {
            pthread_cond_wait(&c->condition, &c->lock);
        }
        pthread_mutex_unlock(&c->lock);
    }
}

void client4(char *message) {
    struct client_connection4 *c = open_client_port4(message);
    send_message4(c);
    printf("\nClient received: %s\n", c->response);
    close(c->sd);
    free(c);
}

void run4() {
    pthread_t thread;
    pthread_create(&thread, NULL, &server4, NULL);
    sleep(1);
    char *message = long_message;
    client4(message);
}





/*
 Question 5
 
 This implementation uses four bytes at the beginning of each message
 to organize messages. The first two indicate the total number of
 messages (max 99), the second two are for that message's seqnum (1-99).
 
 The server code has a problem, though, because if one of the parts of
 a message is lost it hangs. Fix by implementing a timeout similar to
 the one on the client side.
 */

/* SERVER CODE */

char *receive_message5(struct server_connection c, struct sockaddr_in *addr) {
    char *message;
    char buf[MAX_UDP];
    int rc, num_parts, seqnum, start_index;
    int payload_size = MAX_UDP - 4;
    int parts_received;
    
    /* Receive first message */
    bzero(buf, MAX_UDP);
    rc = UDP_Read(c.sd, addr, buf, MAX_UDP);
    if (rc < 0) {
        perror("Error receiving message");
    }
    /* Calculate message size */
    num_parts = 10 * (buf[0]-'0') + (buf[1]-'0');
    message = malloc(num_parts * payload_size);
    assert(message != NULL);
    bzero(message, num_parts * payload_size);
    
    /* Save first message received */
    seqnum = 10 * (buf[2]-'0') + (buf[3]-'0') - 1;  // -1 because seqnum should start at 0, parts start at 1
    start_index = payload_size * seqnum;
    strncpy(&message[start_index], &buf[4], payload_size);
    
    parts_received = 1;
    printf("Message %i received\n", seqnum);
    
    while (parts_received < num_parts) {
        bzero(buf, MAX_UDP);
        rc = UDP_Read(c.sd, addr, buf, MAX_UDP);
        if (rc < 0) {
            perror("Error receiving message");
        }
        seqnum = 10 * (buf[2]-'0') + (buf[3]-'0') - 1;
        start_index = payload_size * seqnum;
        strncpy(&message[start_index], &buf[4], payload_size);
        parts_received++;
        printf("Message %i received\n", seqnum);
    }

    return message;
}

void *server5() {
    struct server_connection *c = start_server3();
    
    printf("Server started\n\n");
    while (1) {
        struct sockaddr_in addr;
        char *message = receive_message5(*c, &addr);
        assert(strcmp(message, long_message) == 0);
        send_response(*c, &addr);
        free(message);
    }
}

void *send_function5(void *client) {
    struct client_connection4 *c = (struct client_connection4 *) client;

    printf("Sending message...\n");
        
    int payload_size = MAX_UDP - 4;         // 2 for num_parts, 2 bytes for seqnum
    unsigned long msg_length = strlen(c->message);
    int num_parts = ((int) msg_length / (payload_size)) + 1;       // +1 for last (probably shorter) message in sequence
    int big_msg_index = 0, rc;
    char buf[MAX_UDP];
    
    for (int msg_num = 1; msg_num <= num_parts; msg_num++) {
        bzero(buf, MAX_UDP);
        
        // First two bytes indicate how many parts message is split into
        buf[0] = '0' + (num_parts / 10);
        buf[1] = '0' + (num_parts % 10);
        // Second two bytes indicate which message this is
        buf[2] = '0' + (msg_num / 10);
        buf[3] = '0' + (msg_num % 10);
        
        // i represents place in buffer, big_msg_index represents place in original message
        for (int i = 4; i < MAX_UDP && big_msg_index < msg_length; i++, big_msg_index++) {
            buf[i] = c->message[big_msg_index];
        }
        
        rc = UDP_Write(c->sd, &c->serverAddr, buf, MAX_UDP);
        printf("Sent part %i of the message.\n", msg_num);
        if (rc < 0) {
            perror("Error sending message");
            exit(EXIT_FAILURE);
        }
    }
    printf("Finished sending message\n\n");
    
    /* Wait for response */
    rc = UDP_Read(c->sd, &c->clientAddr, c->response, MAX_UDP);
    pthread_mutex_lock(&c->lock);
    if (rc < 0) {
        perror("Error sending message");
    } else {
        c->done = 1;
    }
    
    /* Let main function know read is complete */
    c->done_or_timeout = 1;
    rc = pthread_cond_signal(&c->condition);
    assert(rc == 0);
    rc = pthread_mutex_unlock(&c->lock);
    assert(rc == 0);
    
    return NULL;
}

void send_message5(struct client_connection4 *c) {
    pthread_t timer, sender;
    
    while (!c->done) {
        c->done_or_timeout = 0;
        pthread_create(&timer, NULL, timer_function4, c);
        pthread_create(&sender, NULL, send_function5, c);
                
        pthread_mutex_lock(&c->lock);
        while (!c->done_or_timeout) {
            pthread_cond_wait(&c->condition, &c->lock);
        }
        pthread_mutex_unlock(&c->lock);
    }
}

void client5(char *message) {
    struct client_connection4 *c = open_client_port4(message);
    send_message5(c);
    printf("\nClient received: %s\n", c->response);
    close(c->sd);
    free(c);
}

void run5() {
    pthread_t thread;
    pthread_create(&thread, NULL, &server5, NULL);
    sleep(1);
    char *message = long_message;
    client5(message);
}





/*
 Question 6
 
 My initial thought was to create a new pthread for every sent message,
 which would allow a second message to be sent while the first is still
 trying to send, but this would violate the in-order requirement.
 
 Instead, each message is sent without waiting for a timeout, and an
 in-flight message count in the client_connection struct is incremented.
 For simplicity, messages are considered indistinguishable from each other,
 so if an ack is missing, it just sends another copy of the same message.
 Obviously, this isn't realistic.
 
 By the way, if anyone besides me is reading this and (god forbid) trying
 to use this as a guide, I'm very sorry for how much of a mess this has
 become. I tend to run out of steam near the end of a textbook, and I had
 no idea this homework was such a monster. My compromise was to finish it,
 but to choose the lazy way whenever possible. Hence the crappy design choices.
 */

struct client_connection6 {
    int sd, in_flight_messages;
    struct sockaddr_in clientAddr, serverAddr;
    char *message;
} client_connection6;

struct client_connection6 *open_client_port6(char *message) {
    /* Open port */
    int sd = UDP_Open(CLIENT_PORT);
    if (sd < 0) {
        perror("Error opening client port");
        exit(EXIT_FAILURE);
    }
    
    /* Configure socket */
    int buffer_size = MAX_UDP;
    int rc = setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    if (rc < 0) {
        perror("Error setting sockopt");
        exit(EXIT_FAILURE);
    }
    
    /* Initialize connection struct */
    struct client_connection6 *c = malloc(sizeof(client_connection6));
    c->sd = sd;
    c->in_flight_messages = 0;
    c->message = message;
    UDP_FillSockAddr(&c->serverAddr, SERVER_PORT);

    return c;
}

void send_message6(struct client_connection6 *c) {
    printf("Sending message...\n");
        
    int payload_size = MAX_UDP - 4;         // 2 for num_parts, 2 bytes for seqnum
    unsigned long msg_length = strlen(c->message);
    int num_parts = ((int) msg_length / (payload_size)) + 1;       // +1 for last (probably shorter) message in sequence
    int big_msg_index = 0, rc;
    char buf[MAX_UDP];
    
    for (int msg_num = 1; msg_num <= num_parts; msg_num++) {
        bzero(buf, MAX_UDP);
        
        // First two bytes indicate how many parts message is split into
        buf[0] = '0' + (num_parts / 10);
        buf[1] = '0' + (num_parts % 10);
        // Second two bytes indicate which message this is
        buf[2] = '0' + (msg_num / 10);
        buf[3] = '0' + (msg_num % 10);
        
        // i represents place in buffer, big_msg_index represents place in original message
        for (int i = 4; i < MAX_UDP && big_msg_index < msg_length; i++, big_msg_index++) {
            buf[i] = c->message[big_msg_index];
        }
        
        rc = UDP_Write(c->sd, &c->serverAddr, buf, MAX_UDP);
        if (rc < 0) {
            perror("Error sending message");
            exit(EXIT_FAILURE);
        }
    }
    c->in_flight_messages++;
    printf("Finished sending message\n\n");
}

void await_acks(struct client_connection6 *c) {
    char *response = malloc(MAX_UDP);
    sleep(TIMEOUT);     // Need to give server a chance to respond

    // Decrement message count for each ack received
    while ((int) recv(c->sd, response, MAX_UDP, MSG_DONTWAIT) > 0) {
        c->in_flight_messages--;
    }
    
    /* If there are unacknowledged messages, resend them */
    int unacked_msgs = c->in_flight_messages;
    printf("Unacked messages: %i\n", unacked_msgs);
    
    for (int i = 0; i < unacked_msgs; i++) {
        send_message6(c);
    }
    /* If there are unacknowledged messages, recursively await them */
    if (unacked_msgs > 0) {
        await_acks(c);
    }
}

void client6(char *message) {
    struct client_connection6 *c = open_client_port6(message);
    send_message6(c);
    send_message6(c);
    send_message6(c);
    await_acks(c);
}

void run6() {
    pthread_t thread;
    pthread_create(&thread, NULL, &server5, NULL);
    sleep(1);
    char *message = long_message;
    client6(message);
}

/*
 Question 7
 
 Each version of this problem starts a server and lets it spin.
 Not great for testing each of them sequentially. So, you can
 only test one at a time, and when main exits everything gets
 cleaned up.
 
 (You didn't think my answer to the last question would be any
 less hacky than the others, did you?)
 
 Latency:
 Time for version 1 to get a message sent and acknowledged is 603 microseconds. (short message)
 Time for version 2 to get a message sent and acknowledged is 447 microseconds. (short message)
 Time for version 3 is ... omitted. Server sleeps before responding, so not a fair comparison.
 Time for version 4 is ... omitted. Server sleeps before responding, so not a fair comparison.
 Time for version 5 to get a message sent and acknowledged is 1103 microseconds. (long message)
 Time for version 6 is ... omitted. This time, it's a sleepy client throwing it off.

 Average:
 Time for version 1 is 756 microseconds per iteration.
 Time for version 2 is 755 microseconds per iteration.
 Time for version 3 is ... omitted.
 Time for version 4 is ... omitted.
 Time for version 5 is 1438 microseconds per iteration.
 Time for version 6 is ... omitted.

 */

void run7_latency(int version) {
    struct timeval start_time;
    struct timeval end_time;
    
    pthread_t thread;

    switch (version) {
        case 1:
            pthread_create(&thread, NULL, &server, NULL);
            sleep(1);
            
            gettimeofday(&start_time, NULL);
            client();
            gettimeofday(&end_time, NULL);

            break;
            
        case 2:
            pthread_create(&thread, NULL, &server2, NULL);
            sleep(1);
            
            gettimeofday(&start_time, NULL);
            client2();
            gettimeofday(&end_time, NULL);
            
            break;
            
        case 3:
            
            
        case 4:
            printf("No fair! The server sleeps instead of replying right away, so measurements are meaningless.\n");
            
        case 5:
            pthread_create(&thread, NULL, &server5, NULL);
            sleep(1);
            
            gettimeofday(&start_time, NULL);
            client5(long_message);
            gettimeofday(&end_time, NULL);
            
            break;
            
        case 6:
            printf("Sorry, that wouldn't be fair. The sixth version takes a nap instead of using a timeout to wait for ACKs, so most of its elapsed time would just be sleep.\n");
            
        default:
            exit(0);
    }
    long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    
    printf("Time for version %i to get a message sent and acknowledged is %li microseconds.\n", version, elapsed_time);
    if (version == 1 || version == 2) {
        printf("(Note: this is for a very short message.)\n");
    } else {
        printf("(Note: this is for a long message.)\n");
    }
    
}

void run7_average(int version) {
    struct timeval start_time;
    struct timeval end_time;
    
    int short_msg_iterations = 500;
    int long_msg_iterations = 100;
    
    pthread_t thread;

    switch (version) {
        case 1:
            pthread_create(&thread, NULL, &server, NULL);
            sleep(1);
            
            gettimeofday(&start_time, NULL);
            for (int i = 0; i < short_msg_iterations; i++) {
                client();
            }
            gettimeofday(&end_time, NULL);

            break;
            
        case 2:
            pthread_create(&thread, NULL, &server2, NULL);
            sleep(1);
            
            gettimeofday(&start_time, NULL);
            for (int i = 0; i < short_msg_iterations; i++) {
                client2();
            }
            gettimeofday(&end_time, NULL);
            
            break;
            
        case 3:
            printf("No fair! The server sleeps instead of replying right away, so measurements are meaningless.\n");
            
            break;
            
        case 4:
            pthread_create(&thread, NULL, &server3, NULL);
            sleep(1);
            
            gettimeofday(&start_time, NULL);
            for (int i = 0; i < short_msg_iterations; i++) {
                client4(long_message);
            }
            gettimeofday(&end_time, NULL);
            break;
            
        case 5:
            pthread_create(&thread, NULL, &server5, NULL);
            sleep(1);
            
            gettimeofday(&start_time, NULL);
            for (int i = 0; i < long_msg_iterations; i++) {
                client5(long_message);
            }
            gettimeofday(&end_time, NULL);
            
            break;
            
        case 6:
            printf("Sorry, that wouldn't be fair. The sixth version takes a nap instead of using a timeout to wait for ACKs, so most of its elapsed time would just be sleep.\n");
            
        default:
            exit(0);
    }
    long elapsed_time = (end_time.tv_sec * MICROSECS_IN_SECONDS + end_time.tv_usec) - (start_time.tv_sec * MICROSECS_IN_SECONDS + start_time.tv_usec);
    long time_per_iteration = elapsed_time / long_msg_iterations;
    
    /*
     Iteration is a bit loose. It really is iteration for the long messages, but it's actually
     five iterations for the short ones to even things out a bit.
     */
    printf("Time for version %i is %li microseconds per iteration.\n", version, time_per_iteration);
}

