#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "print_lib/print_lib.h"



unsigned short BASE_PORT;
unsigned int own_addresss;
unsigned int neighbours_count;


int main(int argc, char *argv[]){


  printf("\n");

  BASE_PORT = atoi(argv[1]);
  own_addresss = atoi(argv[2]);
  neighbours_count = 0;

  int neighbour_adresses[(argc-2)*2];
  int neighbour_weights[argc-2];

  while(neighbours_count < argc-3){
    char * argumentet = argv[neighbours_count+3];
    char *token = strtok(argumentet, ":");
    neighbour_adresses[neighbours_count] = atoi(token);
    token = strtok(0, ":");
    neighbour_adresses[neighbours_count+(argc-3)] = atoi(token);
    neighbours_count++;
  }


  printf("Creating node with adress: %d\n", own_addresss );
  printf("With %d neighbours: ", neighbours_count );

  int t;
  for(t = 0; t<neighbours_count; t++){
    printf("%d:%d ", neighbour_adresses[t], neighbour_adresses[t+neighbours_count]);
    }

  printf("\n\n");



// Setting up udp socket
  printf("Setting up UDP socket\n" );
  int udp_sockfd;
  struct sockaddr_in udp_servaddr, udp_cliaddr;

// Creating socket file descriptor
  if ( (udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    // Error check
    perror("UDP-socket setup failed");
    close(udp_sockfd);
    exit(-1);
  }

// Fixing Memory
  memset(&udp_servaddr, 0, sizeof(udp_servaddr));
  memset(&udp_cliaddr, 0, sizeof(udp_cliaddr));

// Filling server information
  udp_servaddr.sin_family    = AF_INET; // IPv4
  udp_servaddr.sin_addr.s_addr = INADDR_ANY;
  udp_servaddr.sin_port = htons(BASE_PORT+own_addresss);

// Binding socket with the server address
  if ( bind(udp_sockfd, (const struct sockaddr *)&udp_servaddr, sizeof(udp_servaddr)) < 0 ){
// Error check
    fprintf( stderr, "Failed to bind UDP\n");
    close(udp_sockfd);
    exit(-1);
  }

// Connecting and sending edges to Tcp
  int ret;
  struct sockaddr_in tcp_server_addr;
  int tcp_server_socket;

  printf("Setting up TCP socket connection\n" );
  tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // Error check
    if (tcp_server_socket == -1) {
      perror("Socket failed");
      close(udp_sockfd);
      close(tcp_server_socket);
      return 0;
    }

  tcp_server_addr.sin_family = AF_INET;
  tcp_server_addr.sin_addr.s_addr = INADDR_ANY;
  tcp_server_addr.sin_port = htons(BASE_PORT);

//Connecting node to port
  printf("Node connecting to port %d\n", BASE_PORT);
  ret = connect(tcp_server_socket, (struct sockaddr *)&tcp_server_addr, sizeof(struct sockaddr_in));

// Error check
    if (ret == -1) {
      perror("Failed to connect to TCP");
      close(udp_sockfd);
      close(tcp_server_socket);
      exit(-2);
    }

  printf("Node sending to server\n");
// Sending to server
  ssize_t write_ret = write(tcp_server_socket, &own_addresss, sizeof(own_addresss));
  write_ret = write(tcp_server_socket, &neighbours_count, sizeof(neighbours_count));
  write_ret = write(tcp_server_socket, &neighbour_adresses, sizeof(neighbour_adresses));

// Error check
    if (write_ret == -1) {
      perror("Write failed");
      close(udp_sockfd);
      close(tcp_server_socket);
      return 0;
    }

  printf("Data sent\n");
  printf("\n");

  int *return_table_return;
  int return_size;

// Reading number of returns
  ssize_t read_ret = read(tcp_server_socket, &return_size, sizeof(int));

  if(return_size != 0){

    printf("Router %d should recieve %d routs: ", own_addresss, return_size );
    (return_table_return) = (int*)malloc(sizeof(int)*return_size*2);

    for(int t = 0; t<return_size*2; t++){
      read_ret = read(tcp_server_socket, &return_table_return[t], sizeof(int));
    }

    if (read_ret == -1) {
      close(udp_sockfd);
      close(tcp_server_socket);
      perror("Failed to read");
      return 0;
    }

    for(int l=0; l<return_size;l++){
      printf("%d:%d ",return_table_return[(l*2)], return_table_return[(1+(l*2))] );
    }

    printf("\n");

  }else{
// No routes recieved
    printf("Router %d did not recieve any routes\n\n", own_addresss );
  }

// Closing TCP socket
  close(tcp_server_socket);


// Node 1 operation
  if(own_addresss == 1){
    printf("Node 1 waiting..\n");
    sleep(1);

// Getting file
    FILE * fil = fopen("data.txt", "r");

    unsigned short send_adress;
    char string[1000];
    unsigned short string_length;
    unsigned short own_addresss_2 = own_addresss;
    char end = '\0';

// Reading file
    while(!feof(fil)){

      fscanf(fil, "%hd", &send_adress);
      fscanf(fil, " ");
      fgets(string, 1000, fil);
      string[strcspn(string, "\n")] = 0;
      string_length = strlen(string);

// Converting message
      unsigned char packet[1007];
      unsigned short len2 = sizeof(string_length) + sizeof(send_adress) + sizeof(own_addresss_2) + string_length + sizeof(char);
      unsigned short packetlen = htons(len2);
      unsigned short packetdest = htons(send_adress);
      unsigned short packetsrc = htons(own_addresss_2);

      memcpy(packet, &packetlen, sizeof(unsigned short));
      memcpy(packet+2, &packetdest, sizeof(unsigned short));
      memcpy(packet+4, &packetsrc, sizeof(unsigned short));
      memcpy(packet+6, &string, sizeof(string));

      print_pkt(packet);

      char *msg = (char*)&packet[6];
      int next = -1;
      if(send_adress == own_addresss){
        print_received_pkt(own_addresss_2,packet);

// Quit
        if(strcmp(msg,"QUIT")==0){
          printf("Ruter %d terminated\n", own_addresss);
          free(return_table_return);
          close(udp_sockfd);
          exit(0);
        }

      }

      print_forwarded_pkt(own_addresss_2, packet);

      for(int l=0; l<return_size;l++){

        if(send_adress == return_table_return[(1+(l*2))]){
          next = return_table_return[((l*2))];
        }
      }

      if(next == -1){
// Error check
        printf("Failed to find fastest way to %d to send through %d\n", send_adress, next  );
        free(return_table_return);
        close(udp_sockfd);
        exit(0);
      }

      printf("Sending packet to %d through %d\n",send_adress, next );

      udp_servaddr.sin_port = htons(BASE_PORT + next);
      unsigned short len = ntohs(*(short *)&packet[0]);
      sendto(udp_sockfd, (const char *)packet, len2, 0, (const struct sockaddr *)&udp_servaddr, sizeof(udp_servaddr));
    }

  }else{
    unsigned int n, len;
    unsigned char recieved_message[1007];

    while(1){

      socklen_t cli_size = sizeof(udp_cliaddr);
      n = recvfrom(udp_sockfd, (unsigned char *)recieved_message, 1007, 0, ( struct sockaddr *)&udp_cliaddr, &cli_size);

      unsigned short dest = htons(*(short*)&recieved_message[2]);

      printf("Recieved message for %d\n", dest);
      char *msg = (char*)&recieved_message[6];

      if(dest == own_addresss){
        print_received_pkt(own_addresss,recieved_message);

        if(strcmp(msg,"QUIT")==0){
          printf("Ruter %d terminated\n", own_addresss);

          if(return_size != 0){
            free(return_table_return);
          }

          close(udp_sockfd);
          exit(0);
        }
      }else{
        print_forwarded_pkt(own_addresss,recieved_message);
        int next = -1;

        for(int l=0; l<return_size;l++){
          if(dest == return_table_return[(1+(l*2))]){
          next = return_table_return[((l*2))];
          }
        }

        if(next == -1){
          printf("Failed to find fastest way to %d to send through %d\n", dest, next  );
        }

        printf("Sending packet to %d through %d\n",dest, next );

        udp_servaddr.sin_port = htons(BASE_PORT + next);
        unsigned short len = ntohs(*(short *)&recieved_message[0]);
        sendto(udp_sockfd, (const char *)recieved_message, len, 0, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
      }

    }

  }


  return 0;
}
