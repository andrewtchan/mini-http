#define CLIENT_READ 1
#define CLIENT_WRITE 2
#define CLIENT_EXCEPT 3

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