#include <string.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
 #include <sys/stat.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/time.h>

 #define IP "10.20.0.189"
 #define PORT 3600
 #define MAX_BUFFER 1024

 int global_max = 0;
 int global_min = 0;

 struct data
 {
    int left_num;
    int right_num;
    char op;
    char str[1024];
    short int error;
 };
 struct cal_data
 {
    int left_num;
    int right_num;
    char op;
    int result;
    char str[1024];
    int max;
    int min;
    short int error;
 };
 int main(int argc, char **argv)
 {
    struct sockaddr_in client_addr,server_addr;
    struct data client_data;
    struct cal_data server_cal_data;
    struct sockaddr_in server_socket;
    struct tm server_time;
    time_t current_time;
    size_t total_size = sizeof(struct cal_data) + sizeof(struct tm) 
                                                      + sizeof(struct sockaddr_in);
    server_socket.sin_family = AF_INET;
    server_socket.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &(server_socket.sin_addr));
    int addr_len;
    int listen_fd, client_fd;
    int fd_num;
    int maxfd = 0;
    fd_set readfds, allfds;
    if ((listen_fd = socket(AF_INET,SOCK_STREAM,0)) < 0){
            perror("Error");
            return 1;
    }
    memset((void *)&server_addr,0x00,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    if (bind(listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1 )
    {
        perror("Bind Error");
        return 1;
    }
    if (listen(listen_fd,5) == -1 ){
        perror("Listen Error");
        return 1;
    }
    FD_ZERO(&readfds);
    FD_SET(listen_fd,&readfds);
    maxfd = listen_fd;
    while(1){
        allfds = readfds;
        fd_num = select(maxfd + 1,&allfds,(fd_set *) 0,(fd_set *) 0, NULL);
        if (FD_ISSET(listen_fd,&allfds)){
            addr_len = sizeof(client_addr);
            client_fd = accept(listen_fd,(struct sockaddr *)&client_addr, &addr_len);
            FD_SET(client_fd,&readfds);
            if (client_fd > maxfd) maxfd = client_fd;
            printf("Accept New Client!\n");
            continue;
        }
        for(int sockfd=0; sockfd<=maxfd; sockfd++){
            if (FD_ISSET(sockfd,&allfds)){
                int readn;
                if ( readn = read(sockfd, &client_data, sizeof(client_data)) <= 0 ){
                    close(sockfd);
                    FD_CLR(sockfd,&readfds);
                }else{
                    server_cal_data.left_num = ntohl(client_data.left_num);
                    server_cal_data.right_num = ntohl(client_data.right_num);
                    server_cal_data.op = client_data.op;
                    strcpy(server_cal_data.str,client_data.str);
                    int num1 = server_cal_data.left_num;
                    int num2 = server_cal_data.right_num;
                    char op =  server_cal_data.op;
                    if (num1 == 0 && num2 == 0 && op == '$' 
                                     && strcmp(client_data.str, "quit") == 0)
                    {
                        close(sockfd);
                        FD_CLR(sockfd,&readfds);
                        break;
                    }
                    switch(op)
                    {
                        case '+':
                                server_cal_data.result = num1 + num2;
                                break;
                        case '-':
                                server_cal_data.result = num1  - num2;
                                break;
                        case 'x':
                                server_cal_data.result = num1 * num2;
                                break;
                        case '/':
                                if(num2 == 0)
                                {
                                        server_cal_data.error = 2;
                                        break;
                                }
                                server_cal_data.result = num1 / num2;
                                break;
                        default:
                                server_cal_data.error = 1;
                    }
                    if (global_max == 0 && global_min == 0){
                        global_max = server_cal_data.result;
                        global_min = server_cal_data.result;
                    }else {
                        if (server_cal_data.result < global_min) {
                                global_min = server_cal_data.result;
                        }
                        if (server_cal_data.result > global_max) {
                                global_max = server_cal_data.result;
                        }
                    }
                    server_cal_data.max = global_max;
                    server_cal_data.min = global_min;
                    server_cal_data.left_num = htonl(server_cal_data.left_num);
                    server_cal_data.right_num = htonl(server_cal_data.right_num);
                    server_cal_data.result = htonl(server_cal_data.result);
                    server_cal_data.max = htonl(server_cal_data.max);
                    server_cal_data.min = htonl(server_cal_data.min);
                    time(&current_time);
                    struct tm *temp_time = localtime(&current_time);
                    memcpy(&(server_time),temp_time,sizeof(struct tm));
                    char *sendBuffer = malloc(total_size);
                    memcpy(sendBuffer, &server_cal_data, sizeof(struct cal_data));
                    memcpy(sendBuffer + sizeof(struct cal_data), &(server_time),
                                             sizeof(struct tm));
                    memcpy(sendBuffer + sizeof(struct cal_data) + sizeof(struct tm),
                                         &(server_socket), sizeof(struct sockaddr_in));
                    ssize_t bytes_written = write(sockfd, sendBuffer, total_size);
                    if (bytes_written == -1) {
                        perror("Write failed");
                    }
                    free(sendBuffer);
                }
                if (--fd_num <= 0 ) break;
          }
     }
  }
}
