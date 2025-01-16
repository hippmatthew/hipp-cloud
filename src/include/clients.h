#ifndef hippcloud_clients_h
#define hippcloud_clients_h

#include "common.h"

#include <arpa/inet.h>

#define RESIZE_AMOUNT 10

typedef struct {
  int index;
  int sock_fd;
  struct sockaddr_in addr;
  char data[BUF_32_KB];
} client_t;

typedef struct {
  long clients;
  long max_clients;
  client_t * data;
} clients_t;

void init_clients_db(clients_t * clients);
int add_client(clients_t * clients, int fd, struct sockaddr_in addr);
void remove_client(clients_t * clients, int fd);
client_t * get_client_by_index(int index);
client_t * get_client_by_fd(int fd);

#endif // hippcloud_clients_h