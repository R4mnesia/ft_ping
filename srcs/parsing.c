#include "ft_ping.h"

void    help() 
{
    printf("Usage\n");
    printf("    ft_ping [options] <destination>\n\n");
    printf("Options:\n");
    printf("        -v              varbose output\n\n");
}

int ParseArg(int argc, char **argv, t_ping *dest)
{
    for (int i = 0; i < (int)strlen(*argv); i++)
    {
        if (argv[i] && strcmp(argv[i], "-?") == 0) 
        {
            help();
            return (1);
        }
    }

    char *hostname = NULL;
    dest->verbose = false;
    dest->hostname = NULL;
    dest->sock = -1;

    if (argc == 2)
        hostname = argv[1];
    else if (argc == 3 && strcmp(argv[1], "-v") == 0)
    {
        dest->verbose = true;
        if (argc == 3)
        {
            hostname = argv[2];
        }
    } 
    else if (argc == 3 && strcmp(argv[2], "-v") == 0)
    {
        dest->verbose = true;
        hostname = argv[1];
    }

    if (hostname)
    {
        resolve_hostname(dest, hostname, (struct sockaddr_in *)&dest->addr);
        dest->sock = 0;
        return (1);
    }

    return (0);
}