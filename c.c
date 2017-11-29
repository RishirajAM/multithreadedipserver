#include "csapp.h"

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
	uint32_t fd = open(argv[1], O_RDONLY);
	if(fd<0)
	{
		perror("Error in opening the input file");
		return fd;
	}

	uint32_t ret = 0;
	uint32_t fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char *image = calloc(fileSize, sizeof(char));
	if(image == NULL)
		perror("Error allocating a buffer for image:");

	ret = read(fd, image, fileSize);
	if(ret < 0)
		perror("Error reading the input file:");
	printf("%d bytes are read from input image file into image buffer\n", ret);
	/**/

	while (/*Fgets(buf, MAXLINE, stdin) != NULL*/ret)
	{
		printf("before write\n");
		ret = write(clientfd, image, fileSize);
		printf("%d bytes sent.\n", ret);
		//Rio_writen(clientfd, buf, strlen(buf));
		Rio_readlineb(&rio, buf, MAXLINE);
		Fputs(buf, stdout);
	}

	Close(clientfd);
	exit(0);
}