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

 #define IP "10.20.0.189"
 #define PORT 3600
 #define MAX_BUFFER 1024

 union semun
 {
        int val;
 };
struct cal_trace {
    int max;
    int min;
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
        struct sockaddr_in client_addr,server_addr;
        int addr_len, n;
        pid_t pid, pid_2, pid_3;
        int listen_fd, client_fd;
        int shmid;
        int semid;
        struct cal_trace *cal_trace_data;
        struct data client_data;
        // Use client shared memory
        time_t current_time;
        struct cal_data *common_cal_data;
        struct tm *common_time;
        struct sockaddr_in *common_socket;
        size_t total_size = sizeof(struct cal_data) + sizeof(struct tm) 
                                                          + sizeof(struct sockaddr_in);
        void *shared_memory = NULL;
        union semun sem_union;
        struct sembuf semopen = {0, -1, SEM_UNDO};
        struct sembuf semclose = {0, 1, SEM_UNDO};
        shmid = shmget((key_t)1234, sizeof(struct cal_trace), 0666|IPC_CREAT);
        if (shmid == -1)
        {
                return 1;
        }
        semid = semget((key_t)5678, 1, IPC_CREAT|0666);
        if(semid == -1)
        {
                return 1;
        }
        shared_memory = shmat(shmid, NULL, 0);
        if (shared_memory == (void *)-1)
        {
                return 1;
        }
        sem_union.val = 1;
        if ( -1 == semctl( semid, 0, SETVAL, sem_union))
        {
                return 1;
        }
        if ((listen_fd = socket(AF_INET,SOCK_STREAM,0)) < 0){
                perror("Error");
                return 1;
        }
        cal_trace_data = (struct cal_trace *)shared_memory;
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
        while(1){
            addr_len = sizeof(client_addr);
            client_fd = accept(listen_fd,(struct sockaddr *)&client_addr, &addr_len);
            if (client_fd == -1 ){
                printf("accept error\n");
                break;
            }
            pid = fork();
            if (pid == 0){
                printf("New Client Connect!\n");
                close(listen_fd);
                // Client unique shared memory
                pid_t client_pid = getpid();
                key_t shm_key = ftok(".", client_pid + client_fd);
                key_t sem_key = ftok(".", client_pid + client_fd + 1000);
                union semun client_sem_union;
                void *client_shared_memory = NULL;
                int client_shm_id = shmget(shm_key, total_size, IPC_CREAT | 0666);
                if (client_shm_id == -1) {
                    perror("shmget failed");
                    close(client_fd);
                    continue;
                }
                int client_sem_id = semget(sem_key, 1, IPC_CREAT | 0666);
                if (client_sem_id == -1) {
                    perror("semget failed");
                    shmctl(client_sem_id, IPC_RMID, NULL);
                    close(client_fd);
                    continue;
                }
                client_shared_memory = shmat(client_sem_id, NULL, 0);
                if (client_shared_memory == (void *)-1)
                {
                        return 1;
                }
                client_sem_union.val = 1;
                if ( -1 == semctl( client_sem_id, 0, SETVAL, client_sem_union))
                {
                        return 1;
                }
                common_cal_data = (struct cal_data *)client_shared_memory;
                common_time = (struct tm *)(client_shared_memory + 
                                                sizeof(struct cal_data));
                common_socket = (struct sockaddr_in *)(client_shared_memory 
                                               + sizeof(struct cal_data) + sizeof(struct tm));
                common_cal_data->op = '#';
                common_socket->sin_family = AF_INET;
                common_socket->sin_port = htons(PORT);
                inet_pton(AF_INET, IP, &(common_socket->sin_addr));
                pid_2 = fork();
                // Client producer
                if (pid_2 == 0){
                    pid_3 = fork();
                    if (pid_3 == 0){
                        // Read Client data
                        while(1){
                            if ( n = read(client_fd,
                                            (void *)&client_data,sizeof(client_data)) <= 0){
                                close(client_fd);
                                return 0;
                            }
                            if (semop(client_sem_id, &semopen, 1) == -1) {
                                perror("semop semid open failed");
                                return 1;
                            }
                            if (semop(semid, &semopen, 1) == -1) {
                                perror("semop semid open failed");
                                return 1;
                            }
                            common_cal_data->left_num = ntohl(client_data.left_num);
                            common_cal_data->right_num = ntohl(client_data.right_num);
                            common_cal_data->op = client_data.op;
                            strcpy(common_cal_data->str,client_data.str);
                            int num1 = common_cal_data->left_num;
                            int num2 = common_cal_data->right_num;
                            char op = common_cal_data->op;
                            if (op == '#'){
                                if (semop(semid, &semclose, 1) == -1) {
                                    perror("semop semid close failed");
                                    return 1;
                                }
                                if (semop(client_sem_id, &semclose, 1) == -1) {
                                    perror("semop semid close failed");
                                    return 1;
                                }
                                continue;
                            }
                            switch(op)
                            {
                                case '+':
                                        common_cal_data->result = num1 + num2;
                                        break;
                                case '-':
                                        common_cal_data->result = num1  - num2;
                                        break;
                                case 'x':
                                        common_cal_data->result = num1 * num2;
                                        break;
                                case '/':
                                        if(num2 == 0)
                                        {
                                                common_cal_data->error = 2;
                                                break;
                                        }
                                        common_cal_data->result = num1 / num2;
                                        break;
                                default:
                                        common_cal_data->error = 1;
                            }
                            if (cal_trace_data->max == 0 && cal_trace_data->min == 0){
                                cal_trace_data->max = common_cal_data->result;
                                cal_trace_data->min = common_cal_data->result;
                            }else {
                                if (common_cal_data->result < cal_trace_data->min) {
                                        cal_trace_data->min = common_cal_data->result;
                                }
                                if (common_cal_data->result > cal_trace_data->max) {
                                        cal_trace_data->max = common_cal_data->result;
                                }
                            }
                            common_cal_data->max = cal_trace_data->max;
                            common_cal_data->min = cal_trace_data->min;
                            if (semop(semid, &semclose, 1) == -1) {
                                perror("semop semid close failed");
                                return 1;
                            }
                            if (semop(client_sem_id, &semclose, 1) == -1) {
                                perror("semop semid close failed");
                                return 1;
                            }
                            if (common_cal_data->left_num == 0 && 
                                  common_cal_data->right_num == 0
                                && common_cal_data->op == '$' && 
                                    strcmp(common_cal_data->str, "quit") == 0 ){
                                exit(0);
                            }
                        }
                    }else{
                        while(1){
                            if (semop(client_sem_id, &semopen, 1) == -1) {
                                perror("semop semid open failed");
                                return 1;
                            }
                            time(&current_time);
                            struct tm *temp_time = localtime(&current_time);
                            memcpy(common_time,temp_time,sizeof(struct tm));
                            if (semop(client_sem_id, &semclose, 1) == -1) {
                                perror("semop semid close failed");
                                return 1;
                            }
                            if (common_cal_data->left_num == 0 && 
                                  common_cal_data->right_num == 0 &&
                                common_cal_data->op == '$' && 
                                 strcmp(common_cal_data->str, "quit") == 0 ){
                                exit(0);
                            }
                            sleep(1);
                        }
                    }
                }else{
                    // Client Consumer
                    while(1){
                        if (semop(client_sem_id, &semopen, 1) == -1) {
                                perror("semop semid open failed");
                                return 1;
                        }
                        char *sendBuffer = malloc(total_size);
                        struct cal_data send_cal_data;
                        memset((void *)&send_cal_data,0x00,sizeof(struct cal_data));
                        send_cal_data.left_num = htonl(common_cal_data->left_num);
                        send_cal_data.right_num = htonl(common_cal_data->right_num);
                        send_cal_data.result = htonl(common_cal_data->result);
                        send_cal_data.max = htonl(common_cal_data->max);
                        send_cal_data.min = htonl(common_cal_data->min);
                        send_cal_data.op = common_cal_data->op;
                        strcpy(send_cal_data.str,common_cal_data->str);
                        memcpy(sendBuffer, &send_cal_data, sizeof(struct cal_data));
                        memcpy(sendBuffer + sizeof(struct cal_data), 
                                         common_time, sizeof(struct tm));
                        memcpy(sendBuffer + sizeof(struct cal_data) + 
                                          sizeof(struct tm), common_socket, sizeof(struct sockaddr_in));
                        ssize_t bytes_written = write(client_fd, sendBuffer, total_size);
                        if (bytes_written == -1) {
                            perror("Write failed");
                        }
                        if (semop(client_sem_id, &semclose, 1) == -1) {
                                perror("semop semid close failed");
                                return 1;
                        }
                        free(sendBuffer);
                        if (common_cal_data->left_num == 0 && 
                             common_cal_data->right_num == 0 &&
                            common_cal_data->op == '$' && 
                             strcmp(common_cal_data->str, "quit") == 0 ){
                            
                            close(client_fd);
                            shmdt(client_shared_memory);
                            semctl(client_sem_id,0,IPC_RMID);
                            exit(0);
                        }
                        sleep(10);
                    }
                }
            }else if (pid  > 0){
                close(client_fd);
            }else{
                perror("Fork failed");
                close(client_fd);
            }
        }
        shmdt(shared_memory);
        semctl(semid,0,IPC_RMID);
        close(client_fd);
        return 0;
 }
