#include "csapp.h"
//#include <unistd.h>
#include "image_helper.h"

int main(int argc, char **argv)
{
	int clientfd;
	char *host, buf[MAXLINE];
	uint32_t reti = 0;

	if (argc < 5)
	{
		fprintf(stderr, "usage: %s <host> <port> <i/p_image_name.jpg> <o/p_image_name.jpg>\n", argv[0]);
		exit(0);
	}

	host = argv[1];
	clientfd = Open_clientfd(host, argv[2]);

	//open input image file
	uint32_t fd = open(argv[3], O_RDONLY);
	if(fd<0)
	{
		perror("Error in opening the input file");
		return fd;
	}

	/*Calculate input file size in bytes*/
	uint32_t fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	/*Send file size*/
	snprintf(buf, 15, "%0d", fileSize);
	reti = write(clientfd, buf, strlen(buf));
	int n = 0;

	/*Send file to server*/
	send_image(fd, buf, 256, clientfd);
	close(fd);

	/*Create an output file*/
	int fd2 = open(argv[4], O_WRONLY | O_CREAT , 0666);
	if(fd2 < 0)
	{
		perror("error in creating a file");
		exit(fd2);
	}
	memset(buf, 0, MAXLINE);

	/*Read the BW image file size*/
	n = read(clientfd, buf, 15);
	fileSize = atoi(buf);
	memset(buf, 0, MAXLINE);

	/*Receive an image from Sever - B/W image*/
	recv_image(fd2, buf, 256, clientfd, fileSize);
	close(fd2);
	Close(clientfd);
	printf("Done.\n");
	exit(0);
}