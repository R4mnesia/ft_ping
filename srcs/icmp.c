#include "../include/ping.h"

void    resolve_hostname(t_ping *dest, const char *hostname, struct sockaddr_in *addr_dest)
{
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

unsigned short checksum(void *b, int len)
{
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

void    set_header_icmp(t_header *header)
{
    memset(&header->icmp, 0, sizeof(struct icmphdr));
    memset(header->rest, 0, (64 - sizeof(struct icmphdr)));
    header->icmp.type = ICMP_ECHO; // 8
    header->icmp.code = 0;
    header->icmp.checksum = 0;
    header->icmp.checksum = checksum(&header->icmp, sizeof(header->icmp)); // function for check
    header->icmp.un.echo.sequence = 0; // number of request
    header->icmp.un.echo.id = getpid(); // getpid() ?? 
}

void    set_struct_time(t_time *time)
{

    time->packet_received = 0;
    time->packet_time_diff = 0;
    time->packet_sent = 0;
    time->all_time = 0;
    time->seq = 0;
}
