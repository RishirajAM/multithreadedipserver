#include "csapp.h"
#include <unistd.h>

int main(int argc, char **argv)
{
	int clientfd;//, port;
	char *host, buf[MAXLINE];
	rio_t rio;

	if (argc != 4)
	{
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}

	host = argv[1];
	clientfd = Open_clientfd(host, argv[2]);
	Rio_readinitb(&rio, clientfd);

	/**/

	uint32_t fd = open(argv[3], O_RDONLY);
	if(fd<0)
	{
		perror("Error in opening the input file");
		return fd;
	}
	printf("Input file is : %s [FD:%d]\n", argv[3], fd);

	uint32_t ret = 0;
	uint32_t reti = 0;

	do
	{
		ret = read(fd, buf, 256);
		if(ret < 0)
			perror("Error reading the input file fd:");
		if(ret == 0)
		{
			perror("Finished reading the input file fd:");
			close(fd);
		}

		reti = write(clientfd, buf, 256);
		if(reti < 0)
			perror("Error sending the image file fd:");
		if(reti == 0)
		{
			perror("Finished sending the image file fd:");
		}
		usleep(10000);
	}while (reti);

	sleep(3);
	Close(clientfd);
	exit(0);
}