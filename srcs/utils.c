#include "ft_ping.h"

int sig = 0;
void freeDest(t_ping *dest)
{
    free(dest->hostname);
}

float calc_min(t_time time)
{

    int i = 0;
    float t = time.time[i];
    while (i < time.seq) {
        if (time.time[i] < t)
            t = time.time[i];
        i++;
    }
    return (t);
}

float calc_max(t_time time)
{

    int     i = 0;
    float   t = time.time[i];

    while (i < time.seq)
    {
        if (time.time[i] > t)
            t = time.time[i];
        i++;
    }
    return (t);
}

float calc_avg(t_time time)
{

    float   sum = 0, avg = 0;
    int     i = 0;

    while (i < time.seq)
    {
        sum += time.time[i];
        i++;
    }

    avg = sum / time.seq;
    return (avg);
}

float calc_mdev(t_time time)
{
    float avg = calc_avg(time);
    float sum = 0;
    int     i = 0;

    while (i < time.seq)
    {
        float diff = time.time[i] - avg;
        sum += diff * diff;
        i++;
    }

    return sqrtf(sum / time.seq);
}

float calc_ewma(t_time time, float alpha)
{
    if (time.seq == 0)
        return 0.0f;

    float ewma = time.time[0];
    for (int i = 1; i < time.seq; i++)
    {
        ewma = alpha * time.time[i] + (1.0f - alpha) * ewma;
    }
    return ewma;
}

int check_signal(char *arg, t_time time)
{

    if (sig == CTRLC) 
    {
        float min = 0, max = 0, avg = 0, mdev = 0;

        min = calc_min(time);
        max = calc_max(time);
        avg = calc_avg(time);
        mdev = calc_mdev(time);

        printf("\n--- %s ping statistics ---\n", arg);
        printf("%d packets transmitted, %d received, 0%% packet loss, time %dms\n", time.seq, time.seq, time.all_time);
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", min, avg, max, mdev);
        exit(0);
    }
    return (0);
}

void Error_exit(int n, int sock, char *host)
{

    switch (n) {
        case 1:
            fprintf(stderr, "Error: FD\n");
            free(host);
            exit(0);
        case 2:
            fprintf(stderr, "Error: send message\n");
            close(sock);
            free(host);
            exit(0);
        case 3:
            fprintf(stderr, "Error: Socket TTL\n");
            close(sock);
            free(host);
            exit(0);
        case 4:
            fprintf(stderr, "Error: receive message\n");
            close(sock);
            free(host);
            exit(0);
        case 5: 
            fprintf(stderr, "Error: Packet no echo reply\n");
            close(sock);
            free(host);
            exit(0);
    }
}