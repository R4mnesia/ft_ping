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
    printf("hostname = %s\n", hostname);
    if (hostname) {
        dest->hostname = resolve_hostname(hostname, (struct sockaddr_in *)&dest->addr);
        dest->sock = 0;
        if (!dest->hostname)
            printf("Error resolve hostname\n");
        return (1);
    }

    return (0);
}

void    sendPing(t_ping *dest) {

    //unsigned int seq = 0;
    t_header header;
    socklen_t socklen;

    header.icmp.type = ICMP_ECHO; // 8
    header.icmp.code = 0;
    header.icmp.checksum = 0; // function for check
    header.icmp.un.echo.sequence = 0; // number of request
    header.icmp.un.echo.id = 2; // getpid() ??

    dest->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_ICMP); // IPPROTO_ICMP (specifie au noyau d'utiliser ICMP)
    if (dest->sock == -1) {
        printf("Error: FD\n");
        return ;
    }
    printf("sock = %d\n", dest->sock);
    if (sendto(dest->sock, &header, sizeof(header), 0, (struct sockaddr *)&dest->addr, sizeof(dest->addr)) == -1) {
        printf("Error: send message\n");
        return ;
    }

    socklen = sizeof(dest->addr);
    if (recvfrom(dest->sock, &header, sizeof(header), 0, (struct sockaddr *)&dest->addr, &socklen) == -1) {
        printf("Error: receive message\n");
        return ;
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
    sendPing(&dest);
    freeDest(dest);
    return (0);
}