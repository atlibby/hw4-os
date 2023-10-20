// jdh CS3010 Fall 2023
// example of a mutex lock
// * the producer writes NUM values (10, 20, 30, ...) in a shared variable
// * when the producer is done, it writes a -1 in the shared variable
// * the consumer reads the value that the producer has written
// * if the consumer reads the value -1, then it knows that it's done

#include <stdio.h>
#include <pthread.h>

#define NUM_TO_WRITE 8

typedef struct {
    pthread_mutex_t lock;
    int value;
    int valueIsAvailable;
} ThreadInfo;

void *producer(void *param) {
    int numWritten, valueToWrite;
    ThreadInfo *tinfo = (ThreadInfo *) param;

    numWritten = 0;
    valueToWrite = 10;
    while (numWritten <= NUM_TO_WRITE) {
        pthread_mutex_lock(&tinfo->lock);

        if ( ! tinfo->valueIsAvailable ) {
            if (numWritten == NUM_TO_WRITE) {
                printf("producer writes %d\n", -1);
                tinfo->value = -1;
            } else {
                printf("producer writes %d\n", valueToWrite);
                tinfo->value = valueToWrite;
                valueToWrite = valueToWrite + 10;
            }
            numWritten = numWritten + 1;
            tinfo->valueIsAvailable = 1;
        }

        pthread_mutex_unlock(&tinfo->lock);
    }

    printf("producer is exiting\n");
    pthread_exit(NULL);
} // producer

//--------------------------------------------------

void *consumer(void *param) {
    ThreadInfo *tinfo = (ThreadInfo *) param;
    int done = 0;
    int valueRead;

    while ( ! done ) {
        pthread_mutex_lock(&tinfo->lock);
        if (tinfo->valueIsAvailable) {
            valueRead = tinfo->value;
            printf("consumer read %d\n", valueRead);
            tinfo->valueIsAvailable = 0;
            if (valueRead == -1)
                done = 1;
        }
        pthread_mutex_unlock(&tinfo->lock);
    }

    printf("consumer is exiting\n");
    pthread_exit(NULL);
} // consumer()

//--------------------------------------------------

int main() {
    ThreadInfo tinfo;
    pthread_t producerTid, consumerTid;

    pthread_mutex_init(&tinfo.lock, NULL);
    tinfo.valueIsAvailable = 0;

    pthread_create(&producerTid, NULL, producer, &tinfo);
    pthread_create(&consumerTid, NULL, consumer, &tinfo);

    pthread_join(producerTid, NULL);
    pthread_join(consumerTid, NULL);
    return 0;
} // main()


