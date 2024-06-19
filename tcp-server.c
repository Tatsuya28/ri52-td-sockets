#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 500

void usage(const char *progname) {
    fprintf(stderr, "Usage: %s -p <port_id> -h <serveur_ip>\n", progname);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE + 1];
    struct sockaddr_in my_address;
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
    my_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, host, &my_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(srv_socket);
        exit(EXIT_FAILURE);
    }
    my_address.sin_port = htons(port);
    memset(&(my_address.sin_zero), 0, sizeof(my_address.sin_zero));
    
    // Bind socket to the specified port
    if (bind(srv_socket, (struct sockaddr *) &my_address, sizeof(struct sockaddr_in)) < 0) {
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

    printf("Server listening on port %d\n", port);

    // Accept incomming connections
    while (1) {
        client_socket = accept(srv_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            close(srv_socket);
            exit(EXIT_FAILURE);
        }
        printf("\nClient connected\n\n");

        // Read and print messages from the client
        ssize_t bytes_received;
        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf("Received: %s", buffer);
        }
        
        if (bytes_received < 0) {
            perror("Receive failed");
        }

        printf("\nClient disconnected\n");
        close(client_socket);
    }

    close(srv_socket);
    return 0;
}