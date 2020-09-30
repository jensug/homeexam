CCFLAGS= -g

all: routing_server node

routing_server: ruting_server.c print_lib/print_lib.c
	gcc ruting_server.c print_lib/print_lib.c $(CCFLAGS) -o routing_server

node: node.c print_lib/print_lib.c
	gcc node.c print_lib/print_lib.c $(CCFLAGS) -o node

colors: colors.c
	$(CC) $(CFLAGS) $^ -o $@



run: all
	bash run_2.sh 1500

clean:
	rm -f routing_server
	rm -f node
