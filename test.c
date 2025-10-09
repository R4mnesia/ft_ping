// ft_ping_min.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip.h>       // struct iphdr
#include <netinet/ip_icmp.h>  // struct icmphdr
#include <signal.h>

#define PACKET_SIZE 64

static unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1) {
        sum += *(unsigned char*)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ipv4>\n", argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Set receive timeout so we don't block forever
    struct timeval tv;
    tv.tv_sec = 1; // 1 s timeout
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &dst.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    // Build ICMP echo request
    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    struct icmphdr *icmp = (struct icmphdr *)packet;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid() & 0xFFFF);
    static unsigned short seq = 0;
    icmp->un.echo.sequence = htons(++seq);

    // Put timestamp into payload for RTT measurement
    struct timeval *tv_payload = (struct timeval *)(packet + sizeof(struct icmphdr));
    if (gettimeofday(tv_payload, NULL) < 0) {
        perror("gettimeofday");
    }

    int icmp_len = sizeof(struct icmphdr) + sizeof(struct timeval);
    icmp->checksum = 0;
    icmp->checksum = checksum((unsigned short *)icmp, icmp_len);

    ssize_t sent = sendto(sockfd, packet, icmp_len, 0,
                          (struct sockaddr*)&dst, sizeof(dst));
    if (sent <= 0) {
        perror("sendto");
        close(sockfd);
        return 1;
    }
    printf("Sent %zd bytes to %s\n", sent, argv[1]);

    // Receive
    char buf[1500];
    struct sockaddr_in src;
    socklen_t src_len = sizeof(src);
    ssize_t recvd = recvfrom(sockfd, buf, sizeof(buf), 0,
                             (struct sockaddr*)&src, &src_len);
    if (recvd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            printf("Request timed out.\n");
        else
            perror("recvfrom");
        close(sockfd);
        return 1;
    }

    // Parse IP header (first)
    struct iphdr *ip = (struct iphdr *)buf;
    int ip_header_len = ip->ihl * 4;
    if (recvd < ip_header_len + (int)sizeof(struct icmphdr)) {
        fprintf(stderr, "Packet too short (%zd bytes)\n", recvd);
        close(sockfd);
        return 1;
    }

    struct icmphdr *r_icmp = (struct icmphdr *)(buf + ip_header_len);

    // Check ICMP type
    if (r_icmp->type == ICMP_ECHOREPLY) {
        // Note: we used htons for id & seq when building; convert back when comparing
        unsigned short r_id = ntohs(r_icmp->un.echo.id);
        unsigned short r_seq = ntohs(r_icmp->un.echo.sequence);
        if (r_id == (getpid() & 0xFFFF)) {
            // Retrieve timestamp from payload
            struct timeval *sent_tv = (struct timeval *)((char*)r_icmp + sizeof(struct icmphdr));
            struct timeval now, diff;
            gettimeofday(&now, NULL);
            timersub(&now, sent_tv, &diff);
            double rtt_ms = diff.tv_sec * 1000.0 + diff.tv_usec / 1000.0;
            char src_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &src.sin_addr, src_ip, sizeof(src_ip));
            printf("%zd bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
                   recvd - ip_header_len,
                   src_ip,
                   r_seq,
                   ip->ttl,
                   rtt_ms);
        } else {
            printf("Received ICMP_ECHOREPLY but id mismatch (got %u, expected %u)\n",
                   ntohs(r_icmp->un.echo.id), (getpid() & 0xFFFF));
        }
    } else {
        printf("Received ICMP type %d code %d\n", r_icmp->type, r_icmp->code);
    }

    close(sockfd);
    return 0;
}

