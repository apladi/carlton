#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int sendf(int sock, char filename[], char type[]) { // Send file
    FILE *file;
    size_t length;
    const char *error = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\n\n500 | Internal Server Error"; // Internal server error (can you read)
    char *sendstr;
    char *content;

    file = fopen(filename, "rb"); // Open the file

    if (file == NULL) { // If file could not be opened
        printf("[Carlton] Could not open file \"%s\". Are you sure the file exists?\n", filename); // Guess what this does
        send(sock, error, strlen(error), 0);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);        // Go to the end of file
    length  = ftell(file);           // Get length
    content = malloc(length);        // Malloc content with length bytes
    sendstr = malloc(1000);          // Malloc sendstr to 1k because http request main body
    rewind(file);                    // Go to the start of file
    fread(content, length, 1, file); // Read the bloody thing
    fclose(file);                    // All done, close file

    if (type != NULL) { // If type does not have an input / is null (arg 3)
        strcpy(sendstr, "HTTP/1.1 200 OK\nServer: Carlton\nContent-Type: "); // Start creating request
        strcat(sendstr, type);                                               // Set the type
        strcat(sendstr, "\n\n");                                             // Add 2 newlines to start sending file data
        send(sock, sendstr, strlen(sendstr), 0);
    }

    send(sock, content, length, 0);
    return 0;
}

int sendt(int sock, char input[], char type[]) { // Send text
    char *final;

    final = malloc(strlen(input) + 1000); // Malloc final request to be 1k bytes, plus input length

    if (type == NULL) { // If type does not have an input / is null (arg 3)
        strcpy(final, input);
    } else {
        strcpy(final, "HTTP/1.1 200 OK\nServer: Carlton\nContent-Type: "); // Start creating request
        strcat(final, type);
        strcat(final, "\n\n");
        strcat(final, input);
    }

    if (send(sock, final, strlen(final), 0) == -1) {
        printf("[Carlton] Could not send text to client.\n");
        return 1;
    }
    
    return 0;
}

int main(void) {
    socklen_t size;
    time_t current;
    struct sockaddr_in svradr;
    char *received = malloc(8192);
    char *token;
    int count;
    int sock;
    int cxn;

    printf("[Carlton] Starting...\n");

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) { // If the socket could not create (obv)
        printf("[Carlton] Could not create the socket.\n");
    }

    printf("[Carlton] Socket created.\n");

    svradr.sin_family      = AF_INET;
    svradr.sin_addr.s_addr = INADDR_ANY;
    svradr.sin_port        = htons(1111);

    if (bind(sock, (struct sockaddr*)&svradr, sizeof(svradr)) == -1) { // Binding
        printf("[Carlton] Could not bind.\n");
        return 1;
    }

    printf("[Carlton] Binded.\n");
    
    if (listen(sock, 5) == -1) { // Listen
        printf("[Carlton] Could not listen (on the socket).\n");
        return 1;
    }
    
    printf("[Carlton] Binded and listening.\n");

    while (sock) { // While the socket sock exists
        cxn = accept(sock, (struct sockaddr*)&svradr, &size);
        recv(cxn, received, 8192, 0);

        if (strncmp(received, "GET ", 4) != 0) { // If it is not a get request
            printf("[Carlton] Post requests are unsupported. Blocked one.\n");
            close(cxn);
            continue;
        }

        token = strtok(received, "GET ");
        printf("[Carlton] CXN: %s\n", inet_ntoa(svradr.sin_addr));

        // This part here is where you configurate which request leads to what file
        if (strcmp(token, "/") == 0) {
            sendf(cxn, "pages/index.html", "text/html");
        } else if (strcmp(token, "/example") == 0) {
            sendf(cxn, "pages/example.html", "text/html");
        } else if (strcmp(token, "/favicon.ico") == 0) {
            sendf(cxn, "icons/icon5.png", "image/png");
        } else {
            sendf(cxn, "pages/404.html", "text/html");
        }
        
        close(cxn);
    }
}