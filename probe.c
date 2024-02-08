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

// probe[0] => the ID which started the process
// probe[1] => the ID where the probe is currently present
// probe[2] => the ID where the probe is received next

typedef struct list {
    int probe[3];
} list;

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
    if (bind(sock_id, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    return sock_id;
}

// Sending process to the ID
void send_to_id(int id, int sock_id, list l) {
    struct sockaddr_in client_address;
    memset(&client_address, 0, sizeof(client_address));

    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(id);
    
    sendto(sock_id, &l, sizeof(list), 0, (struct sockaddr *)&client_address, sizeof(client_address));
}

// Sending the probes to each node
void send_probes(int sock_id, int self, int *process, int numEdges, list l) {
    for (int i = 0; i < numEdges; i++) {
        l.probe[2] = process[i];
        send_to_id(process[i], sock_id, l);
        printf("Sending probe to ID: %d\n", process[i]);
    }
}

void print(list l) {
    printf("Recevied probe: ");
    for (int i = 0; i < 3; i++) {
        printf("%d ", l.probe[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int self = atoi(argv[1]); // self ID
    int numEdges = atoi(argv[3]); // other edges

    int process[numEdges]; // the processes to which it is connected
    for (int i = 0; i < numEdges; i++) {
        process[i] = atoi(argv[4 + i]);
    }

    // it starts the probing (deadlock detection) process
    bool start_at = atoi(argv[2]) == 1 ? true : false;

    // Start creating the node
    printf("Creating a node of ID %d and starts: %d\n", self, start_at);
    int sock_id = connect_to_port(self);

    // Implement the structure of the node
    list l;

    if (start_at) { // Initiate the process
        list l1;
        l1.probe[0] = self;
        l1.probe[1] = self;
        send_probes(sock_id, self, process, numEdges, l1);
    }
    
    struct sockaddr_in from;
    int len; // very important

    while (true) {
        memset(&from, 0, sizeof(from)); // initialise the struct
        recvfrom(sock_id, &l, sizeof(list), MSG_WAITALL, (struct sockaddr *)&from, &len); // receive the probe
        print(l);
        if (l.probe[0] == -1) { // source node is lost
            printf("Deadlock has happended\n");
            send_probes(sock_id, self, process, numEdges, l);
            break;
        }
        if (l.probe[0] == self) { // cycle is detected
            printf("Deadlock has happended\n");
            l.probe[0] = -1;
            send_probes(sock_id, self, process, numEdges, l);
            break;
        }
        else { // the source node starts sending the probe
            l.probe[1] = self;
            send_probes(sock_id, self, process, numEdges, l);
        }
    }    

    return 0;
}
