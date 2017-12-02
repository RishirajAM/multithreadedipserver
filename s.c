#include "csapp.h"
#include "image_helper.h"
#include "q.h"
#include <unistd.h>

#define ACTIVE_CORES ((sysconf(_SC_NPROCESSORS_ONLN)<=1)?   \
                     (1):                                   \
                     (sysconf(_SC_NPROCESSORS_ONLN) - 1)    )

#define QLEN ((ACTIVE_CORES)*(2))

queue *fifo;

void echo(int connfd)
{

	size_t n = 0;
	char buf[MAXLINE];
	uint32_t fd2;

	//char bufFS[10] = {};
	/*n = read(connfd, buf, 5);
	uint32_t fileSize = atoi(buf);*/
	uint32_t fileSize = 0;
	n = read(connfd, &fileSize, sizeof(fileSize));
	printf("Server red: %d(%d bytes)\n", fileSize, (int)n);
	//uint32_t fileSize = readImageFileSize(connfd);

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
			/*sprintf(buf, "%0d", fileSize);
			write(connfd, buf, strlen(buf));*/
			write(connfd, &fileSize, sizeof(fileSize));
			//printf("Server wrote: %s (%d)\n", buf, (int)strlen(buf));
			//writeImageFileSize(connfd, fileSize);

    		/*Send a B/W image to the client*/
    		send_image(fd2, buf, 256, connfd);
    		printf("Closing the B/W image file\n");
    		close(fd2);
    	}
    }
    else
    	printf("waitpid returned %d\n", status);
}

void mSec(int mil)
{
	usleep(mil * 1000);
}

void *workersHandler(void *payload)
{
	int d;
	int threadNumber = (int)payload;
	printf("Worker %d\n", threadNumber);
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