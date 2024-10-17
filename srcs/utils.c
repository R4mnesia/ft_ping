#include "../include/ping.h"

int sig = 0;
void freeDest(t_ping *dest) {

    free(dest->hostname);
}

// mettre static qui passe 0 ou 1 pour print, pas avec le signal
void CTRLC(char *arg, int seq, float ms) {
    if (sig == 1) {
        printf("\n--- %s ping statistics ---\n", arg);
        printf("%d packets transmitted, %d received, 0%% packet loss, time %fms\n", seq, seq, ms);
        exit(0);
    }
}

int ParseArg(int argc, char **argv, t_ping *dest) {
    
    if (argc < 2) {
        return (0);
    }
    char *hostname = NULL;
    dest->verbose = 0;
    dest->hostname = NULL;
    dest->sock = -1;
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
        resolve_hostname(dest, hostname, (struct sockaddr_in *)&dest->addr);
        dest->sock = 0;
        printf("aa %s\n", dest->hostname);
        return (1);
    }

    return (0);
}

void ERROR(int n, int sock, char *host) { // penser a free et close le fd

    switch (n) {
        case 1:
            dprintf(2, "Error: FD\n");
            free(host);
            exit(0);
        case 2:
            dprintf(2, "Error: send message\n");
            close(sock);
            free(host);
            exit(0);
        case 3:
            dprintf(2, "Error: Socket TTL\n");
            close(sock);
            free(host);
            exit(0);
        case 4:
            dprintf(2, "Error: receive message\n");
            close(sock);
            free(host);
            exit(0);
        case 5: 
            dprintf(2, "Error: Packet no echo reply\n");
            close(sock);
            free(host);
            exit(0);
    }
}