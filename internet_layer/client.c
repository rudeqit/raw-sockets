#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/udp.h>
#include <linux/ip.h>

#define SERVER_ADDR "192.168.1.5"
#define CLIENT_ADDR "172.17.0.2"
#define DATAGRAM_LEN 255

char msg[] = "Hello there!\n";

unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for(sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    struct in_addr addr;
    char recv_buff[DATAGRAM_LEN];
    int bytes_read;
    char str_ip[INET_ADDRSTRLEN];
    int server_addr_len = sizeof(struct sockaddr_in);
    char datagram[DATAGRAM_LEN];

    memset(recv_buff, 0, DATAGRAM_LEN);
    memset(datagram, 0, DATAGRAM_LEN);
    
    struct iphdr *ip_header = (struct iphdr *)datagram;
    struct udphdr *udp_header = (struct udphdr *)(datagram + sizeof(*ip_header));
    memcpy(datagram + sizeof(*ip_header) + sizeof(*udp_header), msg, sizeof(msg));

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    int val = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    if (inet_aton(SERVER_ADDR, &addr) == 0) {
        perror("inet aton");
        exit(1);
    }
    server_addr.sin_addr.s_addr = addr.s_addr;

    // ip_header->saddr = server_addr.sin_addr.s_addr;
    ip_header->saddr = inet_addr(CLIENT_ADDR);
    ip_header->daddr = server_addr.sin_addr.s_addr;
    ip_header->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(msg);
    ip_header->protocol = IPPROTO_UDP;
    ip_header->ttl = 64;
    ip_header->ihl = 0x5;
    ip_header->version = 0x4;
    ip_header->id = htons(15555);
    ip_header->tos = 0x0;
    ip_header->frag_off = 0x0;
    ip_header->check = 0x0;
    // ip_header->check = csum((unsigned short *)datagram, sizeof(struct iphdr) + sizeof(struct udphdr));

    udp_header->source = htons(8081);
    udp_header->dest = server_addr.sin_port;
    udp_header->len = htons(sizeof(struct udphdr) + sizeof(msg));    
    // ignore checksum

    bytes_read = sendto(sock, datagram, (sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(msg)), 0,
        (struct sockaddr *)&server_addr, server_addr_len);
    if (bytes_read < 0) {
        perror("sendto");
        exit(3);
    } else if (bytes_read == 0) {
        printf("Sendto zero bytes!\n");
    }
    printf("Sent message: %s", datagram + sizeof(*ip_header) + sizeof(*udp_header));

    while (1) {
        bytes_read = recvfrom(sock, recv_buff, DATAGRAM_LEN, 0, NULL, NULL);
        if (bytes_read < 0) {
            perror("recvfrom");
            continue;
        } else if (bytes_read == 0) {
            printf("Recvfrom got zero bytes!\n");
            continue;
        }

        struct iphdr *ip_header_recv = (struct iphdr *)recv_buff;
        struct udphdr *udp_header_recv = (struct udphdr *)(recv_buff + sizeof(struct iphdr));

        inet_ntop(AF_INET, &ip_header_recv->saddr, str_ip, INET_ADDRSTRLEN);

        if ((strcmp(str_ip, SERVER_ADDR) == 0) && (ntohs(udp_header_recv->dest) == 8081)) {
            printf("Message received: %s", (char *)(recv_buff + sizeof(struct iphdr) + sizeof(struct udphdr)));
            break;
        }

        memset(recv_buff, 0, DATAGRAM_LEN);
    }

    return 0;
}
