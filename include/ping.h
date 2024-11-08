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
#include <stdbool.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <net/if.h>
#include <time.h>

//#define DEFAULT_PACKET_SIZE 64
//#define DEFAULT_COUNT_PACKET 4
//#define DEFAULT_INTERVAL_PACKET 1
//#define DEFAULT_TIMEOUT 1
//#define DEFAULT_TTL 64


/*
struct sockaddr_in {
    sa_family_t    sin_family;  famille d'adresses : AF_INET     
    uint16_t       sin_port;    port dans l'ordre d'octets réseau 
    struct in_addr sin_addr;    adresse Internet                  
};
*/

/*

    Type 0 – Echo reply
    Type 3 – Destination unreachable
    Type 5 – Redirect Message
    Type 8 – Echo Request
    Type 11 – Time Exceeded
    Type 12 – Parameter problem

*/

/*

struct icmphdr
{
  u_int8_t type;                / message type 
  u_int8_t code;                / type sub-code
  u_int16_t checksum;
  union
  {
    struct
    {
      u_int16_t        id;
      u_int16_t        sequence;   / count 
    } echo;                        / echo datagram 

    u_int32_t        gateway;        / gateway address
    struct
    {
      u_int16_t        __unused;
      u_int16_t        mtu;
    } frag;                        / path mtu discovery 
    
  } un;

};

*/

/*struct icmp_hdr {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};*/

#define ERROR_FD 1
#define ERROR_SEND 2
#define ERROR_SOCKET 3
#define ERROR_RECEIVE 4
#define ERROR_ECHO_REPLY 5 

extern int sig;

#define PING_SLEEP 1000000
#define NO_SIGNAL 0
#define CTRLC 1
#define CTRLQUIT 2

typedef struct s_header {

    struct icmphdr icmp;
    char    rest[64 - sizeof(struct icmphdr)]; // rest header

}              t_header;

typedef struct s_ping {

    struct sockaddr_in      addr; 
    char                    *hostname;
    int                     sock;
    bool                     verbose;  

}               t_ping;

typedef struct s_time {

    float time[4096];
    float packet_time_diff;
    int packet_received;
    int packet_sent;
    int all_time;
    int seq;


}              t_time;

// Utils
void freeDest(t_ping *dest);
int check_signal(char *arg, t_time time);
int ParseArg(int argc, char **argv, t_ping *dest);
void Error_exit(int n, int sock, char *host);

// main
void  resolve_hostname(t_ping *dest, const char *hostname, struct sockaddr_in *addr_dest);



#endif