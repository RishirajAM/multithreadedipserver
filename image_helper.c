#include "image_helper.h"

void send_image(uint32_t fd, char *buf, size_t len, uint32_t toConnfd)
{
	uint32_t status = 0;
	uint32_t fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	do
	{
		fileSize -= status;
		//printf("reamaining: %d\n", fileSize);
		write(toConnfd, buf, status);

		usleep(500);
	}while(((status = read(fd, buf, len)) > 0) && fileSize);
}

void recv_image(uint32_t fd, char *buf, size_t len, uint32_t fromConnfd, uint32_t fileSize)
{
	uint32_t n = 0;
	do
	{
		n = read(fromConnfd, buf, 256);
		write(fd, buf, n);
		fileSize -= n;
		/*if(n == 0)
			printf("n is 0\n");*/
		//printf("remaining: %d\n", fileSize);
	}while((n > 0) && fileSize);
}

uint32_t readImageFileSize(uint32_t fromConnfd)
{
	uint32_t fileSize;
	int ret = read(fromConnfd, &fileSize, sizeof(fileSize));
	if(ret > 0)
	{
		uint32_t x = ntohl(fileSize);
		printf("FileSize is %d bytes.\n", x);
		return x;
	}
	else
	{
		printf("Received %d bytes of fileSize.\n", ret);
		return ret;
	}

}

void writeImageFileSize(uint32_t toConnfd, uint32_t fileSize)
{
	uint32_t fs = htonl(fileSize);
	uint32_t ret = write(toConnfd, &fs, sizeof(fs));
	printf("Sent %d bytes of fileSize.\n", ret);
}