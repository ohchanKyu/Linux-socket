 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <arpa/inet.h>
 #include <sys/stat.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <sys/time.h>
 #include <time.h>

 #define PORT 3600
 #define IP "10.20.0.189"
 int sock;

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
 int main(int argc, char **argv){
    struct sockaddr_in addr;
    int sock;
    char buffer[1024];
    int bytes_read;
    ssize_t len = sizeof(struct data);
    int sendByte;
    struct data client_data;
    struct tm server_time;
    struct sockaddr_in server_socket;
    struct cal_data calculation;
    char server_read_buffer[sizeof(struct cal_data) + sizeof(struct tm) + 
                                                             sizeof(struct sockaddr_in)];
    sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if ( sock == -1 ){
            return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if ( connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1 )
    {
            printf("fail to connect\n");
            close(sock);
            return 1;
    }
    while (1) {
        printf("> ");
        fflush(stdout);
        memset(buffer,0x00,sizeof(buffer));
        bytes_read = read(STDIN_FILENO,buffer,sizeof(buffer)-1);
        if (bytes_read <= 0 ){
        }
        buffer[bytes_read] = '\0';
        int num1,num2;
        char op;
        char str[1024];
        if (sscanf(buffer,"%d %d %c %[^\n]", &num1,&num2,&op,str) != 4 )
        {
            printf("Invalid input format. Expected : num1 num2 op description\n");
            return 0;
        }
        memset((void *)&client_data, 0x00,sizeof(client_data));
        client_data.left_num = htonl(num1);
        client_data.right_num = htonl(num2);
        client_data.op = op;
        strncpy(client_data.str,str,sizeof(client_data.str) - 1);
        client_data.str[sizeof(client_data.str) -1] = '\0';
        sendByte = write(sock, (char *)&client_data,len);
        if (sendByte != len ){
            return 1;
        }
        if (num1 == 0 && num2 == 0 && op == '$' && strcmp(str, "quit") == 0)
        {
              break;
        }
        printf("Client Process Read : %d %d %c %s\n",num1,num2,op,str);
        ssize_t server_read_len = read(sock, server_read_buffer,
                                           sizeof(server_read_buffer));
        if (server_read_len <= 0) {
            return 1;
        }
        memcpy(&calculation, server_read_buffer, sizeof(struct cal_data));
        memcpy(&server_time, server_read_buffer + sizeof(struct cal_data), 
                                                        sizeof(struct tm));
        memcpy(&server_socket, server_read_buffer + sizeof(struct cal_data) 
                                           + sizeof(struct tm), sizeof(struct sockaddr_in));
        int left_num = ntohl(calculation.left_num);
        int right_num = ntohl(calculation.right_num);
        int result = ntohl(calculation.result);
        int minV = ntohl(calculation.min);
        int maxV = ntohl(calculation.max);
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(server_socket.sin_addr), ip_str, INET_ADDRSTRLEN);
        char time_buffer[80];
        strftime(time_buffer,sizeof(time_buffer),"%a %b %d %H:%M:%S %Y",&server_time);
        printf("%d%c%d=%d %s min=%d max=%d %s from %s \n", left_num, calculation.op,
                        right_num, result, calculation.str,
                        minV, maxV, time_buffer, ip_str);
    }
    close(sock);
    return 0;
 }
