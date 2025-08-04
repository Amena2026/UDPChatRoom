#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100
#define MAX_MSG_SIZE 512
#define PORT 12345

typedef struct {
    struct sockaddr_in addr;
    int active;
} Client;

Client clients[MAX_CLIENTS]; // max # of clients that can connect to the server at the same time is 100

int same_client(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_addr.s_addr == b->sin_addr.s_addr) &&
           (a->sin_port == b->sin_port);
}

// while max clients is not full and a client wishes to connect to server, let them join
void add_client(struct sockaddr_in *clientAddr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && same_client(&clients[i].addr, clientAddr))
            return;
        if (!clients[i].active) {
            clients[i].addr = *clientAddr;
            clients[i].active = 1;
            return;
        }
    }
}

// send messages from our server to any active client
void broadcast(char *msg, int msgLen, struct sockaddr_in *sender, int sock) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && !same_client(&clients[i].addr, sender)) {
            sendto(sock, msg, msgLen, 0,
                   (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
        }
    }
}

int main() {
    // socket descriptor, server addr for client & server, buffer
    int sock;
    struct sockaddr_in servAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    char buffer[MAX_MSG_SIZE];

    // create socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // set addr and port #
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    // bind socket
    if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    while (1) {
        // recv msg from client
        int recvLen = recvfrom(sock, buffer, MAX_MSG_SIZE - 1, 0,
                               (struct sockaddr *)&clientAddr, &addrLen);
        if (recvLen >= 4) {
            uint16_t type, len;
            memcpy(&type, buffer, 2);
            memcpy(&len, buffer + 2, 2);
            type = ntohs(type);
            len = ntohs(len);
            buffer[4 + len] = '\0';

            add_client(&clientAddr);

            // print out the appropriate msg based on the type of msg sent
            if (type == 1)
                printf("Received JOIN from %s:%d - %s\n",
                       inet_ntoa(clientAddr.sin_addr),
                       ntohs(clientAddr.sin_port),
                       buffer + 4);
            else if (type == 2)
                printf("Received CHAT from %s\n", buffer + 4);
            else if (type == 3)
                printf("Received LEAVE from %s\n", buffer + 4);

            broadcast(buffer, recvLen, &clientAddr, sock);
        }
    }

    close(sock);
    return 0;
}
