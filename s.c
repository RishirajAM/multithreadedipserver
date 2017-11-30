#include "csapp.h"

uint32_t fd2;

void echo(int connfd)
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio;
	Rio_readinitb(&rio, connfd);

	fd2 = open("output.jpg", O_WRONLY | O_CREAT , 0666);
	do
	{
		n = read(connfd, buf, 256);
		if(n < 0 || n == 0)
			break;
		n = write(fd2, buf, n);
	}while(1);
	close(fd2);
	if(Fork() == 0)
	{
		printf("in child\n");
		char cgiargs[MAXLINE] = {0};
		char cgiargs2[MAXLINE] = {0};
		getcwd(cgiargs, MAXLINE);
		strncat(cgiargs, "/", 1);
		strncat(cgiargs, "output.jpg", strlen("output.jpg"));
		sprintf(cgiargs2, "%s#%s_output#CTG", cgiargs, cgiargs);
		printf("%s\n", cgiargs2);
		execl("/home/rishiraj/Desktop/fall17/EOS/Project/opencvExample/sample", "sample", cgiargs2, NULL);
		exit(0);
	}
	int status;
    waitpid(-1, &status, 0);
    printf("waitpid returned %d\n", status);
    memset(buf, 0, MAXLINE);
    snprintf(buf, 6, "Hello");
    n = write(connfd, buf, 6);
    printf("n is %d\n", (int)n);
}

int main(int argc, char *argv[])
{
	int listenfd, connfd;
	socklen_t clientlen;
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

		echo(connfd);
		Close(connfd);
	}
	exit(0);
}