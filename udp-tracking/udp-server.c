#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 500

void usage(const char *progname) {
    fprintf(stderr, "Usage: %s -p <port_id> -h <serveur_ip>\n", progname);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE + 1];
    struct sockaddr_in srv_address, client_address;
    int srv_socket, client_socket;

    // Setting up getopt
    char *host;
    int port;
    int opt = 0;

    // Parse command line arguments
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
    srv_socket = socket(AF_INET, SOCK_DGRAM, 0);
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

    printf("Server listening on port %d\n\n", port);

    // Receive messages from client
    socklen_t client_address_len = sizeof(client_address);
    while (1) {
        ssize_t bytes_received = recvfrom(srv_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_address, &client_address_len);

        if (bytes_received < 0) {
            perror("Receive failed");
        }

        buffer[bytes_received] = '\0';
        // Print client address and received message
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("From %s:%d | Message: %s", client_ip, ntohs(client_address.sin_port), buffer);
    }

    close(srv_socket);
    return 0;
}