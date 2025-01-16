#include "include/clients.h"
#include "include/common.h"

#include <stdlib.h>
#include <string.h>

int resize(clients_t * clients, int dir) {
  if (clients->max_clients + RESIZE_AMOUNT > MAX_CLIENTS) return -1;

  client_t * temp = (client_t *)realloc(clients->data, clients->max_clients + RESIZE_AMOUNT * dir);
  if (temp == NULL) return -1;

  clients->data = temp;
  clients->max_clients += RESIZE_AMOUNT;

  return 0;
}

void init_clients_db(clients_t * clients) {
  memset(clients, 0, sizeof(clients_t));

  clients->clients = 0;
  clients->max_clients = 10;
  clients->data = (client_t *)malloc(10 * sizeof(client_t));
}

int add_client(clients_t * clients, int fd, struct sockaddr_in addr) {
  if (clients->clients == clients->max_clients && resize(clients, 1) == -1)
    return -1;

  clients->data[clients->clients].index = clients->clients;
  clients->data[clients->clients].sock_fd = fd;
  clients->data[clients->clients].addr = addr;
  memset(&clients->data[clients->clients].data, '\0', BUF_32_KB);

  ++clients->clients;

  return 0;
}

void remove_client(clients_t * clients, int index) {

}