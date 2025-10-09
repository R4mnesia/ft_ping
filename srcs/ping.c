#include "../include/ping.h"

void    sendPing(t_ping *dest, char *arg)
{
    struct sockaddr_in      d_addr;
    struct icmphdr          icmp;
    struct timeval stop, start, start_all, stop_all;
    unsigned int ttl = 64;
    t_header header;
    t_time  time;
    socklen_t socklen;

    set_header_icmp(&header);
    set_struct_time(&time);
    
    char packet[64];
    memset(packet, 0, sizeof(packet));

    // SET SOCK_RAW
    dest->sock = socket(dest->addr.sin_family, SOCK_RAW, IPPROTO_ICMP); // IPPROTO_ICMP (specifie au noyau d'utiliser ICMP)

    if (dest->sock == -1)
        Error_exit(ERROR_FD, dest->sock, dest->hostname);

    // DEST ADDRESS
    memset(&d_addr, 0, sizeof(d_addr));
    d_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, arg, &d_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    // SET TIMEVAL
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(dest->sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        Error_exit(ERROR_SOCKET, dest->sock, dest->hostname);


    if (setsockopt(dest->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) // set socket for ttl
        Error_exit(ERROR_SOCKET, dest->sock, dest->hostname);
    gettimeofday(&start_all, NULL);




    if (dest->verbose == true)
    {
        printf("ping: sock4.fd: %d (socktype: SOCK_DGRAM), hints.ai_family: AF_INET\n\n", dest->sock);
        printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n", arg);
    }

    printf("PING %s (%s) %ld(%ld) bytes of data.\n", arg, dest->hostname, 64 - sizeof(struct icmphdr), 56 + sizeof(struct icmphdr) + 20);
    while (1)
    {

        // SET HEADER ICMP
        icmp = (struct icmphdr)packet;
        icmp.type = ICMP_ECHO;
        icmp.code = 0;
        icmp.un.echo.id = getpid();
        icmp.un.echo.sequence = 1;
        icmp.checksum = checksum(&icmp, sizeof(packet));
        
        
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
        
        if (icmp.type == ICMP_ECHOREPLY) 
        {
                gettimeofday(&stop, NULL);
                time.packet_time_diff = timedifference_msec(start, stop);
                printf("%ld bytes from %s (%s): icmp_seq:%d ttl=%d time=%.1f ms\n", sizeof(header), arg, dest->hostname, time.seq, ttl, time.packet_time_diff);
                time.packet_received++;
        }
        else
            Error_exit(ERROR_ECHO_REPLY, dest->sock, dest->hostname);
        time.time[time.seq] = time.packet_time_diff;
        time.seq++;
        header.icmp.un.echo.sequence = time.seq;
        usleep(PING_SLEEP);

        
    }

    close(dest->sock);
}