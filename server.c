#include "smartalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

int running = 1;

void sigint_handler(int sig) {
    running = 0;
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigint_handler);
    struct sockaddr_in v4;
    struct sockaddr_in6 v6;

    if (argc != 1 && argc != 2) {
        printf("Usage: %s [IP address]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        if (inet_pton(AF_INET, argv[1], &(v4.sin_addr)) == 1) {
            printf("Valid IPv4 address: %s\n", argv[1]);
        } else if (inet_pton(AF_INET6, argv[1], &(v6.sin6_addr)) == 1) {
            printf("Valid IPv6 address: %s\n", argv[1]);
        } else {
            printf("Invalid IP address: %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("No IP address provided, binding to all interfaces.\n");
    }

    while (running) {
        printf("server running...\n");
        sleep(5);
    }

    printf("\nServer exiting cleanly.\n");

    // clean up stuff

    return 0;
}