#include "csapp.h"

uint32_t fd2;

void send_image(uint32_t fd, char *buf, size_t len, uint32_t toConnfd)
{
	uint32_t status = 0;
	uint32_t fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	do
	{
		fileSize -= status;
		write(toConnfd, buf, status);
		usleep(500);
	}while(((status = read(fd, buf, len)) > 0) && fileSize);
}

void recv_image(uint32_t fd, char *buf, size_t len, uint32_t fromConnfd, uint32_t fileSize)
{
	uint32_t n = 0;
	do
	{
		n = read(fromConnfd, buf, len);
		write(fd, buf, n);
		fileSize -= n;
	}while((n > 0) && fileSize);
}

void echo(int connfd)
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio;
	Rio_readinitb(&rio, connfd);

	n = read(connfd, buf, 15);
	uint32_t fileSize = atoi(buf);
	printf("Server red: %d(%d bytes)\n", fileSize, (int)n);

	fd2 = open("output.jpg", O_WRONLY | O_CREAT , 0666);
	/*Receive and image from client - coloured image*/
	/*do
	{
		n = read(connfd, buf, 256);
		write(fd2, buf, n);
		fileSize -= n;
	}while((n > 0) && fileSize);*/
	recv_image(fd2, buf, n, connfd, fileSize);
	close(fd2);

	if(Fork() == 0)
	{
		char cgiargs[MAXLINE] = {0};
		char cgiargs2[MAXLINE] = {0};

		getcwd(cgiargs, MAXLINE);
		strncat(cgiargs, "/", 1);
		strncat(cgiargs, "output.jpg", strlen("output.jpg"));
		sprintf(cgiargs2, "%s#%s_output#CTG", cgiargs, cgiargs);
		execl("/home/rishiraj/Desktop/fall17/EOS/Project/opencvExample/sample", "sample", cgiargs2, NULL);
	}
	int status=0;
    waitpid(-1, &status, 0);
    if(status == 0)
    {
    	memset(buf, 0, MAXLINE);
    	fd2 = open("output.jpg_output", O_RDONLY);
    	if(fd2 < 0)
    	{
    		perror("Error opening B/W image [Quitting]:");
    	}
    	else
    	{
    		fileSize = lseek(fd2, 0, SEEK_END);
			lseek(fd2, 0, SEEK_SET);
			sprintf(buf, "%0d", fileSize);
			write(connfd, buf, strlen(buf));
			printf("Server wrote: %s (%d)\n", buf, (int)strlen(buf));

    		/*Send a B/W image to the client*/
    		/*do
    		{
    			fileSize -= status;
    			write(connfd, buf, status);
    			usleep(500);
    		}while(((status = read(fd2, buf, 256)) > 0) && fileSize);*/
    		send_image(fd2, buf, 256, connfd);
    		printf("Closing the B/W image file\n");
    		close(fd2);
    	}
    }
    else
    {
    	printf("waitpid returned %d\n", status);
    }
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