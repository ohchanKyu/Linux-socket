 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <signal.h>
 #include <sys/wait.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/time.h>

 #define PORT 3600
 #define MAX_BUFFER 1024

 int global_max = 0;
 int global_min = 0;

 struct cal_data
 {
        int left_num;
        int right_num;
        char op;
        int result;
        int max;
        int min;
        char description[MAX_BUFFER];
        short int error;
 };
 int main(int argc, char **argv)
 {
        struct sockaddr_in client_addr,server_addr;
        int addr_len, n;
        pid_t pid;
        int listen_fd, client_fd;
        struct cal_data rdata,pdata;
        int left_num,right_num,cal_result;
        short int cal_error;
        int connected_clients = 0;
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
        signal(SIGCHLD, SIG_IGN);
        while(1)
        {
                addr_len = sizeof(client_addr);
                client_fd = accept(listen_fd,(struct sockaddr *)&client_addr,&addr_len);
                if (client_fd == -1 ){
                        printf("accept error\n");
                        break;
                }
                connected_clients += 1;
                int fd[2];
                if (pipe(fd) < 0) {
                  perror("Pipe Error : ");
                  return 1;
                }
                pid = fork();
                if (pid == 0){
                        close(fd[0]);
                        close(listen_fd);
                        if ( n = read(client_fd,(void *)&rdata,sizeof(rdata)) <= 0){
                                close(client_fd);
                                return 0;
                        }
                        cal_result = 0;
                        cal_error = 0;
                        left_num = ntohl(rdata.left_num);
                        right_num = ntohl(rdata.right_num);
                        switch(rdata.op)
                        {
                                case '+':
                                        cal_result = left_num + right_num;
                                        break;
                                case '-':
                                        cal_result = left_num  - right_num;
                                        break;
                                case 'x':
                                        cal_result = left_num * right_num;
                                        break;
                                case '/':
                                        if(right_num == 0)
                                        {
                                                cal_error = 2;
                                        break;
                                        }
                                        cal_result = left_num / right_num;
                                        break;
                                default:
                                        cal_error = 1;
                        }
                        if (global_max == 0 && global_min == 0){
                                global_min=cal_result;
                                global_max=cal_result;
                        }else{
                                if (cal_result < global_min){
                                        global_min = cal_result;
                                }
                                if (cal_result > global_max){
                                        global_max = cal_result;
                                }
                        }
                        rdata.result = htonl(cal_result);
                        rdata.max = htonl(global_max);
                        rdata.min = htonl(global_min);
                        rdata.error = htons(cal_error);
                        write(fd[1], (void *)&rdata, sizeof(rdata));
                        close(fd[1]);
                        printf("%d%c%d=%d %s min=%d max=%d \n", left_num, 
                                rdata.op, right_num, cal_result,
                                rdata.description,global_min,global_max);
                        write(client_fd,(void *)&rdata,sizeof(rdata));
                        close(client_fd);
                        return 0;
                }else if (pid  > 0){
                        close(fd[1]);
                        close(client_fd);
                        int bytes_read = read(fd[0], (void *)&pdata, sizeof(pdata));
                        if (bytes_read > 0){
                          global_min = ntohl(pdata.min);
                          global_max = ntohl(pdata.max);
                        }
                        close(fd[0]);
                }else{
                        perror("Fork failed");
                        close(client_fd);
                        close(fd[0]);
                        close(fd[1]);
                }
        }
        close(client_fd);
        return 0;
 }
