#include "csapp.h"
#include <unistd.h>

int main(int argc, char **argv)
{
	int clientfd;
	char *host, buf[MAXLINE];
	rio_t rio;
	uint32_t ret = 0;
	uint32_t reti = 0;
	uint8_t flag = 1;

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
	printf("Input file is : %s [FD:%d]\n", argv[3], fd);

	uint32_t fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	while (1)
	{
		if(flag)
		{
			ret = read(fd, buf, 256);
			if(ret < 0)
				perror("Error reading the input file fd:");
			if(ret == 0)
			{
				perror("Finished reading the input file fd:");
				flag = 0;
				close(fd);
			}
		}

		reti = write(clientfd, buf, ret);
		if(reti < 0)
		{
			perror("Error sending the image file fd:");
			break;
		}

		if(reti == 0)
		{
			perror("Finished sending the image file fd:");
			break;
		}

		printf("read:%d, write:%d\n", ret, reti);
		usleep(100);
	}
	/*ret = read(clientfd, buf, MAXLINE);
	printf("server sent %d\n", buf);*/

	Close(clientfd);
	exit(0);
}