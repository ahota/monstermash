#include "mm_client.h"

int sock_fd, port, message_length;
struct sockaddr_in server_address;
struct hostent *server;

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr,"usage: %s hostname port\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[2]);
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        fprintf(stderr, "Could not open socket\n");
        exit(1);
    }
    server = gethostbyname(argv[1]);
    if(server == NULL) {
        fprintf(stderr,"Invalid hostname\n");
        exit(1);
    }

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&server_address.sin_addr.s_addr,
          server->h_length);
    server_address.sin_port = htons(port);
    if(connect(sock_fd,(struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Error connecting to server\n");
        exit(1);
    }
    printf(GREEN "Connected!\n" RESET);
    char *user_input = malloc(INPUT_BUFFER_SIZE);
    while(1) {
        //Wait for output/response from server
        //Get header
        char *header = malloc(INPUT_BUFFER_SIZE);
        memset(header, 0, INPUT_BUFFER_SIZE);
        int header_length = read(sock_fd, header, INPUT_BUFFER_SIZE);
        if(header_length < 0)
            fprintf(stderr, BOLDRED "Error reading from socket\n");
        if(header_length < INPUT_BUFFER_SIZE) {
            fprintf(stderr, YELLOW "DEBUG: Unexpected header size %d\n" RESET,
                    header_length);
            fprintf(stderr, YELLOW "DEBUG: Header=%s\n" RESET, header);
        }
        int response_length = atoi(header);
        //Get rest of response
        int i;
        char *server_output = malloc(response_length + 1);
        memset(server_output, 0, response_length + 1);
        int received_length = read(sock_fd, server_output + i, response_length);
        printf("%s", server_output);

        //If the user entered "exit" on the last iteration
        //get server response above and then quit
        if(strcmp(user_input, "exit") == 0)
            break;

        //Get user input and write to socket
        user_input = malloc(INPUT_BUFFER_SIZE);
        memset(user_input, 0, INPUT_BUFFER_SIZE);
        get_local_input(&user_input);
        if(strlen(user_input) == 0) {
            user_input[0] = '\n';
            user_input[1] = '\0';
        }
        message_length = write(sock_fd, user_input, strlen(user_input));
        if(message_length < 0)
            fprintf(stderr, BOLDRED "Error reading from socket\n");
    }
    close(sock_fd);
    return 0;
}

