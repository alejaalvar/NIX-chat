/**
 * @file client.c
 * @author Alejandro Alvarado
 * @brief Chat client using UNIX domain sockets
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/chat.sock"
#define BUF_SIZE 256

static volatile int is_running = 1;

static void handle_sig(int __attribute__((unused)) sig) { is_running = 0; }

int main(void) {
  int sockfd;
  struct sockaddr_un addr;
  char buf[BUF_SIZE]; // holding space for read/write
  fd_set read_fds;
  int stdin_fd = fileno(stdin);
  int max_fd;

  {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sig;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGPIPE, &sa,
              NULL); // prevents client from crashing if servere dies mid-send
  }

  // Create the socket
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (-1 == sockfd) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Clear the struct & set the struct
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  // Connect to the socket
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }
  printf("Connected to server!\n");

  max_fd = sockfd > stdin_fd ? sockfd : stdin_fd;

  // Begin listening
  while (is_running) {
    // Setup the file descriptors to watch
    FD_ZERO(&read_fds);
    FD_SET(stdin_fd, &read_fds);
    FD_SET(sockfd, &read_fds);

    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
      if (is_running) {
        perror("select");
      }
      break;
    }

    // Input received from the keyboard
    if (FD_ISSET(stdin_fd, &read_fds)) {
      // Input might be null, so we should quit
      if (fgets(buf, BUF_SIZE, stdin) == NULL) {
        printf("Disconnecting...\n");
        break; // break the infinite loop
      }
      send(sockfd, buf, strlen(buf), 0);
    }
    // Message received from the server
    if (FD_ISSET(sockfd, &read_fds)) {
      ssize_t n = recv(sockfd, buf, BUF_SIZE - 1, 0);
      if (n <= 0) {
        printf("Disconnecting...\n");
        break; // break the infinite loop
      }
      buf[n] = '\0';
      printf("Them: %s", buf);
    }
  }
  close(sockfd);
  exit(EXIT_SUCCESS);
}
