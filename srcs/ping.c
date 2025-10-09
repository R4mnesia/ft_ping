#include "../include/ping.h"

void    sendPing(t_ping *dest, char *arg)
{
    struct sockaddr_in      d_addr;
    struct icmphdr          *icmp;
    //struct timeval          stop, start;
    unsigned int ttl = 64;
    unsigned short pid16 = (unsigned short)(getpid() & 0xFFFF);
    t_header header;
    t_time  time;
    //socklen_t socklen;

    set_header_icmp(&header);
    set_struct_time(&time);
    
    char packet[PACKET_SIZE]; // 64
    memset(packet, 0, PACKET_SIZE);

    // SET SOCK_RAW
    dest->sock = socket(dest->addr.sin_family, SOCK_RAW, IPPROTO_ICMP); // IPPROTO_ICMP (specifie au noyau d'utiliser ICMP)

    if (dest->sock == -1)
        Error_exit(ERROR_FD, dest->sock, dest->hostname);

    // DEST ADDRESS
    memset(&d_addr, 0, sizeof(d_addr));
    d_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, dest->hostname, &d_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(dest->sock);
        printf("fdp");
        return ;
    }

    // SET TIMEVAL
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(dest->sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        Error_exit(ERROR_SOCKET, dest->sock, dest->hostname);


    if (setsockopt(dest->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) // set socket for ttl
        Error_exit(ERROR_SOCKET, dest->sock, dest->hostname);

    if (dest->verbose == true)
    {
        printf("ping: sock4.fd: %d (socktype: SOCK_DGRAM), hints.ai_family: AF_INET\n\n", dest->sock);
        printf("ai->ai_family: AF_INET, ai->ai_canonname: '%s'\n", arg);
    }

    printf("PING %s (%s) %ld(%ld) bytes of data.\n", arg, dest->hostname, 64 - sizeof(struct icmphdr), 56 + sizeof(struct icmphdr) + 20);
    while (1)
    {
        // SET HEADER ICMP
        icmp = (struct icmphdr *)packet;
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->un.echo.id = htons(pid16);;
        icmp->un.echo.sequence = htons(header.icmp.un.echo.sequence);
        
        struct timeval *tv_payload = (struct timeval *)(packet + sizeof(struct icmphdr));
        gettimeofday(tv_payload, NULL);

        if (check_signal(arg, time)) {
            continue ;
        }
        // CHECKSUM
        int icmp_len = PACKET_SIZE;
        icmp->checksum = 0;
        icmp->checksum = checksum(packet, icmp_len);

        if (sendto(dest->sock, &packet, icmp_len, 0, (struct sockaddr *)&dest->addr, sizeof(dest->addr)) == -1)
            Error_exit(ERROR_SEND, dest->sock, dest->hostname);
        


        // RECEIVE
        char recvbuf[1500];
        struct sockaddr_in src;
        socklen_t src_len = sizeof(src);
        ssize_t recvd = recvfrom(dest->sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&src, &src_len);
        if (recvd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("Request timeout for icmp_seq %d\n", time.seq);
                // continue loop (like ping)
                usleep(PING_SLEEP);
                continue;
            }
            else {
                perror("recvfrom");
                Error_exit(ERROR_RECEIVE, dest->sock, dest->hostname);
            }
        }

        
        struct iphdr *ip = (struct iphdr *)recvbuf;
        int ip_hdr_len = ip->ihl * 4;
        if (recvd < ip_hdr_len + (int)sizeof(struct icmphdr)) {
            fprintf(stderr, "Packet too short (%zd bytes)\n", recvd);
            usleep(PING_SLEEP);

            continue;
        }

        struct icmphdr *r_icmp = (struct icmphdr *)(recvbuf + ip_hdr_len);

        if (r_icmp->type == ICMP_ECHOREPLY)
        {
            unsigned short r_id = ntohs(r_icmp->un.echo.id);
            unsigned short r_seq = ntohs(r_icmp->un.echo.sequence);
            if (r_id == pid16)
            {

                // get timeval from payload
                struct timeval *sent_tv = (struct timeval *)((char*)r_icmp + sizeof(struct icmphdr));
                struct timeval now;
                gettimeofday(&now, NULL);
                double rtt = timedifference_msec(*sent_tv, now);

                char src_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &src.sin_addr, src_ip, sizeof(src_ip));

                int icmp_bytes = icmp_len;
                printf("%d bytes from %s (%s): icmp_seq=%u ttl=%u time=%.3f ms\n", icmp_bytes, src_ip, dest->hostname, r_seq, ttl, rtt);
                // tu peux mettre à jour tes compteurs ici (sent/received, min/max/avg)
            }
            else
            {
                // paquet reçu mais pas pour nous
                if (dest->verbose)
                    fprintf(stderr, "Received ICMP reply with different id %u (expected %u)\n", r_id, pid16);
            }
        }
        else
        {
            // gérer autres types (Destination Unreachable, Time Exceeded, ...)
            if (dest->verbose)
                printf("Received ICMP type=%d code=%d\n", r_icmp->type, r_icmp->code);
        }
        time.seq++;
        header.icmp.un.echo.sequence = time.seq;
        usleep(PING_SLEEP);
        
    }

    close(dest->sock);
}