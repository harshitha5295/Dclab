#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>

#define ML 1024
#define MPROC 32

/*
	Function to create a new connection to port `connect_to`
 	1. Creates the socket
  	2. Binds to port
   	3. Returns socket id
*/

int connect_to_port(int connect_to) {
    int sock_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_id < 0) {
        perror("Unable to create a socket");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(connect_to);

    // Bind the socket to a specific address and port
    if (bind(sock_id, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    return sock_id;
}

/*
    sends a message to port id to
*/

void send_to_id(int id , int sock_id, char message[ML]) {
    struct sockaddr_in client_address;
    memset(&client_address, 0, sizeof(client_address));

    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(id);

    sendto(sock_id, (const char *) message, strlen(message), 0, (const struct sockaddr *)&client_address, sizeof(client_address));
}

/*
    starts the election, returns 1 if it wins the round
*/

int election(int id, int *process, int numProc, int self) {
    char message[ML];
    strcpy(message, "ELECTION");
    int is_new_coord = 1; // assume you are the winner until you lose;
    // coord -> The idea behind the Bully Algorithm is to elect the highest-numbered processor as the coordinator.
    for (int i = 0; i < numProc; i++) {
        if (process[i] > self) {
            send_to_id(process[i], id, message);
            printf("Sending election to: %d\n", process[i]);
            is_new_coord = 0; // a proc with id > self exists thus cannot be coord
        }
    }
    return is_new_coord;
}

/*
    announces completion by sending coord messages
*/

void announce_completion (int id, int *process, int numProc, int self) {
    char message[ML];
    strcpy(message, "COORDINATOR");

    for (int i = 0; i < numProc; i++) {
        if (process[i] != self) {
            send_to_id(process[i], id, message);
        }
    }
}

int main(int argc, char *argv[]) {
    // 0. Initialize variables
    int self = atoi(argv[1]), numProc = atoi(argv[2]);
    int process[MPROC];

    for (int i = 0; i < numProc; i++) {
        process[i] = atoi(argv[3 + i]);
    }

    bool start_at = atoi(argv[3 + numProc]) == 1 ? true : false;

    // 1. Create socket
    printf("Creating a node at %d %d\n", self, start_at);
    int sock_id = connect_to_port(self);
    // getchar();

    // 2. check if process is initiator
    if (start_at) {
        election(sock_id, process, numProc, self);
    }

    struct sockaddr_in from;
    int len, bully_id;
    char buff[ML], message[ML];
    
    // 3. if not the initiator, wait for some time
    while (true) {
        memset(&from, 0, sizeof(from));
        int n = recvfrom(sock_id, (char *)buff, ML, MSG_WAITALL, (struct sockaddr *)&from, &len);
        buff[n] = '\0';
        printf("Received message: %s\n", buff);

        if (!strcmp(buff, "ELECTION")) {
            strcpy(message, "E-ACK"); // send election ack
            printf("%s\n", message);
            
            sendto(sock_id, (const char *) message, strlen(message), 0, (const struct sockaddr *) &from, sizeof(from));

            if (election(sock_id, process, numProc, self)) {
                announce_completion(sock_id, process, numProc, self);
                printf("ANNOUNCING SELF AS NEW COORD\n");
            }
        }
        else if (!strcmp(buff, "E-ACK")) {
            printf("%s\n", buff);
            continue; // do nothing, the job is done
        }
        else if (!strcmp(buff, "COORDINATOR")) {
            bully_id = from.sin_port;
        }
    }
	return 0;
}
