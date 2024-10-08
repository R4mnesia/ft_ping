#include "../include/ping.h"
#include <stdio.h>

char    *resolve_hostname(const char *hostname, struct sockaddr_in *addr_dest) {
    
    struct hostent *host_entity;
    char *ipstr = NULL;
    
    ipstr = malloc(NI_MAXHOST * sizeof(char));

    if ((host_entity = gethostbyname(hostname)) == NULL) {
        printf("IP not found\n");
        free(ipstr);
        return (NULL);
    }

    // inet_ntoa (convert binary IP on strings format)
    strcpy(ipstr, inet_ntoa(*(struct in_addr *)host_entity->h_addr));
    (*addr_dest).sin_family = host_entity->h_addrtype; // AF_INET (ipv4)
    (*addr_dest).sin_port = 0;
    (*addr_dest).sin_addr.s_addr = *(long *)host_entity->h_addr;
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
        dest->hostname = resolve_hostname(hostname, &dest->addr);
        if (!dest->hostname)
            printf("Error resolve hostname\n");
        return (1);
    }

    return (0);
}


int main(int argc, char **argv) {

    if (argc > 3 || argc < 2) {
        printf("Error arguments\n");
    }

    t_ping  dest;
    if (!ParseArg(argc, argv, &dest)) {
        printf("Error arguments");
        freeDest(dest);
        return (1);
    }

    freeDest(dest);
    return (0);
}