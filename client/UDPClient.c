#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define MAXLINE 256        // max data amount
#define MSG_HEADER_LEN 4   // max header amount
#define MSG_TYPE_JOIN 1    // 1 for join
#define MSG_TYPE_CHAT 2    // 2 for chat
#define MSG_TYPE_LEAVE 3   // 3 for leave

// Globals for SIGINT handler
int sock_global;
struct sockaddr_in servAddr_global;
char nickName_global[MAXLINE];

// Error handler
void DieWithError(const char *errorMessage);

// Build formatted message
void build_message(uint16_t type, const char *data, char *buffer, int *msg_len) {
    uint16_t len = strlen(data);
    uint16_t type_net = htons(type);
    uint16_t len_net = htons(len);
    memcpy(buffer, &type_net, 2);
    memcpy(buffer + 2, &len_net, 2);
    memcpy(buffer + 4, data, len);
    *msg_len = 4 + len;
}

// Handle Ctrl+C
// when a client exits, send a leave msg to the server
void handle_sigint(int sig) {
    char buffer[512];
    int msg_len;

    build_message(MSG_TYPE_LEAVE, nickName_global, buffer, &msg_len);
    sendto(sock_global, buffer, msg_len, 0,
           (struct sockaddr *)&servAddr_global, sizeof(servAddr_global));

    // Print timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    printf("\n[%s] Sent LEAVE to server (Ctrl+C)\n", time_str); // sends leave msg to server

    close(sock_global);
    exit(0);
}

// Receive messages
// thread will listen to msgs from the server
void *recv_msgs(void *arg) {
    int sock = *((int *)arg);
    char buffer[512];
    struct sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);

    while (1) {
        int recvLen = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr *)&fromAddr, &fromLen);
        if (recvLen >= MSG_HEADER_LEN) {
            uint16_t type, len;
            memcpy(&type, buffer, 2);
            memcpy(&len, buffer + 2, 2);
            type = ntohs(type);
            len = ntohs(len);
            buffer[4 + len] = '\0';

            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char time_str[9];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

            printf("[%s] Message received from server\n", time_str);
            printf("[%s] %s\n", time_str, buffer + 4);
        }
    }
    return NULL;
}

int main() {
    // socket descripter, server address, port # and other general information
    int sock;
    struct sockaddr_in servAddr;
    unsigned short servPort = 12345;
    char *servIP = "127.0.0.1";
    char nickName[MAXLINE];
    char buffer[512];

    // Prompt for nickname
    printf("Enter nickname: ");
    fgets(nickName, MAXLINE, stdin);
    nickName[strcspn(nickName, "\n")] = '\0';

    // Create UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // Set server address
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port = htons(servPort);

    // Set global vars for SIGINT
    sock_global = sock;
    servAddr_global = servAddr;
    strncpy(nickName_global, nickName, MAXLINE);

    // Catch Ctrl+C
    signal(SIGINT, handle_sigint);

    // Send JOIN
    int msg_len;
    build_message(MSG_TYPE_JOIN, nickName, buffer, &msg_len);
    if (sendto(sock, buffer, msg_len, 0,
               (struct sockaddr *)&servAddr, sizeof(servAddr)) != msg_len)
        DieWithError("sendto() JOIN failed");

    // Start recv thread
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, recv_msgs, (void *)&sock) != 0)
        DieWithError("pthread_create() failed");

    // Send loop
    while (1) {
        char input[MAXLINE];
        fgets(input, MAXLINE, stdin);
        input[strcspn(input, "\n")] = '\0';

        char fullMsg[MAXLINE + 32];
        snprintf(fullMsg, sizeof(fullMsg), "[%s]: %s", nickName, input); // send and print out msg to the server

        build_message(MSG_TYPE_CHAT, fullMsg, buffer, &msg_len);
        if (sendto(sock, buffer, msg_len, 0,
                   (struct sockaddr *)&servAddr, sizeof(servAddr)) != msg_len)
            DieWithError("sendto() CHAT failed");

        // Print own message with timestamp
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_str[9];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
        printf("[%s] [You]: %s\n", time_str, input);
    }

    close(sock);
    return 0;
}
