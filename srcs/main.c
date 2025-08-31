#include "../include/ping.h"

// Free hostname on sigint handler
static t_ping *ptr_dest = NULL;

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.00f + (t1.tv_usec - t0.tv_usec) / 1000.00f;
}


void sigint_handler(int signal)
{
    if (signal == SIGINT) {
        sig = CTRLC;
    }
    else if (signal == SIGQUIT) {
        sig = CTRLQUIT;
    }
    if (ptr_dest && ptr_dest->hostname)
        free(ptr_dest->hostname);
}

int main(int argc, char **argv)
{

    if (argc > 3 || argc < 2)
    {
        printf("Error arguments\n");
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
        printf("Error arguments\n");
        freeDest(&dest);
        return (1);
    }
    
    if (!dest.hostname) {
        printf("ft_ping: %s: Name or service not known\n", argv[1]);
    }
    else
        sendPing(&dest, argv[1]);
    
    return (0);
}