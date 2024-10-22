#include "../include/ping.h"

int sig = 0;
void freeDest(t_ping *dest) {

    free(dest->hostname);
}

//float calc_min

// mettre static qui passe 0 ou 1 pour print, pas avec le signal
void Quit_ProgramSIGINT(char *arg, int seq, float ms, int packet_sent, int packet_received, t_time *time) {
    if (sig == CTRLC) {
        printf("\n--- %s ping statistics ---\n", arg);
        printf("%d packets transmitted, %d received, 0%% packet loss, time %fms\n", seq, seq, ms);
        exit(0);
    }
    else if (sig == CTRLQUIT) {
        // 3/3 packets, 0% loss, min/avg/ewma/max = 17.580/17.656/17.674/17.704 ms
        float packet_loss = (float)((packet_sent - packet_received) / packet_sent) * 100;
        //float min = calc_min(time);
        //float max = calc_max(time)
        printf("%d/%d packets, %f loss, min,avg,ewma,max =  %.3fms\n", packet_received, packet_sent, packet_loss, time[0].time);

        usleep(SL);
        sig = 0;
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