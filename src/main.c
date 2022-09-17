#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define newline "\n"
#define begin "\n\n"

char server_name[100];
char version[100];

struct {
    char c200[100]; // 200 OK
    char c404[100]; // 404 Not Found
    char c500[100]; // 500 Internal Server Error
} codes;

int sendf(int sock, char filename[], char type[]) { // Send file
    FILE *file;
    size_t length;
    char *sendstr = malloc(1000);
    char *content;

    file = fopen(filename, "rb"); // Open the file

    if (file == NULL) { // If file could not be opened
        strcpy(sendstr, version);          // Setting HTTP version
        strcat(sendstr, codes.c500);       // Setting the code (200, 404, 500 etc)
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Content-Type: "); // Content-Type:
        strcat(sendstr, type);             // The type provided
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Server: ");       // Server: 
        strcat(sendstr, server_name);      // The server's name
        strcat(sendstr, begin);            // Newlines (2 of them) to start sending the rest of the data
        strcat(sendstr, codes.c500);       // Sending error 500
        send(sock, sendstr, strlen(sendstr), 0);
        printf("[%s] Could not open file \"%s\". Are you sure the file exists?\n", server_name, filename); // Guess what this does
        close(sock);
        free(sendstr);
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
        strcpy(sendstr, version);          // Setting HTTP version
        strcat(sendstr, codes.c200);       // Setting the code (200, 404, 500 etc)
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Content-Type: "); // Content-Type: 
        strcat(sendstr, type);             // The type provided
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Server: ");       // Server: 
        strcat(sendstr, server_name);      // The server's name
        strcat(sendstr, begin);            // Newlines (2 of them) to start sending the rest of the data
        send(sock, sendstr, strlen(sendstr), 0);
    }

    send(sock, content, length, 0);
    free(sendstr);
    free(content);
    return 0;
}

int sendt(int sock, char input[], char type[]) { // Send text
    char *final;

    final = malloc(strlen(input) + 1000); // Malloc final request to be 1k bytes, plus input length

    if (type == NULL) { // If type does not have an input / is null (arg 3)
        strcpy(final, input);
    } else {
        strcpy(final, "HTTP/1.1 200 OK\nServer: %s\nContent-Type: "); // Start creating request
        strcat(final, type);
        strcat(final, "\n\n");
        strcat(final, input);
    }

    if (send(sock, final, strlen(final), 0) == -1) {
        printf("[%s] Could not send text to client.\n", server_name);
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

    strcpy(version,     "HTTP/1.1 "); // Setting version
    strcpy(server_name, "Carlton");   // Setting server name

    strcpy(codes.c200, "200 OK");                    // 200 OK
    strcpy(codes.c404, "404 Not Found");             // 404 Not Found
    strcpy(codes.c500, "500 Internal Server Error"); // 500 Internal Server Error


    printf("[%s] Starting...\n", server_name);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) { // If the socket could not create (obv)
        printf("[%s] Could not create the socket.\n", server_name);
    }

    printf("[%s] Socket created.\n", server_name);

    svradr.sin_family      = AF_INET;
    svradr.sin_addr.s_addr = INADDR_ANY;
    svradr.sin_port        = htons(1111);

    if (bind(sock, (struct sockaddr*)&svradr, sizeof(svradr)) == -1) { // Binding
        printf("[%s] Could not bind.\n", server_name);
        return 1;
    }

    printf("[%s] Binded.\n", server_name);
    
    if (listen(sock, 5) == -1) { // Listen
        printf("[%s] Could not listen (on the socket).\n", server_name);
        return 1;
    }
    
    printf("[%s] Binded and listening.\n", server_name);

    while (sock) { // While the socket sock exists
        cxn = accept(sock, (struct sockaddr*)&svradr, &size);
        recv(cxn, received, 8192, 0);

        token = strtok(received, "GET ");
        printf("[%s] CXN: %s\n", server_name, inet_ntoa(svradr.sin_addr));

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