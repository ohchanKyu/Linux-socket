 #include <sys/socket.h>
 #include <sys/stat.h>
 #include <arpa/inet.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>
 #define MAXBUF 1024
 #define MAX_CLIENTS 3
void replace_newline_with_space(char *str) {
  size_t len = strlen(str);
  if (len > 0 && str[len - 1] == '\n') {
      str[len - 1] = ' ';
  }
}
int main(int argc, char **argv){
  int server_sockfd, client_sockfd[MAX_CLIENTS], client_len, n;
  int connected_clients = 0;
  char buf[MAX_CLIENTS][MAXBUF]; 
  struct sockaddr_in clientaddr, serveraddr;
  client_len = sizeof(clientaddr);
  if ((server_sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
  {
       perror("socket error : ");
      exit(0);
  }
  memset(&serveraddr, 0x00, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(atoi(argv[1]));
  if (bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
      perror("bind error : ");
      exit(0);
  }
  listen(server_sockfd, 3);
  while (connected_clients < MAX_CLIENTS)
  {
      client_sockfd[connected_clients] = 
      accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
      if (client_sockfd[connected_clients] == -1) {
          perror("accept error : ");
          continue;
      }
      printf("New Client[%d] Connect: %s\n", connected_clients, inet_ntoa(clientaddr.sin_addr));
      connected_clients++;
  }
  while(1){
      for (int i = 0; i < MAX_CLIENTS; i++) {
          memset(buf[i], 0x00, MAXBUF);
          if ((n = read(client_sockfd[i], buf[i], MAXBUF)) <= 0)
          {
              close(client_sockfd[i]);
              continue;
          }
          replace_newline_with_space(buf[i]);
          char combined_msg[MAXBUF] = "";
          memset(combined_msg, 0x00, MAXBUF);
          for (int j = 0; j <= i; j++) {
              strcat(combined_msg, buf[j]);
          }
          if (write(client_sockfd[i], combined_msg, strlen(combined_msg)) <= 0) {
              perror("write error : ");
              close(client_sockfd[i]);
          }
          close(client_sockfd[i]);
      }
  }
 
  close(server_sockfd);
  return 0;
}
