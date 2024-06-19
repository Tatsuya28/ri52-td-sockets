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
    struct sockaddr_in srv_address;
    int sock_to_server;

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

    sock_to_server = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_to_server < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    srv_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, host, &srv_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock_to_server);
        exit(EXIT_FAILURE);
    }
    srv_address.sin_port = htons(port);
    memset(&(srv_address.sin_zero), 0, sizeof(srv_address.sin_zero));

    // Send messages to server
    while (1) {
        printf("Enter your message: ");
        fflush(stdout);
        size_t bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            perror("Read failed");
            close(sock_to_server);
            exit(EXIT_FAILURE);
        }
        buffer[bytes_read] = '\0';

        size_t bytes_sent = sendto(sock_to_server, buffer, bytes_read+1, 0, (struct sockaddr *) &srv_address, sizeof(struct sockaddr_in));
        if (bytes_sent < 0) {
            perror("Send failed");
            close(sock_to_server);
            exit(EXIT_FAILURE);
        }
    }

    close(sock_to_server);
    return 0;
}