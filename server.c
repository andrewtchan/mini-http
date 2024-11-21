#include "smartalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

int running = 1;

void sigint_handler(int sig) {
    running = 0;
}

int main(int argc, char **argv)
{
    // set up signal handler
    signal(SIGINT, sigint_handler);

    // parse command line args
    struct sockaddr_in v4;
    struct sockaddr_in6 v6;
    int addr_version;
    if (argc != 1 && argc != 2) {
        printf("Usage: %s [IP address]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        if (inet_pton(AF_INET, argv[1], &(v4.sin_addr)) == 1) {
            printf("Valid IPv4 address: %s\n", argv[1]);
            addr_version = AF_INET;
        } else if (inet_pton(AF_INET6, argv[1], &(v6.sin6_addr)) == 1) {
            printf("Valid IPv6 address: %s\n", argv[1]);
            addr_version = AF_INET6;
        } else {
            fprintf(stderr, "Invalid IP address: %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("No IP address provided, binding to all interfaces.\n");
        addr_version = 0;
    }

    // create socket
    int sock_fd;
    if (addr_version == AF_INET) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            perror("Error creating socket.\n");
            return -1;
        }
    } else {
        sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            perror("Error creating socket.\n");
            return -1;
        }
        int sock_opt = 0;
        if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_V6ONLY, &sock_opt, sizeof(sock_opt)) < 0) {
            perror("Failed to clear V6ONLY socket option.\n");
            close(sock_fd);
            return -1;
        }
    }

    // bind socket to address and port (handle v4 or v6)
    if (addr_version == AF_INET) {
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(0);
        server_addr.sin_addr = v4.sin_addr;
        if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Failed to bind socket to IPv4 address.\n");
            close(sock_fd);
            return -1;
        }
    } else {
        struct sockaddr_in6 server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_port = htons(0);
        if (addr_version == 0) {
            server_addr.sin6_addr = in6addr_any;
        } else {
            server_addr.sin6_addr = v6.sin6_addr;
        }
        if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Failed to bind socket to IPv6 address.\n");
            close(sock_fd);
            return -1;
        }
    }

    // listen for 10 connections
    if (listen(sock_fd, 10) < 0) {
        perror("Failed to listen.\n");
        close(sock_fd);
        return -1;
    }

    // startup output
    in_port_t port;
    if (addr_version == AF_INET) {
        struct sockaddr_in temp;
        socklen_t len = sizeof(temp);
        if (getsockname(sock_fd, (struct sockaddr *)&temp, &len) == -1) {
            perror("Failed to getsockname.\n");
            close(sock_fd);
            return -1;
        }
        port = ntohs(temp.sin_port);
    } else {
        struct sockaddr_in6 temp;
        socklen_t len = sizeof(temp);
        if (getsockname(sock_fd, (struct sockaddr *)&temp, &len) == -1) {
            perror("Failed to getsockname.\n");
            close(sock_fd);
            return -1;
        }
        port = ntohs(temp.sin6_port);
    }
    printf("HTTP server is using TCP port %d\n", port);
    printf("HTTPS server is using TCP port -1\n");
    fflush(stdout);

    // MAIN SERVER LOOP
    while (running) {
        printf("server running...\n");
        sleep(5);
    }

    printf("\nServer exiting cleanly.\n");
    // clean up stuff
    close(sock_fd);

    return 0;
}