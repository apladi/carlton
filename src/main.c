#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define MAXSIZE 2000

int sendf(int sock, char filename[], char type[]) {
    FILE *file;
    const char *error = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\n\n500 | Internal Server Error";
    size_t length;
    char *sendstr;
    char *content;

    file = fopen(filename, "rb");

    if (file == NULL) {
        printf("[Carlton] Could not open file \"%s\". Are you sure the file exists?\n", filename);
        send(sock, error, strlen(error), 0);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    length  = ftell(file);
    content = malloc(length);
    sendstr = malloc(1000);
    rewind(file);
    fread(content, length, 1, file);
    fclose(file);

    if (type != NULL) {
        strcpy(sendstr, "HTTP/1.1 200 OK\nServer: Carlton\nContent-Type: ");
        strcat(sendstr, type);
        strcat(sendstr, "\n\n");
        send(sock, sendstr, strlen(sendstr), 0);
    }

    send(sock, content, length, 0);
    return 0;
}

int sendt(int sock, char input[], char type[]) {
    char *final;

    final = malloc(strlen(input) + 1000);

    if (type == NULL) {
        strcpy(final, input);
    } else {
        strcpy(final, "HTTP/1.1 200 OK\nServer: Carlton\nContent-Type: ");
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

    if (sock == -1) {
        printf("[Carlton] Could not create the socket.\n");
    }

    printf("[Carlton] Socket created.\n");

    svradr.sin_family      = AF_INET;
    svradr.sin_addr.s_addr = INADDR_ANY;
    svradr.sin_port        = htons(1111);

    if (bind(sock, (struct sockaddr*)&svradr, sizeof(svradr)) == -1) {
        printf("[Carlton] Could not bind.\n");
        return 1;
    }

    printf("[Carlton] Binded.\n");
    
    if (listen(sock, 5) == -1) {
        printf("[Carlton] Could not listen (on the socket).\n");
        return 1;
    }
    
    printf("[Carlton] Binded and listening.\n");

    while (sock) {
        cxn = accept(sock, (struct sockaddr*)&svradr, &size);
        recv(cxn, received, 8192, 0);

        token = strtok(received, "GET ");
        printf("[Carlton] CXN: %s\n", inet_ntoa(svradr.sin_addr));

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