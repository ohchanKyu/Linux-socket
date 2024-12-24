#include <sys/ipc.h>
 #include <sys/shm.h>
 #include <sys/sem.h>
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
 int main(int args,char **argv)
 {
        int shmid;
        int semid;
        time_t current_time;
        struct cal_data *server_cal_data;
        struct tm *server_time;
        struct sockaddr_in *server_socket;
        void *shared_memory = NULL;
        struct sembuf sem_wait = {0,-1,SEM_UNDO};
        struct sembuf sem_signal = {0,1,SEM_UNDO};
        shmid = shmget((key_t)2880,sizeof(struct cal_data)+
                 sizeof(struct tm)+sizeof(struct sockaddr_in),0666);
        if (shmid == -1 ){
                perror("shmget failed : ");
                return 1;
        }
        semid = semget((key_t)9266,0,0666);
        if (semid == -1){
                perror("semget failed : ");
                return 1;
        }
        shared_memory = shmat(shmid,NULL,0);
        if (shared_memory == (void *)-1){
                perror("shmat failed : ");
                return 1;
        }
        server_cal_data = (struct cal_data *)shared_memory;
        server_time = (struct tm *)(shared_memory + sizeof(struct cal_data));
        server_socket = (struct sockaddr_in *)
                        (shared_memory + sizeof(struct cal_data) + sizeof(struct tm));
        while(1){
            semop(semid, &sem_wait, 1);
            if (server_cal_data->left_num == 0 && server_cal_data->right_num == 0 &&
                        server_cal_data->op == '$' && 
                        strcmp(server_cal_data->str, "quit") == 0 ){
                        printf("Exit Consumer Program!\n");
                        break;
            }
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(server_socket->sin_addr), ip_str, INET_ADDRSTRLEN);
            char time_buffer[80];
            strftime(time_buffer,sizeof(time_buffer),"%a %b %d %H:%M:%S %Y",server_time);
            printf("%d%c%d=%d %s min=%d max=%d %s from %s \n", 
                          server_cal_data->left_num, server_cal_data->op,
                          server_cal_data->right_num, 
                          server_cal_data->result, server_cal_data->str,
                        server_cal_data->min, server_cal_data->max, time_buffer, ip_str);
             semop(semid,&sem_signal,1);
        }
 }
