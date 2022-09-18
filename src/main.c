#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define newline "\n"
#define begin "\n\n"

struct sockaddr_in svradr;

struct {
    uint16_t port;
    char server_name[100];
    char version[100];
    int sock;
    int cxn;
} app;

struct {
    char c200[100]; // 200 OK
    char c404[100]; // 404 Not Found
    char c500[100]; // 500 Internal Server Error
} codes;

const char *strcode(int code) { // Returns status codes as strings (from int)
    switch (code) {
        case 200:
            return codes.c200;
            break;
        case 404:
            return codes.c404;
            break;
        case 500:
            return codes.c500;
            break;
        default:
            return codes.c200; // Defaultly return code 200
            break;
    }
}

int sendf(int sock, char filename[], char type[], int code) { // Send file
    FILE *file;
    size_t length;
    char *sendstr = malloc(1000);
    char *content;

    file = fopen(filename, "rb"); // Open the file

    if (file == NULL) { // If file could not be opened
        strcpy(sendstr, app.version);      // Setting HTTP version
        strcat(sendstr, codes.c500);       // Setting the status code (200, 404, 500 etc)
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Content-Type: "); // Content-Type:
        strcat(sendstr, type);             // The type provided
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Server: ");       // Server: 
        strcat(sendstr, app.server_name);  // The server's name
        strcat(sendstr, begin);            // Newlines (2 of them) to start sending the rest of the data
        strcat(sendstr, codes.c500);       // Sending error 500
        send(sock, sendstr, strlen(sendstr), 0);
        printf("[%s] Could not open file \"%s\". Are you sure the file exists?\n", app.server_name, filename); // Guess what this does
        close(sock);
        free(sendstr);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);        // Go to the end of file
    length  = ftell(file);           // Get length
    content = malloc(length);        // Malloc content with length bytes
    rewind(file);                    // Go to the start of file
    fread(content, length, 1, file); // Read the bloody thing
    fclose(file);                    // All done, close file

    if (type != NULL) { // If type does not have an input / is null (arg 3)
        strcpy(sendstr, app.version);      // Setting HTTP version
        strcat(sendstr, strcode(code));    // Setting the status code
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Content-Type: "); // Content-Type: 
        strcat(sendstr, type);             // The type provided
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Server: ");       // Server: 
        strcat(sendstr, app.server_name);  // The server's name
        strcat(sendstr, begin);            // Newlines (2 of them) to start sending the rest of the data
    }

    send(sock, sendstr, strlen(sendstr), 0);
    send(sock, content, length, 0);
    free(sendstr);
    free(content);
    return 0;
}

int sendt(int sock, char input[], char type[], int code) { // Send text
    char *sendstr = malloc(strlen(input) + 1000); // Malloc final request to be 1k bytes, plus input length

    if (type == NULL) { // If type does not have an input / is null (arg 3)
        strcpy(sendstr, input);
    } else {
        strcpy(sendstr, app.version);      // Setting HTTP version
        strcat(sendstr, strcode(code));    // Setting the status code
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Content-Type: "); // Content-Type: 
        strcat(sendstr, type);             // The type provided
        strcat(sendstr, newline);          // Add newline for next bit of data
        strcat(sendstr, "Server: ");       // Server: 
        strcat(sendstr, app.server_name);  // The server's name
        strcat(sendstr, begin);            // Newlines (2 of them) to start sending the rest of the data
        strcat(sendstr, input);            // Data
    }

    send(sock, sendstr, strlen(sendstr), 0);

    free(sendstr);    
    return 0;
}

void *s_cxn(void *arg) { // Multithread function
    char *received;
    char token[100];
    int loop;
    int sock = app.cxn;

    memset(token, 0, 100);

    received = malloc(8129);
    recv(sock, received, 8192, 0);

    if (strncmp(received, "GET ", 4) == 0) { // If it is a GET request
        loop = 0;
        for (int i = 4; i < strlen(received); ++i) { // Reading page requested
            if (received[i] == ' ') {
                break;
            }

            token[loop] = received[i];
            ++loop;
        }
    } else {
        sendt(sock, "Unsupported request.", "text/plain", 500); // Send 500
        close(sock);
    }

    printf("[%s] CXN: %s\n", app.server_name, inet_ntoa(svradr.sin_addr));

    // This part here is where you configurate which request leads to what file
    if (strcmp(token, "/") == 0) {
        sendf(sock, "pages/index.html", "text/html", 200);
    } else if (strcmp(token, "/example") == 0) {
        sendf(sock, "pages/example.html", "text/html", 200);
    } else if (strcmp(token, "/favicon.ico") == 0) {
        sendf(sock, "icons/favicon.ico", "image/x-icon", 200);
    } else {
        sendf(sock, "pages/404.html", "text/html", 404);
    }

    close(sock);
    free(received);
    return 0;
}

void ctrlc(int dummy) {
    printf("\n[%s] Closing main sockets...\n", app.server_name);
    close(app.sock);
    close(app.cxn);
    shutdown(app.sock, SHUT_RDWR);
    shutdown(app.cxn,  SHUT_RDWR);
    printf("[%s] Exiting...\n", app.server_name);
    exit(0);
}
 
int main(void) {
    pthread_t thread;
    socklen_t size;
    int optval = 1;

    signal(SIGINT, ctrlc);

    strcpy(app.version,     "HTTP/1.1 "); // Setting version
    strcpy(app.server_name, "Carlton");   // Setting server name

    strcpy(codes.c200, "200 OK");                    // 200 OK
    strcpy(codes.c404, "404 Not Found");             // 404 Not Found
    strcpy(codes.c500, "500 Internal Server Error"); // 500 Internal Server Error


    printf("[%s] Starting...\n", app.server_name);

    app.sock = socket(AF_INET, SOCK_STREAM, 0);
    app.port = 1111;

    if (app.sock == -1) { // If the socket could not create (obv)
        printf("[%s] Could not create the socket.\n", app.server_name);
    }

    printf("[%s] Socket created.\n", app.server_name);

    svradr.sin_family      = AF_INET;         // Vin Diesel
    svradr.sin_addr.s_addr = INADDR_ANY;      // Address (None really)
    svradr.sin_port        = htons(app.port); // Port

    setsockopt(app.sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // Allows reuse of the socket so you do not have to wait for it to time out after exit.

    if (bind(app.sock, (struct sockaddr*)&svradr, sizeof(svradr)) == -1) { // Binding
        printf("[%s] Could not bind.\n", app.server_name);
        return 1;
    }

    printf("[%s] Binded.\n", app.server_name);
    
    if (listen(app.sock, 5) == -1) { // Listen
        printf("[%s] Could not listen (on the socket).\n", app.server_name);
        return 1;
    }
    
    printf("[%s] Binded and listening.\n", app.server_name);

    while (app.sock) { // While the socket sock exists
        if ((app.cxn = accept(app.sock, (struct sockaddr*)&svradr, &size)) == -1) {
            printf("[%s] Could not accept the connection.\n", app.server_name);
            close(app.cxn);
            continue;
        }

        pthread_create(&thread, NULL, s_cxn, NULL);
    }
}