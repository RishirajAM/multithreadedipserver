#include "csapp.h"

uint32_t fd2;

void echo(int connfd)
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio;
	int sum = 0;
	Rio_readinitb(&rio, connfd);
	uint32_t i = 0;

	//while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
	//while((n = Rio_readnb(&rio, buf, MAXLINE)) != 0)
	uint32_t ni = 0;
	fd2 = open("output.jpg", O_WRONLY | O_CREAT , 0666);
	//while((n = read(connfd, buf, 256)) > 0)
	do
	{
		n = read(connfd, buf, 256);
		printf("%d:%d\n", i, n);
		ni = write(fd2, buf, n/*strlen(buf)*/);
		if(n < 0 || n == 0)
			break;

	}while(1);
	printf("out of the while loop!\n");
	close(fd2);
	perror("Closed the file fd1!\n");
}

int main(int argc, char *argv[])
{
	int listenfd, connfd, clientlen;
	struct sockaddr_in clientaddr;
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	listenfd = Open_listenfd(argv[1]);
	while (1)
	{
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		/*fd2 = open("output.jpg", O_WRONLY | O_CREAT , 0666);
		if(fd2 < 0)
			perror("Error in opening output file:");*/
		printf("output.jpg is file %d\n", fd2);
		echo(connfd);
		Close(connfd);
	}
	exit(0);
}