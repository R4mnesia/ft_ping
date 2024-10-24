#include "../include/ping.h"

int sig = 0;
void freeDest(t_ping *dest) {

    free(dest->hostname);
}

float calc_min(t_time *time, int sequence) {

    int i = 0;
    float t = time[i].time;
    while (i < sequence) {
        if (time[i].time < t)
            t = time[i].time;
        i++;
    }
    return (t);
}

float calc_max(t_time *time, int sequence) {

    int i = 0;
    float t = time[i].time;
    while (i < sequence) {
        if (time[i].time > t)
            t = time[i].time;
        i++;
    }
    return (t);
}

float calc_avg(t_time *time, int sequence) {

    float sum = 0, avg = 0;
    int i = 0;
    while (i < sequence) {
        sum += time[i].time;
        i++;
    }
    avg = sum / sequence;
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
void Quit_ProgramSIGINT(char *arg, int seq, float ms, int packet_sent, int packet_received, t_time *time, int time_sigint) {
    float packet_loss = 0, min = 0, max = 0, avg = 0;
    (void)ms;
    if (sig == CTRLC) {
        packet_loss = (float)((packet_sent - packet_received) / packet_sent) * 100;
        min = calc_min(time, seq);
        max = calc_max(time, seq);
        avg = calc_avg(time, seq);
        printf("\n--- %s ping statistics ---\n", arg);
        printf("%d packets transmitted, %d received, 0%% packet loss, time %dms\n", seq, seq, time_sigint);
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/0 ms", min, avg, max);
        exit(0);
    }
    else if (sig == CTRLQUIT) {
        // 3/3 packets, 0% loss, min/avg/ewma/max = 17.580/17.656/17.674/17.704 ms
        packet_loss = (float)((packet_sent - packet_received) / packet_sent) * 100;
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