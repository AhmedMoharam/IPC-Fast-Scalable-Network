#include <stdio.h>              /*common.h perror*/
#include <cstdlib>

#define handle_error(msg)  \
            do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define DEFAULT_SOCKET_PATH "/tmp/local_netwrok_socket"
#define MAX_PACKET_SIZE 512
