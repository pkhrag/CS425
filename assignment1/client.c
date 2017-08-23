#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in cli_addr, serv_addr;
    struct hostent *server;
    int s;
    char* user, auth[500], *addr;

    if(argc != 2){
        fprintf(stderr, "Usage:%s <username>:<password>@<serverip>\n", argv[0]);
        exit(1);
    }

    user = argv[1];
    addr = strchr(argv[1], '@') + 1; 
    
    if ((s=socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    bzero(&cli_addr, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(0);

    if (bind(s, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) == -1){
        perror("bind");
        exit(1);
    }

    if ((server=gethostbyname(addr)) == NULL){
        perror("gethostbyname");
        exit(1);
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = *((ulong *) server->h_addr);
    serv_addr.sin_port = htons(2222);

    if(connect(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        perror("connect");
        exit(1);
    }
    
    if(write(s, argv[1], strlen(argv[1])) != strlen(argv[1])){
        perror("Write username");
        exit(1);
    }

    int re=read(s, auth, sizeof(auth));
    
    if(re <= 0){
        fprintf(stderr, "Error in authentication\n");
        exit(1);
    }

    if(!strcmp(auth, "Authentication Failure!!!")){
        printf("%s\nWrong username or password!\n", auth);
        exit(0);
    }
    else{
        printf("Please enter the file name:");
        char fname[100];
        scanf("%s", fname);

        if(write(s, fname, strlen(fname)) != strlen(fname)){
            perror("Write file name");
            exit(1);
        }

        char exist[1000];
        int res = read(s, exist, sizeof(exist));
        if (res <= 0) {
            fprintf(stderr, "Error in checking the existing of file");
            exit(1);
        }

        if(!strcmp(exist, "File Not Found")){
            printf("File:%s doesn't exist on the server!\n", fname);
            exit(0);
        }
        else{
            printf("%s\n", exist);
            exit(0);
        }
    }
}
