#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>

#define CLIENT_READ 1
#define CLIENT_WRITE 2
#define CLIENT_EXCEPT 3
#define CLIENT_BUFF_BLOCKSIZE 150

struct client {
    char *buff;
    int len;
    int capacity;
    int fd;
    int state;
    struct client *next;
};

/*
    Adds fd's from global client list to given fd_sets r, w, e. Adds listening
    socket fd listen_fd to read set. Returns highest numbered fd added to sets.
*/
int init_sets(int listen_fd, fd_set *r, fd_set *w, fd_set *e);

/*
    Performs next read on given client socket and updates state.
*/
void handle_read(struct client *client);

/*
    Performs next write on given client socket and updates state.
*/
void handle_write(struct client *client);

/*
    Handle except condition on given client socket (drop connection).
*/
void handle_except(struct client *client);

/*
    Accept any incoming connections on listening socket and add them to the 
    global client list. Returns number of new connections on success or -1 on
    failure.
*/
int accept_connections(int listen_fd);

/*
    Search global client list for client state for given connection fd. Returns
    NULL if no match found in client list.
*/
struct client *find_client(int client_fd);

/*
    Free memory in global client list and close listening fd.
*/
void cleanup_server(int listen_fd);

#endif
