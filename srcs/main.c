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

unsigned short checksum(void *b, int len) {

    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void    set_header_icmp(t_header *header) {

    memset(&header->icmp, 0, sizeof(struct icmphdr));
    memset(header->rest, 0, (64 - sizeof(struct icmphdr)));
    header->icmp.type = ICMP_ECHO; // 8
    header->icmp.code = 0;
    header->icmp.checksum = 0;
    header->icmp.checksum = checksum(&header->icmp, sizeof(header->icmp)); // function for check
    header->icmp.un.echo.sequence = 0; // number of request
    header->icmp.un.echo.id = getpid(); // getpid() ?? 
}

void    set_struct_time(t_time *time) {

    time->packet_received = 0;
    time->packet_time_diff = 0;
    time->packet_sent = 0;
    time->all_time = 0;
    time->seq = 0;
}

void    sendPing(t_ping *dest, char *arg) {

    struct sockaddr_in      d_addr;
    struct icmphdr          icmp;
    struct timeval stop, start, start_all, stop_all;
    unsigned int ttl = 64;
    t_header header;
    t_time  time;
    socklen_t socklen;

    set_header_icmp(&header);
    set_struct_time(&time);

    dest->sock = socket(dest->addr.sin_family, SOCK_DGRAM, IPPROTO_ICMP); // IPPROTO_ICMP (specifie au noyau d'utiliser ICMP)
    
    if (dest->sock == -1)
        Error_exit(ERROR_FD, dest->sock, dest->hostname);
    if (setsockopt(dest->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) // set socket for ttl
        Error_exit(ERROR_SOCKET, dest->sock, dest->hostname);
    gettimeofday(&start_all, NULL);

    if (dest->verbose == true) {
        printf("ping: sock4.fd: %d (socktype: SOCK_DGRAM), hints.ai_family: AF_INET\n\n", dest->sock);
        printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n", arg);
    }

    printf("PING %s (%s) %ld(%ld) bytes of data.\n", arg, dest->hostname, 64 - sizeof(struct icmphdr), 56 + sizeof(struct icmphdr) + 20);
    while (1) {
        header.icmp.checksum = checksum(&header.icmp, sizeof(header.icmp));
        gettimeofday(&stop_all, NULL);
        time.all_time = timedifference_msec(start_all, stop);
        if (check_signal(arg, time)) {
            continue ;
        }
        gettimeofday(&start, NULL);

        //if (sig == NO_SIGNAL) {
        if (sendto(dest->sock, &header, sizeof(header), 0, (struct sockaddr *)&dest->addr, sizeof(dest->addr)) == -1)
            Error_exit(ERROR_SEND, dest->sock, dest->hostname);
        time.packet_sent++;

        socklen = sizeof(d_addr);

        if (recvfrom(dest->sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)&d_addr, &socklen) == -1)
            Error_exit(ERROR_RECEIVE, dest->sock, dest->hostname);
        
        if (icmp.type == ICMP_ECHOREPLY) {
                gettimeofday(&stop, NULL);
                time.packet_time_diff = timedifference_msec(start, stop);
                printf("%ld bytes from %s (%s): icmp_seq:%d ttl=%d time=%.1f\n", sizeof(header), arg, dest->hostname, time.seq, ttl, time.packet_time_diff);
                time.packet_received++;
        }
        else
            Error_exit(ERROR_ECHO_REPLY, dest->sock, dest->hostname);
        time.time[time.seq] = time.packet_time_diff;
        time.seq++;
        header.icmp.un.echo.sequence = time.seq;
        //    }
        usleep(PING_SLEEP);

        
    }

    close(dest->sock);
}

void sigint_handler(int signal) {
    if (signal == SIGINT) {
        sig = CTRLC;
    }
    else if (signal == SIGQUIT) {
        sig = CTRLQUIT;
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
    sigaction(SIGQUIT, &act, NULL);
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
        sendPing(&dest, argv[1]);
    //printf("%s\n", dest.hostname);
    free(dest.hostname);
    //freeDest(&dest);
    return (0);
}