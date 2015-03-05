#include "mm_client.h"

int sock_fd, port, message_length;
struct sockaddr_in server_address;
struct hostent *server;

int main() {
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        fprintf(stderr, BOLDRED "Could not open socket\n" RESET);
        exit(1);
    }

    //get server address
    char *user_input = malloc(INPUT_BUFFER_SIZE);
    printf("Server address : ");
    get_local_input(&user_input);
    server = gethostbyname(user_input);
    if(server == NULL) {
        fprintf(stderr, BOLDRED "Invalid host\n" RESET);
        exit(1);
    }

    //get port number
    printf("Port           : ");
    free(user_input);
    //really no need for it to be this big
    user_input = malloc(INPUT_BUFFER_SIZE);
    get_local_input(&user_input);
    port = atoi(user_input);

    //try to connect
    memset((void *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy(server->h_addr, &(server_address.sin_addr.s_addr), server->h_length);
    server_address.sin_port = htons(port);
    int err;
    if(connect(sock_fd, (struct sockaddr *) &server_address,
               sizeof(server_address)) < 0) {
        err = errno;
        fprintf(stderr, BOLDRED "Could not connecti, error=%d\n" RESET, err);
        exit(1);
    }

    free(user_input);
    printf(GREEN "Connected!\n" RESET);

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

