#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <sys/poll.h>

#define BUFFER_SIZE 500
#define MAX_CLIENTS 100

void usage(const char *progname) {
    fprintf(stderr, "Usage: %s -p <port_id> -h <serveur_ip>\n", progname);
    exit(EXIT_FAILURE);
}

void close_client(struct pollfd *fds, int i, int *nfds) {
    close(fds[i].fd);
    for (int j = i; j < *nfds - 1; j++) {
        fds[j] = fds[j + 1];
    }
    (*nfds)--;
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE + 1], response[BUFFER_SIZE + 1];
    struct sockaddr_in srv_address, client_address;
    int srv_socket, client_socket;

    // Setting up getopt
    char *host;
    int port;
    int opt = 0;

    while ((opt = getopt(argc, argv, "p:h:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                printf("Defined port: %d\n", port);
                break;
            case 'h':
                host = optarg;
                printf("Defined host: %s\n", host);
                break;
            default:
                usage(argv[0]);
        }
    }

    if (host == NULL || port == 0) {
        usage(argv[0]);
    }

    // Create socket
    srv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    srv_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, host, &srv_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(srv_socket);
        exit(EXIT_FAILURE);
    }
    srv_address.sin_port = htons(port);
    memset(&(srv_address.sin_zero), 0, sizeof(srv_address.sin_zero));
    
    // Bind socket to the specified port
    if (bind(srv_socket, (struct sockaddr *) &srv_address, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind failed");
        close(srv_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incomming connections
    if (listen(srv_socket, 10) < 0) {
        perror("Listen failed");
        close(srv_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server %s listening on port %d\n", host, port);

    // Initialize the poll structure
    int nfds = 1, i, j;
    struct pollfd fds[MAX_CLIENTS];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = srv_socket;
    fds[0].events = POLLIN;

    // Accept incomming connections
    while (1) {
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0) {
            perror("Poll failed");
            break;
        }

        for (i = 0; i < nfds; i++) {
            if (fds[i].revents == 0) continue;

            if (fds[i].revents != POLLIN) {
                fprintf(stderr, "Error: revents = %d\n", fds[i].revents);
                close_client(fds, i, &nfds);
                continue;
            }

            if (fds[i].fd == srv_socket) {
                socklen_t client_address_len = sizeof(client_address);
                client_socket = accept(srv_socket, (struct sockaddr *) &client_address, &client_address_len);
                if (client_socket < 0) {
                    perror("Accept failed");
                    continue;
                }

                if (nfds < MAX_CLIENTS) {
                    fds[nfds].fd = client_socket;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    printf("New connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                } else {
                    fprintf(stderr, "Max clients reached, rejecting new connection.\n");
                    close(client_socket);
                }
            } else {
                ssize_t bytes_received = recv(fds[i].fd, buffer, BUFFER_SIZE, 0);
                if (bytes_received <= 0) {
                    if (bytes_received == 0) {
                        printf("Client disconnected\n");
                    } else {
                        perror("Recv failed");
                    }
                    close_client(fds, i, &nfds);
                } else {
                    buffer[bytes_received] = '\0';
                    printf("Received: %s", buffer);
                    for (j = 1; j < nfds; j++) {
                        if (fds[j].fd != fds[i].fd && fds[j].fd != -1) {
                            if (send(fds[j].fd, buffer, bytes_received, 0) < 0) {
                                perror("Send failed");
                                close_client(fds, j, &nfds);
                                j--;
                            }
                        }
                    }
                }
            }
        }
    }

    for (i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0) {
            close(fds[i].fd);
        }
    }
    return 0;
}