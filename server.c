/**
 * @file server.c
 * @author Alejandro Alvarado
 * @breif Chat server using Unix domain sockets
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/chat.sock"
#define BUF_SIZE 256
#define NUM_CLIENTS 2

int main(void) {
  int sockfd;
  int client_fds[NUM_CLIENTS]; // the fds to listen on
  struct sockaddr_un addr;
  char buf[BUF_SIZE]; // holding space for read/write
  fd_set read_fds;
  int maxfd;

  // Create socket
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Bind to path
  unlink(
      SOCKET_PATH); // in case the server crashed - prevents bind from failing
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  // Listen for connections
  if (listen(sockfd, NUM_CLIENTS) ==
      -1) { // will listen until we get NUM_CLIENTS active connections
    perror("listen");
    exit(EXIT_FAILURE);
  }
  printf("Server listening on %s\n", SOCKET_PATH);

  // We have two connnections - accept two clients
  for (int i = 0; i < NUM_CLIENTS; i++) {
    client_fds[i] = accept(sockfd, NULL, NULL);
    if (client_fds[i] == -1) {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    printf("Client %d connected.\n", i + 1);
  }
  printf("Both clients connected, starting relay.\n");

  // Relay loop - this forwards the message
  maxfd = client_fds[0] > client_fds[1] ? client_fds[0] : client_fds[1];

  while (1) {
    FD_ZERO(&read_fds);
    FD_SET(client_fds[0], &read_fds);
    FD_SET(client_fds[1], &read_fds);

    // maxfd + 1 is required by select (max fds to select on, plus one is reqd)
    if (select(maxfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(EXIT_FAILURE);
    }

    // Now we see if we have any messages on each fd (connection)
    for (int i = 0; i < NUM_CLIENTS; i++) {
      int other_connection_idx;
      if (FD_ISSET(client_fds[i], &read_fds)) {
        ssize_t n = recv(client_fds[i], buf, BUF_SIZE - 1, 0);
        if (n <= 0) {
          printf("Client %d disconnected.\n", i + 1);
          goto cleanup;
        }
        buf[n] = '\0';
        // Forward to the other client
        other_connection_idx = 1 - i;
        send(client_fds[other_connection_idx], buf, n, 0);
      }
    }
  }
cleanup:
  close(client_fds[0]);
  close(client_fds[1]);
  close(sockfd);
  unlink(SOCKET_PATH);
  exit(EXIT_SUCCESS);
}
