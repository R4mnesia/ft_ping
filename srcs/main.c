#include "../include/ping.h"
#include <stdio.h>

char    *resolve_hostname(const char *hostname, struct sockaddr_in *addr_dest) {
    
    struct hostent *host;
    char *ipstr = NULL;
    
    ipstr = malloc(NI_MAXHOST * sizeof(char));

    host = gethostbyname(hostname);
    if (host == NULL) {
        free(ipstr);
        return (NULL);
    }

    // inet_ntoa (convert binary IP on strings format)
    strcpy(ipstr, inet_ntoa(*(struct in_addr *)host->h_addr));
    (*addr_dest).sin_family = host->h_addrtype; // AF_INET (ipv4)
    (*addr_dest).sin_port = 0;
    (*addr_dest).sin_addr.s_addr = *(long *)host->h_addr;
    return (ipstr);
}

int ParseArg(int argc, char **argv, t_ping *dest) {
    
    if (argc < 2) {
        return (0);
    }
    char *hostname = NULL;
    dest->verbose = 0;

    if (argc == 2)
        hostname = argv[1];
    else if (argc == 3 && strcmp(argv[1], "-v") == 0) {
        dest->verbose = 1;
        if (argc == 3) {
            hostname = argv[2];
        }
    } 
    else if (argc == 3 && strcmp(argv[2], "-v") == 0) {
        dest->verbose = 1;
        hostname = argv[1];
    }
    if (hostname) {
        dest->hostname = resolve_hostname(hostname, (struct sockaddr_in *)&dest->addr);
        dest->sock = 0;
        if (!dest->hostname)
            printf("Error resolve hostname\n");
        return (1);
    }

    return (0);
}

void    sendPing(t_ping *dest, char *arg) {

    unsigned int seq = 0;
    unsigned int ttl = 64;
    char    buffer[128] = {0};
    struct sockaddr_in      d_addr;
    t_header header;
    socklen_t socklen;

    memset(header.rest, 0, (64 - sizeof(struct icmphdr)));
    header.icmp.type = ICMP_ECHO; // 8
    header.icmp.code = 0;
    header.icmp.checksum = 0; // function for check
    header.icmp.un.echo.sequence = 0; // number of request
    header.icmp.un.echo.id = 2; // getpid() ??

    dest->sock = socket(dest->addr.sin_family, SOCK_DGRAM, IPPROTO_ICMP); // IPPROTO_ICMP (specifie au noyau d'utiliser ICMP)
    if (dest->sock == -1) {
        printf("Error: FD\n");
        return ;
    }
    if (setsockopt(dest->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) { // set socket for ttl
        printf("Error: Socket TTL\n");
        return ;
    }
    while (1) {
        if (sendto(dest->sock, &header, sizeof(header), 0, (struct sockaddr *)&dest->addr, sizeof(dest->addr)) == -1) {
            printf("Error: send message\n");
            return ;
        }
        usleep(SL);
        socklen = sizeof(d_addr);
        if (recvfrom(dest->sock, &buffer, sizeof(buffer), 0, (struct sockaddr *)&d_addr, &socklen) == -1) {
            printf("Error: receive message\n");
            return ;
        }
        printf("%ld bytes from %s (%s): icmp_seq:%d ttl=%d time=time\n", sizeof(header), arg, dest->hostname, seq, ttl);
        seq++;
    }
    /* 64 bytes from par10s39-in-f14.1e100.net (216.58.214.78): icmp_seq=1 ttl=117 time=28.7 ms  */
    /* size packet, hostname, IP, count_seq,  */
    close(dest->sock);
    printf("OK");

}


int main(int argc, char **argv) {

    if (argc > 3 || argc < 2) {
        printf("Error arguments\n");
        return (1);
    }

    t_ping  dest;
    if (!ParseArg(argc, argv, &dest)) {
        printf("Error arguments");
        freeDest(dest);
        return (1);
    }
    sendPing(&dest, argv[1]);
    freeDest(dest);
    return (0);
}