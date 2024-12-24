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
 #define IP "127.0.0.1"
 struct cal_data
 {
    int left_num;
    int right_num;
    char op;
    int result;
    int max;
    int min;
    char description[1024];
    short int error;
 };
 int main(int argc, char **argv)
 {
    struct sockaddr_in addr;
    int s;
    ssize_t len = sizeof(struct cal_data);
    int sbyte, rbyte;
    struct cal_data sdata;
    char buffer[1024];
    int bytes_read;
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
    {
         return 1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if ( connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1 )
    {
         printf("fail to connect\n");
         close(s);
         return 1;
    }
    memset((void *)&sdata, 0x00, sizeof(sdata));
    memset(buffer, 0x00, sizeof(buffer));
    bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        perror("Failed to read input");
        return 1;
    }
    buffer[bytes_read] = '\0';
    int num1, num2;
    char op;
    char description[1024];
    if (sscanf(buffer, "%d %d %c %[^\n]", &num1, &num2, &op, description) != 4) {
        printf("Invalid input format. Expected: num1 num2 op description\n");
        return 1;
    }
    sdata.left_num = htonl(num1);
    sdata.right_num = htonl(num2);
    sdata.op = op;
    strncpy(sdata.description, description, sizeof(sdata.description) - 1);
    sdata.description[sizeof(sdata.description) - 1] = '\0';
    sbyte = write(s, (char *)&sdata, len);
    if(sbyte != len)
    {
         return 1;
    }
    rbyte = read(s, (char *)&sdata, len);
    if(rbyte != len)
    {
         return 1;
    }
    if (ntohs(sdata.error != 0))
    {
         printf("CALC Error %d\n", ntohs(sdata.error));
    }
    printf("%d %c %d = %d min=%d max=%d str=%s\n",
                    ntohl(sdata.left_num), sdata.op, ntohl(sdata.right_num),
                    ntohl(sdata.result),
                    ntohl(sdata.min),ntohl(sdata.max),sdata.description);
    close(s);
    return 0;
 }
