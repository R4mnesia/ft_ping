#include "../include/ping.h"

int init_socket(t_ping *dest, unsigned int ttl) 
{
    // SET SOCK_RAW (use with sudo)
    // IPPROTO_ICMP (specifie au noyau d'utiliser ICMP)
    int sock = socket(dest->addr.sin_family, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1)
        Error_exit(ERROR_FD, dest->sock, dest->hostname);

    // SET TIMEVAL
    struct timeval timeout = {1, 0}; // set tv_sec and tv_usec (1s timeout)
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        Error_exit(ERROR_SOCKET, dest->sock, dest->hostname);

    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) // set socket for ttl
        Error_exit(ERROR_SOCKET, sock, dest->hostname);

    return (sock);
}

void    build_packet(char *packet, unsigned short pid16, unsigned short seq)
{
        struct icmphdr *icmp = (struct icmphdr *)packet;
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->un.echo.id = htons(pid16);
        icmp->un.echo.sequence = htons(seq);

        struct timeval *tv_payload = (struct timeval *)(packet + sizeof(struct icmphdr));
        gettimeofday(tv_payload, NULL);

        icmp->checksum = 0;
        icmp->checksum = checksum(packet, PACKET_SIZE);
}

ssize_t receive_packet(int sockfd, char *buf, size_t buflen, struct sockaddr_in *src)
{
    socklen_t srclen = sizeof(*src);
    int       recv = recvfrom(sockfd, buf, buflen, 0, (struct sockaddr *)src, &srclen);

    return (recv);
}

int handle_reply(char *recvbuf, ssize_t recvd,
                 unsigned short pid16, t_ping *dest, t_time *time)
{
    struct  iphdr *ip = (struct iphdr *)recvbuf;
    int     ip_hdr_len = ip->ihl * 4;

    if (recvd < ip_hdr_len + (int)sizeof(struct icmphdr)) {
        if (dest->verbose)
            fprintf(stderr, "Packet too short (%zd bytes)\n", recvd);
        return 0;
    }

    struct  icmphdr *r_icmp = (struct icmphdr *)(recvbuf + ip_hdr_len);

    if (r_icmp->type == ICMP_ECHOREPLY)
    {
        unsigned short r_id = ntohs(r_icmp->un.echo.id);
        unsigned short r_seq = ntohs(r_icmp->un.echo.sequence);

        if (r_id != pid16) {
            if (dest->verbose)
                fprintf(stderr, "Received ICMP reply with different id %u (expected %u)\n", r_id, pid16);
            return 0;
        }

        // RTT calculation
        struct timeval *sent_tv = (struct timeval *)((char*)r_icmp + sizeof(struct icmphdr));
        struct timeval now;
        gettimeofday(&now, NULL);
        double rtt = timedifference_msec(*sent_tv, now);

        time->packet_time_diff = rtt;

        int icmp_bytes = recvd - ip_hdr_len;
        printf("%d bytes from %s (%s): icmp_seq=%u ttl=%u time=%.1f ms\n",
               icmp_bytes, dest->hostname, dest->hostname, r_seq, ip->ttl, rtt);

        return (1); // valid reply handled
    }
    else
    {
        if (dest->verbose)
            printf("Received ICMP type=%d code=%d\n", r_icmp->type, r_icmp->code);
    }

    return (0); // not a valid echo reply
}

void    sendPing(t_ping *dest, char *arg)
{
    int             ttl = DEFAULT_TTL;
    // Keep only the 16 least significant bits of the PID for portability
    uint16_t        pid16 = (uint16_t)(getpid() & 0xFFFF);
    char            packet[PACKET_SIZE]; // 64b
    t_time          time;

    set_struct_time(&time);
    memset(packet, 0, PACKET_SIZE);

    dest->sock = init_socket(dest, ttl);

    if (dest->verbose == true)
    {
        printf("ping: sock4.fd: %d (socktype: SOCK_DGRAM), hints.ai_family: AF_INET\n\n", dest->sock);
        printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n", arg);
    }

    printf("PING %s (%s) %d(%ld) bytes of data.\n", arg, dest->hostname, ICMP_DATA_LEN, PACKET_SIZE + 20);
    while (1)
    {

        // SET HEADER ICMP
        build_packet(packet, pid16, time.seq);

        if (check_signal(arg, time))
            continue ;

        if (sendto(dest->sock, &packet, PACKET_SIZE, 0, (struct sockaddr *)&dest->addr, sizeof(dest->addr)) == -1)
            Error_exit(ERROR_SEND, dest->sock, dest->hostname);
        time.packet_sent++;

        // RECEIVE
        char    recvbuf[1500];
        struct  sockaddr_in src;
        ssize_t recvd = receive_packet(dest->sock, recvbuf, 1500, &src);
        if (recvd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("Request timeout for icmp_seq %d\n", time.seq);
                // continue loop (like ping)
                usleep(PING_SLEEP);
                continue;
            }
            else
            {
                perror("recvfrom");
                Error_exit(ERROR_RECEIVE, dest->sock, dest->hostname);
            }
        }
        time.packet_received++;
        
        /*
            ihl = Internet Header Length 
            get the actual size of the IP header in bytes -> * 4 

        */
        struct          iphdr *ip = (struct iphdr *)recvbuf;
        unsigned int    ip_hdr_len = ip->ihl * 4;

        if (recvd < ip_hdr_len + (int)sizeof(struct icmphdr))
        {
            fprintf(stderr, "Packet too short (%zd bytes)\n", recvd);
            usleep(PING_SLEEP);

            continue;
        }

        // ECHO REPLY
        handle_reply(recvbuf, recvd, pid16, dest, &time);

        time.time[time.seq] = time.packet_time_diff;
        time.seq++;
        usleep(PING_SLEEP);
        
    }

    close(dest->sock);
}