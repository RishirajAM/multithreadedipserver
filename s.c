#include "csapp.h"
#include "image_helper.h"
#include "q.h"
#include <unistd.h>
#include <sys/syscall.h>

#define ACTIVE_CORES ((sysconf(_SC_NPROCESSORS_ONLN)==1)?   \
                     (1):                                   \
                     (sysconf(_SC_NPROCESSORS_ONLN) - 1)    )

#define QLEN ((ACTIVE_CORES)*(2))

queue *fifo;

void echo(int connfd)
{
	size_t n = 0;
	uint32_t fd2;
	uint32_t fileSize;
	char buf[MAXLINE];
	char ipImage[15] = {0};
	char opImage[15] = {0};
	char imageTypeBuf[10] = {0};
	enum fileType ipFileType;

	fileSize = readImageFileSize(connfd);
	ipFileType = readImageFileType(connfd);

	if(ipFileType == jpg)
		sprintf(imageTypeBuf, "jpg");
	if(ipFileType == jpeg)
		sprintf(imageTypeBuf, "jpeg");
	if(ipFileType == png)
		sprintf(imageTypeBuf, "png");
	if(ipFileType == gif)
		sprintf(imageTypeBuf, "gif");

	sprintf(ipImage, "%05d_ip.%s", (int)syscall(__NR_gettid), imageTypeBuf);
	sprintf(opImage, "%05d_op.%s", (int)syscall(__NR_gettid), imageTypeBuf);

	fd2 = open(ipImage, /*O_WRONLY*/ O_RDWR | O_CREAT , 0666);
	if(fd2 < 0)
	{
		printf("%s:%d:", __func__, __LINE__);
		perror("Invalid input file:");
		exit(fd2);
	}

	/*Receive an image from client - coloured image*/
	recv_image(fd2, buf, 256, connfd, fileSize);

	Close(fd2);

	if(Fork() == 0)
	{
		char cgiargs[MAXLINE] = {0};
		char cgiargs2[MAXLINE] = {0};
		sprintf(cgiargs, "%s/", getcwd(NULL, MAXLINE));
		sprintf(cgiargs, "%s%s#", cgiargs, ipImage);
		sprintf(cgiargs, "%s%s/", cgiargs, getcwd(NULL, MAXLINE));
		sprintf(cgiargs, "%s%s#", cgiargs, opImage);
		sprintf(cgiargs, "%s%s", cgiargs, "CTG");
		//execl("../opencvExample/sample", "sample", cgiargs, NULL);
		execl("sIPA/sample", "sample", cgiargs, NULL);
	}

	int status = 0;
	uint32_t result = 1;
    waitpid(-1, &status, 0);
    if(status == 0)
    {
    	/*Send the conversion status to client*/
    	result = 0;
    	write(connfd, &result, sizeof(result));

    	memset(buf, 0, MAXLINE);
    	fd2 = open(opImage, /*O_RDONLY*/O_RDWR | O_CREAT , 0666);
    	if(fd2 < 0)
    	{
    		printf("%s:%d:", __func__, __LINE__);
			perror("Error in creating BW file:");
    		exit(fd2);
    	}
    	else
    	{
    		fileSize = Lseek(fd2, 0, SEEK_END);
			Lseek(fd2, 0, SEEK_SET);

			/*Send BW output file size to the client*/
			writeImageFileSize(connfd, fileSize);

    		/*Send a BW image to the client*/
    		send_image(fd2, buf, 256, connfd);
    		printf("Closing the B/W image file\n");
    		Close(fd2);
    	}
    }
    else
    {
    	printf("waitpid was unsuccessful:");
    	write(connfd, &result, sizeof(result));
    }
    remove(opImage);
    remove(ipImage);
}

void mSec(int mil)
{
	usleep(mil * 1000);
}

void *workersHandler(void *payload)
{
	int d;
	int threadNumber = (int)payload;
	pthread_t thread_id = pthread_self();
	struct sched_param param;
	int priority, policy;
	pthread_getschedparam (thread_id, &policy, &param);
	priority = param.sched_priority;	
	printf("Worker %d has priority %d \n", threadNumber, priority);

    while(1)
    {
        pthread_mutex_lock (fifo->mut);
        while (fifo->empty)
        {
            pthread_cond_wait (fifo->notEmpty, fifo->mut);
        }
        queueDeq (fifo, &d);
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notFull);
        printf ("Worker: received %d.\n", d);
        mSec(10000);
        echo(d);
        Close(d);
    }

	return NULL;
}

void *managerHandler(void *payload)
{
	int listenfd, connfd;
	listenfd = (int)payload;
	printf("Manager: listenfd is %d.\n", listenfd);

	pthread_t thread_id = pthread_self();
	struct sched_param param;
	int priority, policy;
	pthread_getschedparam (thread_id, &policy, &param);
	priority = param.sched_priority;	
	printf("Manager has priority %d \n", priority);

	while(1)
	{
		printf("Manager: Waiting for connection request\n");
		connfd = Accept(listenfd, NULL, NULL);
		printf("Manager: Accepted a new connection [%d].\n", connfd);

		pthread_mutex_lock (fifo->mut);
        while (fifo->full) {
            pthread_cond_wait (fifo->notFull, fifo->mut);
        }
        queueEnq (fifo, connfd);
        printf ("Producer: Sent %d.\n", connfd);
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notEmpty);
	}

	return (void *)0;
}

int main(int argc, char *argv[])
{
	int listenfd, connfd;
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	fifo = queueInit(QLEN);
    if (fifo ==  NULL) {
        fprintf (stderr, "Main: Queue Init failed.\n");
        exit (1);
    }

    pthread_attr_t managetAttr, workersAttr;
	pthread_t workerThreads[ACTIVE_CORES];
	pthread_t managerThreads;
	struct sched_param my_param;
	int policy;

	my_param.sched_priority = sched_get_priority_min(SCHED_FIFO);
	int min_priority = my_param.sched_priority;
	pthread_setschedparam(pthread_self(), SCHED_RR, &my_param);
	pthread_getschedparam (pthread_self(), &policy, &my_param);

	pthread_attr_init(&managetAttr);
	pthread_attr_init(&workersAttr);
	pthread_attr_setinheritsched(&managetAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setinheritsched(&workersAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&managetAttr, SCHED_FIFO);
	pthread_attr_setschedpolicy(&workersAttr, SCHED_FIFO);

	my_param.sched_priority = min_priority + 1;
	pthread_attr_setschedparam(&workersAttr, &my_param);
	my_param.sched_priority = min_priority + 3;
	pthread_attr_setschedparam(&managetAttr, &my_param);

	char i;
	for(i=0; i<ACTIVE_CORES; i++)
	{
		pthread_create(&workerThreads[i], &workersAttr, workersHandler, (void *)i);
	}

	listenfd = Open_listenfd(argv[1]);
	printf("Main: Handing over the listening socket[%d] to the manager thread\n", listenfd);

	/*Manager Thread*/
	int ch = pthread_create(&managerThreads, &managetAttr, managerHandler, (void *)listenfd);
	//printf("ch is %d\n", ch);
	perror("");

	while (1);
	exit(0);
}