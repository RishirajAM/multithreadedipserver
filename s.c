#include "csapp.h"
#include "image_helper.h"

void echo(int connfd)
{
	size_t n;
	char buf[MAXLINE];
	uint32_t fd2;

	n = read(connfd, buf, 15);
	uint32_t fileSize = atoi(buf);
	printf("Server red: %d(%d bytes)\n", fileSize, (int)n);

	fd2 = open("color.jpg", O_WRONLY | O_CREAT , 0666);

	/*Receive an image from client - coloured image*/
	recv_image(fd2, buf, n, connfd, fileSize);
	close(fd2);

	if(Fork() == 0)
	{
		char cgiargs[MAXLINE] = {0};
		char cgiargs2[MAXLINE] = {0};

		getcwd(cgiargs, MAXLINE);
		strncat(cgiargs, "/", 1);
		strncat(cgiargs, "color.jpg", strlen("color.jpg"));
		sprintf(cgiargs2, "%s#%s_BW#CTG", cgiargs, cgiargs);
		execl("/home/rishiraj/Desktop/fall17/EOS/Project/opencvExample/sample", "sample", cgiargs2, NULL);
	}
	int status=0;
    waitpid(-1, &status, 0);
    if(status == 0)
    {
    	memset(buf, 0, MAXLINE);
    	fd2 = open("color.jpg_BW", O_RDONLY);
    	if(fd2 < 0)
    	{
    		perror("Error opening B/W image [Quitting]:");
    	}
    	else
    	{
    		fileSize = lseek(fd2, 0, SEEK_END);
			lseek(fd2, 0, SEEK_SET);

			/*Send BW output file size to the client*/
			sprintf(buf, "%0d", fileSize);
			write(connfd, buf, strlen(buf));
			printf("Server wrote: %s (%d)\n", buf, (int)strlen(buf));

    		/*Send a B/W image to the client*/
    		send_image(fd2, buf, 256, connfd);
    		printf("Closing the B/W image file\n");
    		close(fd2);
    	}
    }
    else
    	printf("waitpid returned %d\n", status);
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