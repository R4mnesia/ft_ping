#include "../include/ping.h"

void freeDest(t_ping dest) {

    free(dest.hostname);
}
