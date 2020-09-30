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
#include <limits.h>


#include "print_lib/print_lib.h"

int print_nodes(int** node_adresses, int** node_neighbours, int nodes){
// Printer alle noder:
      printf("\n");
  int print_index_1;
  for(print_index_1 = 0; print_index_1<nodes; print_index_1++){
    int print_neighbours = node_adresses[print_index_1][1];
    printf("Neighbours with adresss %d has %d neighbours\n", node_adresses[print_index_1][0], print_neighbours );
    int print_index_2;
    printf("Neighbours are: " );
    for(print_index_2 = 0; print_index_2<print_neighbours; print_index_2++){
      printf("%d:%d ", node_neighbours[print_index_1][print_index_2], node_neighbours[print_index_1][print_index_2+print_neighbours]);
    }
    printf("\n");
  }
  printf("\n");
  return 1;
}


int find_position_of_adress(int** node_adresses, int** node_neighbours, int nodes, int adress){
  int position;
  for(position = 0; position<nodes;position++){
    if(node_adresses[position][0] == adress){
      break;
    }
    if(position == nodes-1){
      perror("Adress does not exist");
      exit(EXIT_FAILURE);
    }
  }
  return position;
}


int co_responding_check(int** node_adresses, int** node_neighbours, int nodes){

// Check that edges go both ways
  printf("Checking co-respondence\n");
  int error_count = 0;

  int check_index_1;
  for(check_index_1 = 0; check_index_1<nodes; check_index_1++){
    int check_neighbours = node_adresses[check_index_1][1];
    int original_node_adress =  node_adresses[check_index_1][0];
    int check_index_2;
    for(check_index_2 = 0; check_index_2<check_neighbours; check_index_2++){
// Check node adress
      int check_neighbour_node = node_neighbours[check_index_1][check_index_2];
      int check_neighbour_node_weight =node_neighbours[check_index_1][check_index_2+check_neighbours];
      int neighbour_neighbour_index = find_position_of_adress(node_adresses, node_neighbours, nodes, check_neighbour_node);

// Checking that the neighbour has the orignial node as neighbour
      int adress_neighbour_count = node_adresses[neighbour_neighbour_index][1];
      int adress_neighbour_index;
      for(adress_neighbour_index=0; adress_neighbour_index<adress_neighbour_count; adress_neighbour_index++){
        if(node_neighbours[neighbour_neighbour_index][adress_neighbour_index]==original_node_adress){
// Found node, check wieght
          if(node_neighbours[neighbour_neighbour_index][adress_neighbour_index+adress_neighbour_count] != check_neighbour_node_weight){
            perror("Neighbour weight does not co-respond");
            error_count = 1;
          }
          if(error_count == 1){
            printf("WARNING: Edge does not co-respond, ignoring ruter %d edge %d\n", original_node_adress, node_neighbours[neighbour_neighbour_index][adress_neighbour_index]);

            break;
          }

        }
      }
      if(error_count == 0){
          perror("Neighbour does not co-respond");
      }
    }
  }
  printf("All neighbours are co-responding\n" );
  return 1;
}


// Dijekstra algorythm
void dijkstra(int** node_adresses, int** node_neighbours, int nodes, int start_adress, int* dist, int** path){

  int start_potition = find_position_of_adress(node_adresses, node_neighbours, nodes, start_adress);

  int * visited = (int*)malloc(sizeof(int)*nodes);
  int * prev = (int*)malloc(sizeof(int)*nodes);
  int amount_visited;
  int last_visited;

  for(int i = 0; i<nodes; i++){
    dist[i] = INT_MAX;
    visited[i] = 0;
    prev[i] = -1;
  }
  prev[0] = 0;
  dist[start_potition] = 0;


  printf("\nDijekstra:\n");

  int current_position = start_potition;
  for(amount_visited = 0; amount_visited<nodes; amount_visited++){
    printf("Going through loop from %d\n", node_adresses[current_position][0]);
    int edges = node_adresses[current_position][1];
// Going through edges
    int min_edge_position = -1;
    int min_edge_position_value = 0;
    int edges_visited = 0;

    for(int u = 0; u<edges; u++){

      printf("Finding position for current edge %d\n", node_neighbours[current_position][u]);
        int edge_position = find_position_of_adress(node_adresses, node_neighbours, nodes, node_neighbours[current_position][u]);
          if(node_neighbours[current_position][u+edges]+dist[current_position] < dist[edge_position]){
            printf("New distance for %d is %d\n", node_adresses[edge_position][0] , (node_neighbours[current_position][u+edges]+dist[current_position]));
            dist[edge_position] = node_neighbours[current_position][u+edges]+dist[current_position];
            printf("New previous for %d is %d\n",node_adresses[edge_position][0], node_adresses[current_position][0]);
            prev[edge_position] = current_position;
            visited[edge_position] = 0;
            amount_visited--;
          }
          if(visited[edge_position] != 1){
            if(min_edge_position == -1){
              printf("Finding first smallest of edges %d\n", node_neighbours[current_position][u]);
              min_edge_position = find_position_of_adress(node_adresses, node_neighbours, nodes, node_neighbours[current_position][u]);
              min_edge_position_value = node_neighbours[current_position][u+edges];
            }else if(node_neighbours[current_position][u+edges] < min_edge_position_value){
              printf("Finding smallest of edges %d\n", node_neighbours[current_position][u]);
              min_edge_position = find_position_of_adress(node_adresses, node_neighbours, nodes, node_neighbours[current_position][u]);
              min_edge_position_value = node_neighbours[current_position][u+edges];
            }
          }else{
            edges_visited++;
          }
    }

    if(amount_visited == nodes-1){
      printf("All nodes visited\n");
      break;
    }
      printf("Changing current position from %d to %d\n",node_adresses[current_position][0],  node_adresses[min_edge_position][0]);
      visited[current_position] = 1;
      amount_visited += 1;
      last_visited = current_position;
      current_position = min_edge_position;
  }
  printf("\n");

// Setting up path for ruters from ruter 1
// Paths come in the wrong order because i use the previous list to find it
  printf("Setting up paths for nodes\n");
  for(int t = 0; t < nodes; t++){
    path[t] = (int*)malloc(sizeof(int)*nodes);
    for(int v = 0; v < nodes; v++){
      path[t][v] = 0;
    }
    int path_index = 0;
    int current_prev = find_position_of_adress(node_adresses, node_neighbours, nodes, node_adresses[t][0]);
    while(current_prev != prev[current_prev]){
      path[t][path_index] = node_adresses[prev[current_prev]][0];
      path_index++;
      current_prev = prev[current_prev];
    }
  }

// Printing
  printf("\nShortest paths from %d\n", start_adress);
  for(int t = 0; t<nodes; t++){
    printf("to %d is %d with path: ", node_adresses[t][0], dist[t]);
      int path_step = 0;
      while(path[t][path_step] != 0){
        printf("%d, ", path[t][path_step]);
        path_step++;
      }
      printf("\n" );
  }


  free(visited);
  free(prev);
}


// Setting up tcp server
void tcp_setup(int** node_adresses, int** node_neighbours, int nodes, int TCP_PORT){

  int ret;
  int yes;
  int master_socket = 1;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;
  fd_set readfds;

  int* client_socket = (int*)malloc(sizeof(int)*nodes);
  for(int c = 0; c<nodes; c++){
    client_socket[c] = 0;
  }


  printf("\n");

  printf("Creating TCP socket at port:%d\n", TCP_PORT);
// Creating a socket
  master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (master_socket == -1) {
      perror("socket");
      exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET; // IPv4 address
  server_addr.sin_port = htons(TCP_PORT); //Tcp port fra argv
  server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from anyone.


// Makes it so that you can reuse port immediately after previous user
  int stsock_ret = setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (stsock_ret == -1) {
      perror("stssockopt");
      exit(EXIT_FAILURE);
  }

  ret = bind(master_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (ret) {
      perror("bind");
      exit(EXIT_FAILURE);
  }

// Listening for nodes
    printf("Server listening for nodes..\n");
    ret = listen(master_socket, nodes);
      if (ret == -1) {
          perror("listen");
          exit(EXIT_FAILURE);
      }


    int read_count = 0;
    while(read_count<nodes){

// Setting fds for select
      FD_ZERO(&readfds);
      FD_SET(master_socket, &readfds);
      FD_SET(client_socket[read_count], &readfds);
      if(FD_ISSET(master_socket, &readfds)){


        printf("Waiting for nodes\n");
        ret = select(master_socket+1, &readfds, NULL, NULL, NULL);

// Accepting node
        socklen_t addrlen = sizeof(client_addr);
        client_socket[read_count] = accept(master_socket,(struct sockaddr*)&client_addr, &addrlen);
        printf("Server recieving from node\n");

        int neighbours_count;
        (node_adresses)[read_count] =(int*)malloc(sizeof(int)*2);

// Reading adress
        int adressen;
        ssize_t read_ret = read(client_socket[read_count], &adressen, sizeof(int));
        (node_adresses)[read_count][0] = adressen;

// Reading amount of neighbours
        read_ret = read(client_socket[read_count], &neighbours_count, sizeof(int));
        (node_adresses)[read_count][1] = neighbours_count;

// Saving nodes
        int node_neighbour_read[neighbours_count*2];
        (node_neighbours)[read_count] = (int*)malloc(sizeof(int)*neighbours_count*2);
        read_ret = read(client_socket[read_count], &node_neighbour_read, sizeof(int)*neighbours_count*2);

// Saving neighbours
        int add_index;
        for(add_index = 0; add_index<neighbours_count*2; add_index++){
        (node_neighbours)[read_count][add_index] = node_neighbour_read[add_index];
        }

        if (ret == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
      }
      if(ret == -1){
        perror("select");
        exit(EXIT_FAILURE);
      }
      read_count++;
  }

  print_nodes(node_adresses, node_neighbours, nodes);

  co_responding_check(node_adresses,node_neighbours, nodes);


    int * dist = (int*)malloc(sizeof(int)*nodes);
    int ** path = (int**)malloc(sizeof(int*)*nodes);


  dijkstra(node_adresses, node_neighbours, nodes, 1, dist, path);

  for(int t = 0; t<nodes; t++){
    for(int p = 0; p<nodes; p++){
      int pathlens = 0;
      if(t == p){
        pathlens = 0;
      }else if(node_adresses[t][0] == 1){
        pathlens = dist[p];
      }else{
        pathlens= dist[p]-dist[t];
      }
      print_weighted_edge(node_adresses[t][0], node_adresses[p][0], pathlens);
    }
  }

// Creating list of path directions for all nodes(destination, next neighbour, destination, next neigbours...)
  int* path_amount = (int*)malloc(sizeof(int)*nodes);
  int** ruter_return_tables = (int**)malloc(sizeof(int*)*nodes);

  for(int t = 0; t<nodes; t++){
    int path_step = 0;
    while(path[t][path_step] != 0){
      int path_position = find_position_of_adress(node_adresses, node_neighbours, nodes, path[t][path_step]);
      path_amount[path_position]++;
      path_step++;
    }
  }

  for(int f = 0; f<nodes; f++){
    ruter_return_tables[f] = (int*)malloc(sizeof(int)*path_amount[f]*2);
    path_amount[f] = 0;
  }


  for(int t = 0; t<nodes; t++){
    int path_step = 0;
    while(path[t][path_step] != 0){
      int path_position = find_position_of_adress(node_adresses, node_neighbours, nodes, path[t][path_step]);
      if(path_step == 0){
        ruter_return_tables[path_position][path_amount[path_position]*2] = node_adresses[t][0];
      }else{
        ruter_return_tables[path_position][path_amount[path_position]*2] = path[t][path_step-1];
      }
      ruter_return_tables[path_position][1+(path_amount[path_position]*2)] = node_adresses[t][0];
      path_amount[path_position]++;
      path_step++;
    }
  }

  printf("\n" );
    for(int s=0; s<nodes; s++){
      printf("Router %d should recieve: ", node_adresses[s][0] );
      for(int l=0; l<path_amount[s];l++){
        printf("%d:%d ",ruter_return_tables[s][(l*2)], ruter_return_tables[s][(1+(l*2))] );
      }
      printf("\n");
    }



// Sending back paths

  for(int f = 0; f<nodes; f++){

    printf("Server sending back to node\n");

      ssize_t write_ret = write(client_socket[f], &path_amount[f], sizeof(int));
      if(path_amount[f] != 0){
// Will not read int pointer so sending one by one
        for(int t = 0; t<path_amount[f]*2; t++){
          write_ret = write(client_socket[f], &ruter_return_tables[f][t], sizeof(int));
        }
      }
      if (write_ret == -1) {
          perror("write");
          return;
      }
    printf("Info sent\n");
  }



  printf("\nClosing socket..\n");
  close(master_socket);

  for(int free_index = 0; free_index<nodes; free_index++){
    free(path[free_index]);
  }

// Freeing
  free(path);
  free(dist);
  free(client_socket);

}


int main(int argc, char const *argv[]) {

// Arguments info
  int TCP_PORT = atoi(argv[1]);
  int nodes = atoi(argv[2]);

//Nodes declaration

  int ** node_adresses = (int**)malloc(sizeof(int*)*nodes);
  int ** node_neighbours = (int**)malloc(sizeof(int*)*nodes);

// Setting up TCP
  tcp_setup(node_adresses, node_neighbours, nodes, TCP_PORT);

// Freeing
  for(int free_index = 0; free_index<nodes; free_index++){
    free(node_adresses[free_index]);
    free(node_neighbours[free_index]);
  }

  free(node_adresses);
  free(node_neighbours);


  printf("\n");

  return 0;
}
