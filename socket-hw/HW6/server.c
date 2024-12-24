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
#include <pthread.h>

#define IP "10.20.0.189"
#define PORT 3600
#define MAX_BUFFER 1024

pthread_mutex_t t_lock;
pthread_cond_t t_cond;
int global_max = 0;
int global_min = 0;

void *producer(void *arg);
void *consumer(void *arg);

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
 struct shared_data
 {
    int client_fd;
    struct cal_data server_cal_data;
    struct sockaddr_in server_socket;
    struct tm server_time;
 };
 void * handle_client(void *client_fd){
    printf("New Client Connect!\n");
    int sockfd = *((int *)client_fd);
    struct shared_data *data = malloc(sizeof(struct shared_data));
    if (data == NULL) {
        perror("Malloc failed");
        return NULL;
    }
    struct data client_data;
    pthread_t producer_id;
    pthread_t consumer_id;
    data->client_fd = sockfd;
    data->server_socket.sin_family = AF_INET;
    data->server_socket.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &(data->server_socket.sin_addr));
    int readn;
    pthread_create(&producer_id, NULL, producer, (void *)data);
    pthread_create(&consumer_id, NULL, consumer, (void *)data);
    while((readn = read(sockfd, &client_data, sizeof(client_data))) > 0)
    {
        pthread_mutex_lock(&t_lock);
        data->server_cal_data.left_num = ntohl(client_data.left_num);
        data->server_cal_data.right_num = ntohl(client_data.right_num);
        data->server_cal_data.op = client_data.op;
        strcpy(data->server_cal_data.str,client_data.str);
        int num1 = data->server_cal_data.left_num;
        int num2 = data->server_cal_data.right_num;
        char op =  data->server_cal_data.op;
        if (op == '#'){
            continue;
        }
        switch(op)
        {
            case '+':
                    data->server_cal_data.result = num1 + num2;
                    break;
            case '-':
                    data->server_cal_data.result = num1  - num2;
                    break;
            case 'x':
                    data->server_cal_data.result = num1 * num2;
                    break;
            case '/':
                    if(num2 == 0)
                    {
                            data->server_cal_data.error = 2;
                            break;
                    }
                    data->server_cal_data.result = num1 / num2;
                    break;
            default:
                    data->server_cal_data.error = 1;
        }
        if (global_max == 0 && global_min == 0){
            global_max = data->server_cal_data.result;
            global_min = data->server_cal_data.result;
        }else {
            if (data->server_cal_data.result < global_min) {
                    global_min = data->server_cal_data.result;
            }
            if (data->server_cal_data.result > global_max) {
                    global_max = data->server_cal_data.result;
            }
        }
        data->server_cal_data.max = global_max;
        data->server_cal_data.min = global_min;
        pthread_mutex_unlock(&t_lock);
        if (data->server_cal_data.left_num == 0 && data->server_cal_data.right_num == 0
            && data->server_cal_data.op == '$' && strcmp(data->server_cal_data.str,
               "quit") == 0 ){
            pthread_join(producer_id,NULL);
            pthread_join(consumer_id,NULL);
            printf("Exit main worker thread!");
            free(data);
            return NULL;
        }
    }
 }
 void * producer(void *arg){
    struct shared_data *data = (struct shared_data *)arg;
    time_t current_time;
    while(1){
        pthread_mutex_lock(&t_lock);
        time(&current_time);
        struct tm *temp_time = localtime(&current_time);
        memcpy(&(data->server_time),temp_time,sizeof(struct tm));
        pthread_mutex_unlock(&t_lock);
       if (data->server_cal_data.left_num == 0 && data->server_cal_data.right_num == 0 &&
     data->server_cal_data.op == '$' && strcmp(data->server_cal_data.str, "quit") == 0 ){
            printf("Exit Client producer");
            return NULL;
        }
        sleep(1);
    }
 }
 void * consumer(void *arg){
    struct shared_data *data = (struct shared_data *)arg;
    int client_fd = data->client_fd;
    size_t total_size = sizeof(struct cal_data) + sizeof(struct tm) + 
                                                        sizeof(struct sockaddr_in);
    while(1){
        pthread_mutex_lock(&t_lock);
        char *sendBuffer = malloc(total_size);
        struct cal_data send_cal_data;
        memset((void *)&send_cal_data,0x00,sizeof(struct cal_data));
        send_cal_data.left_num = htonl(data->server_cal_data.left_num);
        send_cal_data.right_num = htonl(data->server_cal_data.right_num);
        send_cal_data.result = htonl(data->server_cal_data.result);
        send_cal_data.max = htonl(data->server_cal_data.max);
        send_cal_data.min = htonl(data->server_cal_data.min);
        send_cal_data.op = data->server_cal_data.op;
        strcpy(send_cal_data.str,data->server_cal_data.str);
        memcpy(sendBuffer, &send_cal_data, sizeof(struct cal_data));
        memcpy(sendBuffer + sizeof(struct cal_data), &(data->server_time), 
                                                           sizeof(struct tm));
        memcpy(sendBuffer + sizeof(struct cal_data) + sizeof(struct tm),  
                               &(data->server_socket), sizeof(struct sockaddr_in));
        ssize_t bytes_written = write(client_fd, sendBuffer, total_size);
        if (bytes_written == -1) {
            perror("Write failed");
        }
        pthread_mutex_unlock(&t_lock);
        free(sendBuffer);
        if (data->server_cal_data.left_num == 0 && data->server_cal_data.right_num == 0
         && data->server_cal_data.op == '$' && 
            strcmp(data->server_cal_data.str, "quit") == 0 ){
            printf("Exit Client consumer");
            close(client_fd);
            return NULL;
      }
        sleep(10);
    }
 }
 int main(int argc, char **argv)
 {
    struct sockaddr_in client_addr,server_addr;
    int addr_len;
    int listen_fd, client_fd;
    pthread_t thread_id;
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
    pthread_mutex_init(&t_lock,NULL);
    while(1)
    {
        addr_len = sizeof(client_addr);
        client_fd = accept(listen_fd,(struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1 ){
            printf("accept error\n");
                break;
        }else{
            pthread_create(&thread_id, NULL, handle_client, (void *)&client_fd);
            pthread_detach(thread_id);
        }
    }
    close(listen_fd);
    return 0;
 }
