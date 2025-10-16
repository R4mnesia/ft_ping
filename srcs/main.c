#include "../include/ping.h"

// Free hostname on sigint handler
static t_ping *ptr_dest = NULL;

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.00f + (t1.tv_usec - t0.tv_usec) / 1000.00f;
}


void sigint_handler(int signal)
{
    if (signal == SIGINT)
    {
        sig = CTRLC;
        if (ptr_dest && ptr_dest->hostname)
            free(ptr_dest->hostname);
    }
}

int main(int argc, char **argv)
{
    if (geteuid() != 0)
    {
        fprintf(stderr, "ft_ping: must be run as root\n");
        return (1);
    }
    if (argc == 1)
    {
        fprintf(stderr, "ft_ping: usage error: Destination address required\n");
        return (1);
    }
    struct sigaction act;
    bzero(&act, sizeof(act));

    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

    t_ping  dest;
    ptr_dest = &dest;

    if (!ParseArg(argc, argv, &dest))
    {
        printf("ft_ping: %s: Name or service not known\n", argv[argc - 1]);
        freeDest(&dest);
        return (1);
    }
    if (!dest.hostname) {
        fprintf(stderr, "ft_ping: %s: Name or service not known\n", argv[1]);
    }
    else
        sendPing(&dest, argv[1]);
    
    return (0);
}