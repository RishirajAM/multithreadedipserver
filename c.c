#include "csapp.h"
#include <unistd.h>

int main(int argc, char **argv)
{
	int clientfd;
	char *host, buf[MAXLINE];
	rio_t rio;
	uint32_t reti = 0;

	if (argc < 4)
	{
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}

	host = argv[1];
	clientfd = Open_clientfd(host, argv[2]);
	Rio_readinitb(&rio, clientfd);

	uint32_t fd = open(argv[3], O_RDONLY);
	if(fd<0)
	{
		perror("Error in opening the input file");
		return fd;
	}

	uint32_t fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	snprintf(buf, 15, "%0d", fileSize);
	reti = write(clientfd, buf, strlen(buf));
	printf("Client sent %s[%d bytes]\n", buf, reti);
	int n = 0;

	/*Send file to server*/
	while((n = read(fd, buf, 256)) > 0)
	{
		reti = write(clientfd, buf, n);
		usleep(500);
	}
	close(fd);

	int fd2 = open("bw_image.jpg", O_WRONLY | O_CREAT , 0666);
	if(fd2 < 0)
	{
		perror("error in creating a file");
		exit(fd2);
	}
	memset(buf, 0, MAXLINE);

	n = read(clientfd, buf, 15);
	fileSize = atoi(buf);
	printf("Client red %s[%d]\n", buf, n);
	memset(buf, 0, MAXLINE);

	/*Receive an image from Sever - B/W image*/
	do
	{
		n = read(clientfd, buf, 256);
		write(fd2, buf, n);
		fileSize -= n;
	}while((n > 0) && fileSize);

	printf("Received the entire image... storing it...\n");
	close(fd2);
	Close(clientfd);
	exit(0);
}