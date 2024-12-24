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
 #include <signal.h>
 union semun
 {
        int val;
 };
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
        int shmid;
        int semid;
        int shmid_2;
        int semid_2;
        time_t current_time;
        struct data *server_data;
        struct cal_data *server_cal_data;
        struct tm *server_time;
        struct sockaddr_in *server_socket;
        void *shared_memory = NULL;
        void *shared_memory_2 = NULL;
        union semun sem_union;
        union semun sem_union_2;
        struct sembuf semopen = {0, -1, SEM_UNDO};
        struct sembuf semclose = {0, 1, SEM_UNDO};
        shmid = shmget((key_t)1234, sizeof(struct data), 0666|IPC_CREAT);
        if (shmid == -1)
        {
                return 1;
        }
        shmid_2 = shmget((key_t)2880, sizeof(struct cal_data) + sizeof(struct tm) +
                                          sizeof(struct sockaddr_in), 0666|IPC_CREAT);
        if (shmid_2 == -1)
        {
                return 1;
        }
        semid = semget((key_t)5678, 1, IPC_CREAT|0666);
        if(semid == -1)
        {
                return 1;
        }
        semid_2 = semget((key_t)9266, 1, IPC_CREAT|0666);
        if(semid_2 == -1)
        {
                return 1;
        }
        shared_memory = shmat(shmid, NULL, 0);
        if (shared_memory == (void *)-1)
        {
                return 1;
        }
        shared_memory_2 = shmat(shmid_2, NULL, 0);
        if (shared_memory_2 == (void *)-1)
        {
                return 1;
        }
        server_data = (struct data *)shared_memory;
        server_cal_data = (struct cal_data *)shared_memory_2;
        server_time = (struct tm *)(shared_memory_2 + sizeof(struct cal_data));
        server_socket = (struct sockaddr_in *)
                          (shared_memory_2 + sizeof(struct cal_data) + sizeof(struct tm));
        const char *ip = "10.20.0.189";
        server_socket->sin_family = AF_INET;
        server_socket->sin_port = htons(8080);
        inet_pton(AF_INET, ip, &(server_socket->sin_addr));
        sem_union.val = 1;
        if ( -1 == semctl( semid, 0, SETVAL, sem_union))
        {
                return 1;
        }
        sem_union_2.val = 1;
        if ( -1 == semctl( semid_2, 0, SETVAL, sem_union_2))
        {
                return 1;
        }
        while(1)
        {
                if(semop(semid, &semopen, 1) == -1)
                {
                        return 1;
                }
                int num1 = server_data->left_num;
                int num2 = server_data->right_num;
                char op = server_data->op;
                char str[1024];
                strcpy(str,server_data->str);
                if (semop(semid, &semclose, 1) == -1) {
                        perror("semop semid close failed");
                        return 1;
                }
                if(semop(semid_2, &semopen, 1) == -1){
                        return 1;
                }
                if (num1 != server_cal_data->left_num || 
                     num2 != server_cal_data->right_num ||
                        op != server_cal_data->op || 
                        strcmp(server_cal_data->str,str) != 0){
                        printf("Producer Process Read : %d %d %c %s\n",num1,num2,op,str);
                }
                server_cal_data->left_num = num1;
                server_cal_data->right_num = num2;
                server_cal_data->op = op;
                strcpy(server_cal_data->str,str);
                if (num1 == 0 && num2 == 0 && op == '$' && strcmp(str, "quit") == 0 ){
                        printf("Exit Producer Program!\n");
                        break;
                }
                time(&current_time);
                struct tm *temp_time = localtime(&current_time);
                memcpy(server_time,temp_time,sizeof(struct tm));
                switch(op)
                {
                        case '+':
                                server_cal_data->result = num1 + num2;
                                break;
                        case '-':
                                server_cal_data->result = num1  - num2;
                                break;
                        case 'x':
                                server_cal_data->result = num1 * num2;
                                break;
                        case '/':
                                if(num2 == 0)
                                {
                                        server_cal_data->error = 2;
                                        break;
                                }
                                server_cal_data->result = num1 / num2;
                                break;
                        default:
                                server_cal_data->error = 1;
                }
                if (server_cal_data->max == 0 && server_cal_data->min == 0){
                        server_cal_data->max = server_cal_data->result;
                        server_cal_data->min=server_cal_data->result;
                } else {
                        if (server_cal_data->result < server_cal_data->min) {
                                server_cal_data->min = server_cal_data->result;
                        }
                        if (server_cal_data->result > server_cal_data->max) {
                                server_cal_data->max = server_cal_data->result;
                        }
                }
                sleep(10);
                if (semop(semid_2, &semclose, 1) == -1) {
                        perror("semop semid close failed");
                        return 1;
                }
        }
        return 0;
 }
