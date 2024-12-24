 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <sys/sem.h>
 #include <string.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <arpa/inet.h>
 #include <sys/stat.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <sys/time.h>
 struct data
 {
        int left_num;
        int right_num;
        char op;
        char str[1024];
        short int error;
 };
 int main(int args,char **argv)
 {
        int shmid;
        int semid;
        struct data *client_data;
        int bytes_read;
        char buffer[1024];
        void *shared_memory = NULL;
        struct sembuf sem_wait = {0,-1,SEM_UNDO};
        struct sembuf sem_signal = {0,1,SEM_UNDO};
        shmid = shmget((key_t)1234,sizeof(struct data),0666);
        if (shmid == -1 ){
                perror("shmget failed : ");
                exit(0);
        }
        semid = semget((key_t)5678, 0 , 0666);
        if (semid == -1)
        {
                perror("semget failed : ");
                return 1;
        }
        shared_memory = shmat(shmid,NULL,0);
        if (shared_memory == (void *)-1)
        {
                perror("shmat failed : ");
                exit(0);
        }
        client_data = (struct data *) shared_memory;
        while(1)
        {
                memset(buffer,0x00,sizeof(buffer));
                bytes_read = read(STDIN_FILENO,buffer,sizeof(buffer)-1);
                if (bytes_read <= 0 ){
                        continue;
                }
                buffer[bytes_read] = '\0';
                int num1,num2;
                char op;
                char str[1024];
                if (sscanf(buffer,"%d %d %c %[^\n]", &num1,&num2,&op,str) != 4 )
                {
                        printf("Invalid input format. Expected : 
                                  num1 num2 op description\n");
                        continue;
                }
                semop(semid,&sem_wait,1);
                client_data->left_num = num1;
                client_data->right_num = num2;
                client_data->op = op;
                strcpy(client_data->str,str);
                if (num1 == 0 && num2 == 0 && op == '$' && strcmp(str, "quit") == 0)
                {
                                printf("Exit Trigger! \n");
                                break;
                }
                printf("Trigger Process Read : %d %d %c %s\n",num1,num2,op,str);
                semop(semid,&sem_signal,1);
        }
        return 0;
 }
