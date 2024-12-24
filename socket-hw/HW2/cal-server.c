 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/stat.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/time.h>

 #define PORT 3600
 #define MAX_CLIENTS 3
 #define MAX_BUFFER 1024

 int global_max = 0;
 int global_min = 0;

 void replace_newline_with_space(char *str){
    size_t len = strlen(str);
    if (len > 0 && str[len -1] == '\n'){
            str[len -1] = ' ';
    }
 }
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
        struct sockaddr_in client_addr, sock_addr;
        char buf[MAX_CLIENTS][MAX_BUFFER];
        int listen_sockfd, client_sockfd[MAX_CLIENTS];
        int addr_len,n;
        struct cal_data rdata;
        int left_num, right_num, cal_result;
        short int cal_error;
        int connected_clients=0;
        if( (listen_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                perror("Error ");
                return 1;
        }
        memset((void *)&sock_addr, 0x00, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        sock_addr.sin_port = htons(PORT);
        if( bind(listen_sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
        {
                perror("Bind Error ");
                return 1;
        }
        if(listen(listen_sockfd, 5) == -1)
        {
                perror("Listening Error ");
                return 1;
        }
        while (connected_clients < MAX_CLIENTS){
                addr_len = sizeof(client_addr);
                client_sockfd[connected_clients] = accept(listen_sockfd,
                                (struct sockaddr *)&client_addr, &addr_len);
                if (client_sockfd[connected_clients] == -1 ){
                        perror("accept error : ");
                        continue;
                }
                printf("New Client[%d] Connect : %s\n",
                connected_clients,inet_ntoa(client_addr.sin_addr));
                connected_clients++;
        }
        while(1){
                for (int i=0; i < MAX_CLIENTS; i++){
                  if ( n = read(client_sockfd[i], (void *)&rdata, sizeof(rdata)) <= 0){
                        close(client_sockfd[i]);
                        continue;
                  }
                  cal_result=0;
                  cal_error=0;
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
                   if (i == 0){
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
                   strcpy(buf[i], rdata.description);
                   replace_newline_with_space(buf[i]);
                   char combined_msg[MAX_BUFFER] = "";
                   for (int j = 0; j <= i; j++) {
                        strcat(combined_msg, buf[j]);
                        if (j < i) {
                                strcat(combined_msg, " ");
                        }
                   }
                   strncpy(rdata.description, combined_msg, sizeof(rdata.description) - 1);
                   rdata.description[sizeof(rdata.description) - 1] = '\0';
                   rdata.error = htons(cal_error);
                   printf("%d %c %d = %d  min=%d max=%d str=%s \n", 
                                    left_num, rdata.op, right_num, cal_result,global_min
                                   ,global_max,combined_msg);
                   write(client_sockfd[i], (void *)&rdata, sizeof(rdata));
                   close(client_sockfd[i]);
                }
        }
        close(listen_sockfd);
        return 0;
 }
