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
#include <fcntl.h>
#include <sys/select.h>

#include "server.h"

int running = 1;
struct client *client_list = NULL;

void sigint_handler(int sig) {
    running = 0;
}

int init_sets(int listen_fd, fd_set *r, fd_set *w, fd_set *e) {
    FD_ZERO(r);
    FD_ZERO(w);
    FD_ZERO(e);
    FD_SET(listen_fd, r); // add listening socket to read set
    int max_fd = listen_fd;

    // loop over client list, adding fd's to sets based on state and update max
    struct client *cur = client_list;
    while (cur) {
        if (cur->fd > max_fd) {
            max_fd = cur->fd;
        }
        switch (cur->state) {
            case CLIENT_READ:
                FD_SET(cur->fd, r);
                break;
            case CLIENT_WRITE:
                FD_SET(cur->fd, w);
                break;
            case CLIENT_EXCEPT:
                FD_SET(cur->fd, e);
                break;
        }
        cur = cur->next;
    }

    return max_fd;
}

void handle_read(struct client *client) {
    return;
}

void handle_write(struct client *client) {
    return;
}

void handle_except(struct client *client) {
    return;
}

int accept_connections(int listen_fd) {
    struct client *new;
    int new_fd;
    int added = 0;
    while (1) {
        new_fd = accept(listen_fd, NULL, NULL);
        if (new_fd < 0) { // no more incoming or fatal error
            perror("accept.\n");
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != ENETDOWN &&
                errno != EPROTO && errno != ENOPROTOOPT && errno != EHOSTDOWN &&
                errno != ENONET && errno != EHOSTUNREACH && errno != EOPNOTSUPP &&
                errno != ENETUNREACH) {
                return -1;
            }
            return added;
        }

        // add new client to global client list (head) in read state
        new = malloc(sizeof(struct client));
        if (!new) {
            perror("malloc failed.\n");
            return -1;
        }
        new->buff = malloc(CLIENT_BUFF_BLOCKSIZE);
        if (!new) {
            perror("malloc failed.\n");
            free(new);
            return -1;
        }
        new->len = 0;
        new->capacity = CLIENT_BUFF_BLOCKSIZE;
        new->fd = new_fd;
        new->state = CLIENT_READ;
        new->next = client_list;

        client_list = new;

        added += 1;
    }
}

struct client *find_client(int client_fd) {
    struct client *cur = client_list;
    while (cur) {
        if (cur->fd == client_fd) {
            return cur;
        } else {
            cur = cur->next;
        }
    }

    return NULL;
}

void cleanup_server(int listen_fd) {
    struct client *cur = client_list;
    struct client *temp = NULL;
    while (cur) {
        temp = cur->next;
        free(cur->buff);
        close(cur->fd); // close all open sockets
        free(cur);
        cur = temp;
    }
    cur = NULL;
    temp = NULL;

    close(listen_fd); // close listening socket fd
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
            perror("Failed to create socket.\n");
            return -1;
        }
    } else {
        sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            perror("Failed to create socket.\n");
            return -1;
        }
        int sock_opt = 0;
        if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_V6ONLY, &sock_opt, sizeof(sock_opt)) < 0) {
            perror("Failed to clear V6ONLY socket option.\n");
            close(sock_fd);
            return -1;
        }
    }

    // put socket in non-blocking mode
    int cntl_flag;
    cntl_flag = fcntl(sock_fd, F_GETFL, 0);
    if (cntl_flag == -1) {
        perror("Failed to get socket flags.\n");
        close(sock_fd);
        return -1;
    }
    if (fcntl(sock_fd, F_SETFL, cntl_flag | O_NONBLOCK) == -1) {
        perror("Failed to set socket flags.\n");
        close(sock_fd);
        return -1;
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

    // create read, write, except fd sets
    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    // MAIN SERVER LOOP
    int retval, max_fd, i;
    struct client *c;
    while (running) {
        // init fd sets from global client list & set max fd value
        max_fd = init_sets(sock_fd, &rfds, &wfds, &efds);

        // get ready fds
        retval = select(max_fd + 1, &rfds, &wfds, &efds, NULL);
        if (retval == -1) {
            perror("Failed to get fds in select.\n");
            cleanup_server(sock_fd);
            return -1;
        }

        for (i = 0; i <= max_fd; i++) {
            // read set (check for listening socket fd)
            if (FD_ISSET(i, &rfds)) {
                if (i == sock_fd) {
                    // accept and configure all incoming connections
                    retval = accept_connections(sock_fd);
                    if (retval == -1) {
                        cleanup_server(sock_fd);
                        return -1;
                    }
                } else {
                    // search client list for connection with fd
                    c = find_client(i);
                    if (!c) {
                        fprintf(stderr, "client in rfds not in clist.\n");
                        continue;
                    }

                    handle_read(c);
                }
                continue;
            }

            // write set
            if (FD_ISSET(i, &wfds)) {
                // search client list for connection with fd
                c = find_client(i);
                if (!c) {
                    fprintf(stderr, "client in wfds not in clist.\n");
                    continue;
                }

                handle_write(c);
                continue;
            }

            // except set
            if (FD_ISSET(i, &efds)) {
                // search client list for connection with fd
                c = find_client(i);
                if (!c) {
                    fprintf(stderr, "client in efds not in clist.\n");
                    continue;
                }

                handle_except(c);
                continue;
            }
        }
    }

    printf("Server exiting cleanly.\n");
    cleanup_server(sock_fd);

    return 0;
}
