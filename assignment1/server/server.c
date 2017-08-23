#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<netdb.h>

int check(char cred[]){
    FILE* fp;
    char buffer[500];

    fp = fopen("users.txt", "r");

    while(fgets(buffer, 500, (FILE*) fp)){
        int n=strlen(buffer);
        buffer[n-1] = '\0';
        if (!strcmp(buffer, cred)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]){

    struct sockaddr_in cli_addr, serv_addr;
    int cli_addr_len, s, cli_s;

    if((s=socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }
    
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(2222);

    if(bind(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
        perror("bind");
        exit(1);
    }

    listen(s, 5);

    while(1){
        char user[500];
        if((cli_s=accept(s, (struct sockaddr *)&cli_addr, &cli_addr_len)) == -1){
            perror("accept");
            exit(1);
        }
        int re=read(cli_s, user, sizeof(user));
        if(re <= 0){     
            perror("read username");
            exit(1);
        }
    
        char *len = strchr(user, '@');
        char cred[len-user+1];
        for (int i = 0; i < len-user; i++) {
            cred[i] = user[i];
        }
        cred[len-user] = '\0';

        if(check(cred))
        {
            char* colon = strchr(user, ':');
            
            char auth[500] = "Hello ";
            strncat(auth, user, colon-user);

            if(write(cli_s, auth, sizeof(auth)) != sizeof(auth)){
                perror("write");
                exit(1);
            }

            char fname[100];
            int fi=read(cli_s, fname, sizeof(fname));
            if (fi<=0) {
                perror("read file name");
                exit(1);
            }
            
            //fname[strlen(fname)-1] = '\0';
            //printf("%s\n", fname);
            FILE* fp;
            if (!(fp = fopen(fname, "r"))) {
                char fail[] = "File Not Found";
                
                if (write(cli_s, fail, sizeof(fail)) != sizeof(fail)) {
                    perror("file error");
                    exit(1);
                }
            }
            else{
                char data[1000];
                
                fread(data, sizeof(char), sizeof(data), fp);

                if (write(cli_s, data, sizeof(data)) != sizeof(data)) {
                    perror("file error");
                    exit(1);
                }

                fclose(fp);
            }
            close(cli_s);
        }
        else{
            char auth[] = "Authentication Failure!!!";

            if(write(cli_s, auth, sizeof(auth)) != sizeof(auth)){
                perror("write");
                exit(1);
            }
            close(cli_s);
        }
    }
}
