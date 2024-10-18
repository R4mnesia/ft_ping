#include "../include/ping.h"

int globalVariable = 0;

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.00f + (t1.tv_usec - t0.tv_usec) / 1000.00f;
}

void    resolve_hostname(t_ping *dest, const char *hostname, struct sockaddr_in *addr_dest) {
    
    struct hostent   *host;
    
    host = gethostbyname(hostname);
    if (!host)
        return ;

    /*
        struct in_addr {
        unsigned long s_addr;  // load with inet_aton()
        };
    */

    dest->hostname = strdup(inet_ntoa(*(struct in_addr *)host->h_addr)); // binary IP to strings IP
    (*addr_dest).sin_family = host->h_addrtype; // AF_INET (ipv4)
    (*addr_dest).sin_port = 0;
    (*addr_dest).sin_addr.s_addr = *(long *)host->h_addr;
}

void    sendPingNoVerbose(t_ping *dest, char *arg) {

    unsigned int seq = 0;
    unsigned int ttl = 64;
    struct sockaddr_in      d_addr;
    struct icmphdr          icmp;
    t_header header;
    socklen_t socklen;
    struct timeval stop, start;
    float diff = 0;

    memset(header.rest, 0, (64 - sizeof(struct icmphdr)));
    header.icmp.type = ICMP_ECHO; // 8
    header.icmp.code = 0;
    header.icmp.checksum = 0; // function for check
    header.icmp.un.echo.sequence = 0; // number of request
    header.icmp.un.echo.id = 2; // getpid() ??

    dest->sock = socket(dest->addr.sin_family, SOCK_DGRAM, IPPROTO_ICMP); // IPPROTO_ICMP (specifie au noyau d'utiliser ICMP)
    if (dest->sock == -1)
        ERROR(1, dest->sock, dest->hostname);
    if (setsockopt(dest->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) // set socket for ttl
        ERROR(2, dest->sock, dest->hostname);

    while (1) {
        CTRLC(arg, seq, diff);
        gettimeofday(&start, NULL);

        if (sendto(dest->sock, &header, sizeof(header), 0, (struct sockaddr *)&dest->addr, sizeof(dest->addr)) == -1)
            ERROR(3, dest->sock, dest->hostname);

        socklen = sizeof(d_addr);

        if (recvfrom(dest->sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)&d_addr, &socklen) == -1)
            ERROR(4, dest->sock, dest->hostname);
        
        if (!sig) // sig = 0 = no CTRLC 
        { 
            if (icmp.type == ICMP_ECHOREPLY) {
                gettimeofday(&stop, NULL);
                diff = timedifference_msec(start, stop);
                printf("%ld bytes from %s (%s): icmp_seq:%d ttl=%d time=%f\n", sizeof(header), arg, dest->hostname, seq, ttl, diff);
            }
            else
                ERROR(5, dest->sock, dest->hostname);
        }
        usleep(SL);
        seq++;
    }

    close(dest->sock);
    printf("OK");

}
void sigint_handler(int signal)
{
    if (signal == SIGINT) {
        sig = 1;
    }
}

int main(int argc, char **argv) {

    if (argc > 3 || argc < 2) {
        printf("Error arguments\n");
        return (1);
    }
    
    struct sigaction act;
    bzero(&act, sizeof(act));

    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);
    t_ping  dest;
    if (!ParseArg(argc, argv, &dest)) {
        printf("Error arguments\n");
        freeDest(&dest);
        return (1);
    }
    if (!dest.hostname) {
        printf("ft_ping: %s: Name or service not known\n", argv[1]);
    }
    else
        sendPingNoVerbose(&dest, argv[1]);
    //printf("%s\n", dest.hostname);
    free(dest.hostname);
    //freeDest(&dest);
    return (0);
}