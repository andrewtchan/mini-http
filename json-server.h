#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>

#define CLIENT_READ 1
#define CLIENT_WRITE 2
#define CLIENT_EXCEPT 3
#define CLIENT_BUFF_BLOCKSIZE 250
#define URI_IMPLEMENTED 1
#define URI_QUIT 2
#define URI_ABOUT 3
#define RES_404 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 162\r\n\r\n<HTML><HEAD><TITLE>HTTP ERROR 404</TITLE></HEAD><BODY>404 Not Found.  Your request could not be completed due to encountering HTTP error number 404.</BODY></HTML>"
#define RES_IMPLEMENTED "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 94\r\n\r\n[\r\n{ \"feature\": \"about\", \"URL\": \"/json/about.json\"},{ \"feature\": \"quit\", \"URL\": \"/json/quit\"}]\r\n"
#define RES_QUIT "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 26\r\n\r\n{\r\n  \"result\": \"success\"\r\n}\r\n"
#define RES_ABOUT "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 80\r\n\r\n{\r\n  \"author\": \"Andrew Chan\",  \"email\": \"achan136@calpoly.edu\",  \"major\": \"CSC\"}\r\n"
#define RES_MAXLEN 250

struct client {
    char *buff;
    int len; // index of next readable/writable byte 
    int capacity; // size of buffer or length of response
    int fd;
    int state;
    int uri;
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
    Looks for a CRLF sequence in buff up to len. Returns pointer to start of
    CRLF if found, else returns -1.
*/
char *find_crlf(char *buff, int len);

/*
    Looks for a CRLFCRLF sequence in buff up to len. Returns 0 if found, else
    returns -1.
*/
int find_crlfcrlf(char *buff, int len);

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

/*
    Close connection associated with client_fd and free memory in global client
    list.
*/
void close_client(int client_fd);

#endif
