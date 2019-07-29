#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_ADDR "192.168.1.5"

int main() {
    int sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    struct in_addr addr;
    char buff[255];
    int bytes_read;
    int client_addr_len = sizeof(struct sockaddr_in);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);     
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (inet_aton(SERVER_ADDR, &addr) == 0) {
        perror("inet aton");
        exit(1);
    }
    server_addr.sin_addr.s_addr = addr.s_addr; 
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(2);
    }

    while (1) {
        bytes_read = recvfrom(sock, buff, 255, 0, 
                (struct sockaddr *)&client_addr, &client_addr_len);

        if (bytes_read < 0) {
            perror("recvfrom");
            exit(3);
        } else if (bytes_read == 0) {
            printf("Got zero bytes!\n");
        }

        printf("Message received: %s", buff);
        buff[1] = 'F';

        if (sendto(sock, buff, bytes_read, 0,
                (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
            perror("sendto");
            exit(3);
        }
        
        printf("Sent message: %s", buff);

        memset(buff, 0, sizeof(buff));
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr_len = sizeof(struct sockaddr_in);
    }

    close(sock);

    return 0;
}