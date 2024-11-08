#include "../include/ping.h"

int sig = 0;
void freeDest(t_ping *dest) {

    free(dest->hostname);
}

float calc_min(t_time time) {

    int i = 0;
    float t = time.time[i];
    while (i < time.seq) {
        if (time.time[i] < t)
            t = time.time[i];
        i++;
    }
    return (t);
}

float calc_max(t_time time) {

    int i = 0;
    float t = time.time[i];
    while (i < time.seq) {
        if (time.time[i] > t)
            t = time.time[i];
        i++;
    }
    return (t);
}

float calc_avg(t_time time) {

    float sum = 0, avg = 0;
    int i = 0;
    while (i < time.seq) {
        sum += time.time[i];
        i++;
    }
    avg = sum / time.seq;
    return (avg);
}

/*
Calculer la moyenne des temps de réponse.
Soustraire la moyenne de chaque temps de réponse pour obtenir les écarts.
Élever chaque écart au carré.
Faire la somme de ces carrés.
Diviser par le nombre de réponses pour obtenir la variance.
Prendre la racine carrée de la variance pour obtenir l'écart type (mdev)
*/

//float calc_mdev(t_time *time, int sequence, float avg) {
//
//    float tab[sequence] = {0};
//    int i = 0;
//    while (i < sequence) {
//        tab[i] = time[i].time - avg;
//        i++;
//    }
//
//}

// mettre static qui passe 0 ou 1 pour print, pas avec le signal
int check_signal(char *arg, t_time time) {

    float packet_loss = 0, min = 0, max = 0, avg = 0;

    if (sig == CTRLC) 
    {
        packet_loss = (float)((time.packet_sent - time.packet_received) / time.packet_sent) * 100;

        min = calc_min(time);
        max = calc_max(time);
        avg = calc_avg(time);
        printf("\n--- %s ping statistics ---\n", arg);
        printf("%d packets transmitted, %d received, 0%% packet loss, time %dms\n", time.seq, time.seq, time.all_time);
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/0 ms", min, avg, max);
        exit(0);
    }
    else if (sig == CTRLQUIT) 
    {
        packet_loss = (float)((time.packet_sent - time.packet_received) / time.packet_sent) * 100;
    
        min = calc_min(time);
        max = calc_max(time);
        avg = calc_avg(time);
        packet_loss = (float)((time.packet_sent - time.packet_received) / time.packet_sent) * 100;
        printf("%d/%d packets, %f loss, min,avg,ewma,max =  %.3f/%.3f/%.3f/%.3fms\n", time.packet_received, time.packet_sent, packet_loss, min, avg, max, time.time[0]);
        //usleep(PING_SLEEP);
        sig = 0;
        return (1);
    }
    return (0);
}

int ParseArg(int argc, char **argv, t_ping *dest) {
    
    if (argc < 2) {
        return (0);
    }
    char *hostname = NULL;
    dest->verbose = false;
    dest->hostname = NULL;
    dest->sock = -1;
    if (argc == 2)
        hostname = argv[1];
    else if (argc == 3 && strcmp(argv[1], "-v") == 0) {
        dest->verbose = true;
        if (argc == 3) {
            hostname = argv[2];
        }
    } 
    else if (argc == 3 && strcmp(argv[2], "-v") == 0) {
        dest->verbose = true;
        hostname = argv[1];
    }
    if (hostname) {
        resolve_hostname(dest, hostname, (struct sockaddr_in *)&dest->addr);
        dest->sock = 0;
        return (1);
    }

    return (0);
}

void Error_exit(int n, int sock, char *host) { // penser a free et close le fd

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