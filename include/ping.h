#ifndef PING_H
#define PING_H

#include "../libft/libft.h"
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <net/if.h>

#define DEFAULT_PACKET_SIZE 64
#define DEFAULT_COUNT_PACKET 4
#define DEFAULT_INTERVAL_PACKET 1
#define DEFAULT_TIMEOUT 1
#define DEFAULT_TTL 64

typedef struct s_ping {

    struct sockaddr_in      addr; 
    char                    *hostname;
    int                     sock;
    int                     verbose;

}               t_ping;


// Utils
void freeDest(t_ping dest);

#endif