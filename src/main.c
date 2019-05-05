#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

#include "../includes/aux.h"

#define PORT "3333"
#define BACKLOG 10
#define REQUESTSIZE 1024

typedef struct aux{
    int id;
    sem_t *sem;
    int fsd;
    struct sockaddr_storage info_client;
    FILE* log;
}aux_t;

sem_t semafaro;

char *HTMLheader = "HTTP/1.1 200 OK\nServer: BServer\nAccept-Ranges: bytes\nContent-Type: text/html\n";
char *CSSheader = "HTTP/1.1 200 OK\nServer: BServer\nAccept-Ranges: bytes\nContent-Type: text/css\n";
char *ICOheader = "HTTP/1.1 200 OK\nServer: BServer\nAccept-Ranges: bytes\nContent-Type: image/x-icon\n";
char *JPEGheader = "HTTP/1.1 200 OK\nServer: BServer\nAccept-Ranges: bytes\nContent-Type: image/jpeg\n";
char *PNGheader = "HTTP/1.1 200 OK\nServer: BServer\nAccept-Ranges: bytes\nContent-Type: image/png\n";
char *NOTheader = "HTTP/1.1 404 NOT FOUND\nServer: BServer\nAccept-Ranges: bytes\nContent-Type: text/html\nContent-Length: 206\nConnection: Closed\n";
char *BADheader = "HTTP/1.1 400 BAD REQUEST\nServer: BServer\nAccept-Ranges: bytes\nContent-Type: text/html\nContent-Length: 317\nConnection: Closed\n";
char *Theader = "HTTP/1.1 200 OK\nServer: BServer\nAccept-Ranges: bytes\n";

int setupServer(char* port){
    struct addrinfo setup, *res, *p;
    int sockfd;
    int status;
    int yes = 1;

    memset(&setup, 0, sizeof setup);
    setup.ai_family = AF_UNSPEC;
    setup.ai_socktype = SOCK_STREAM;
    setup.ai_flags = AI_PASSIVE;

    int final = -1;
    if((status = getaddrinfo(NULL, port, &setup, &res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    };
    for(p = res;p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sockfd == -1){
            printf("socket error trying another\n");
            continue;
        }
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if(status == -1){
            printf("bindind error trying another\n");
            continue;
        }else{
            final = sockfd;
            break;
        }
    }
    p = NULL;
    freeaddrinfo(res);
    return final;
}

int lock = 0;

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* handle_client(void* a){
    aux_t *aux = a;
    int fsd = aux->fsd;
    char *fileBuffer;
    char s[INET6_ADDRSTRLEN];
    char requestBuffer[REQUESTSIZE];
    char header[512];
    char* name;
    char path[32] = "./files";
    int bytes_sent = 0;
    int bytesSize;

    if(fsd == -1)
        pthread_exit((void*)-1);
    recv(fsd, requestBuffer, REQUESTSIZE-1, 0);
    // puts(requestBuffer);
    if((inet_ntop(aux->info_client.ss_family, (get_in_addr((struct sockaddr *)&aux->info_client)), s, sizeof s)) == NULL){
        // printf("================================ DEU ERRO ===================================");
    }
    sem_wait(*(&aux->sem));
    fputs(s, aux->log);
    fputs("\n",aux->log);
	fputs(requestBuffer,aux->log);
    fflush(aux->log);    
    sem_post(*(&aux->sem));
    // printf("SSF: %u",(aux->info_client.ss_family));

    // puts("PASSEIIIIIIIIIIIIIIIIIIIIIIIIi");
    


    if(requestBuffer[0] != 'G'){
        // puts("Entrei aqui post");
        fileBuffer = ReadFile("./files/badrequest.html", &bytesSize);
        strcpy(header,BADheader);
        time_t t = time(NULL);
        struct tm tm = *gmtime(&t);
        char buf[128];
        strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z\n", &tm);
        char date[64] = "Date: ";
        strcat(date,buf);
        strcat(header,date);
        strcat(header,"\n");
        send(fsd, header, strlen(header), 0);
        send(fsd, fileBuffer, bytesSize, 0);
        free(fileBuffer);
        // fputs(header,aux->log);
        // fputs("\n",aux->log);
        // fflush(aux->log);

        close(fsd);

        pthread_exit((void*)-1);
    }else{
        name = getFileName(requestBuffer);
        strcat(path, name);
        fileBuffer = ReadFile(path, &bytesSize);
        if(fileBuffer == NULL){
            free(fileBuffer);
            bytesSize = 0;
            fileBuffer = ReadFile("./files/notfound.html", &bytesSize);
            strcpy(header,NOTheader);
            time_t t = time(NULL);
            struct tm tm = *gmtime(&t);
            char buf[128];
            strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z\n", &tm);
            char date[64] = "Date: ";
            strcat(date,buf);
            strcat(header,date);
            strcat(header,"\n");

            send(fsd, header, strlen(header), 0);
            bytes_sent = send(fsd, fileBuffer, bytesSize, 0);

            // fputs(header,aux->log);
            // fputs("\n",aux->log);
            // fflush(aux->log);
            recv(fsd, requestBuffer, REQUESTSIZE-1, 0);
            close(fsd);

            pthread_exit((void*)-1);
        }
        
    }

    if(strstr(name, ".ico")!=NULL){
        strcpy(header,ICOheader);
    }else if(strstr(name, ".html")!=NULL){
        strcpy(header,HTMLheader);
    }else if(strstr(name, ".jpeg")!=NULL){
        strcpy(header,JPEGheader);
    }else if(strstr(name, ".png")!=NULL){
        strcpy(header,PNGheader);
    }else if(strstr(name, ".css")!=NULL){
        strcpy(header,CSSheader);
    }else{
        strcpy(header,Theader);
    }
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    char buf[128];
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z\n", &tm);
    char date[64] = "Date: ";
    strcat(date,buf);
    strcat(header,date);
    strcat(header,"\n");

    bytes_sent = send(fsd, header, strlen(header), 0);
    bytes_sent = send(fsd, fileBuffer, bytesSize, 0);

    // fputs(header,aux->log);
    // fputs("\n",aux->log);
    // fflush(aux->log);

    free(fileBuffer);
    close(fsd);
}



int main(int argc, char const *argv[]){
    char port[16];
    char buff[32];
    if(argc == 1){
        puts("Missing port, using default port 3333. Usage:");
        sprintf(buff,"%s PORT\n\n", argv[0]);
        puts(buff);
        strcpy(port, PORT);
    }else{
        strcpy(port, argv[1]);
    }
    if(sem_init(&semafaro, 0, 1)){
        printf("Erro semafaro");
        return 255;
    }
    int sockfd = setupServer(port);
    listen(sockfd, BACKLOG);
    // printf("%d\n", sockfd);
    struct sockaddr_storage their_addr;
    int new_fd;
    socklen_t addr_size;
    addr_size = sizeof their_addr;
    FILE* LOGFILE = fopen("./log.txt", "a+");
    if(LOGFILE == NULL) printf("DEU RUIM");
    int tcount = 0;
    pthread_t threads[100];
    aux_t info[100];
    sprintf(buff,"Running server in port %s", port);
    puts(buff);
    while(1){
        if(tcount == 100) tcount = 0;
        // puts("to aqui7");
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

        if(new_fd != -1){
            info[tcount].info_client = their_addr;
            info[tcount].log = LOGFILE;
            info[tcount].id = tcount;
            info[tcount].sem = &semafaro;
            info[tcount].fsd = new_fd;
            int nt = pthread_create(&threads[tcount], NULL, handle_client, (void*) &info[tcount]);
            tcount++;
        }

       
    }
    close(sockfd);
    return 0;
}   
