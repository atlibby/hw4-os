#include <stdio.h>
#include <pthread.h>

#define SHBUFLEN 10
#define NUM_TO_WRITE 10


typedef struct {
    int buffer[SHBUFLEN]; // the buffer
    int in; // location to which a producer will write
    int out; // location from which a consumer will read
    int count; // total # items in the buffer
    int active; // whether consumers are still active
    pthread_mutex_t lock; // the lock
} BufferInfo;

typedef struct {
    int myid; // id of the producer
    int numWritten; // total number of characters written by this producer
    BufferInfo *bufferInfo; // pointer to the BufferInfo
} ProducerInfo;

#define NUM_CONSUMERS 2
typedef struct {
    int myid; // id of the producer
    int numRead; // total number of characters written by this consumer
    BufferInfo *bufferInfo; // pointer to the BufferInfo
} ConsumerInfo;

void *producer(void *param) {
    ProducerInfo *pinfo = (ProducerInfo *) param;
    int valueToWrite;
    int written;

    for (int i = 0; i <= NUM_TO_WRITE; i++) {
        if (i < NUM_TO_WRITE) {
            valueToWrite = i;
        } else {
            valueToWrite = -1;
        }
        written = 0;
        while ( !written ) {
            pthread_mutex_lock(&pinfo->bufferInfo->lock);
            if (pinfo->bufferInfo->count < SHBUFLEN) {
                pinfo->bufferInfo->buffer[pinfo->bufferInfo->in] = valueToWrite;
                printf("producer %d wrote %d\n", pinfo->myid, valueToWrite);
                pinfo->bufferInfo->in = (pinfo->bufferInfo->in + 1) % SHBUFLEN;
                pinfo->bufferInfo->count = pinfo->bufferInfo->count + 1;
                pinfo->numWritten = pinfo->numWritten + 1;
                written = 1;
            }
            pthread_mutex_unlock(&pinfo->bufferInfo->lock);
        }
    }
    printf("producer is exiting\n");
    pthread_exit(NULL);

} // producer

//--------------------------------------------------

void *consumer(void *param) {
    ConsumerInfo *cinfo = (ConsumerInfo *) param;
    int done = 0;
    int gotValue;
    int valueRead;

    while (done == 0) {
        gotValue = 0;
        while (gotValue == 0 && done == 0) {
            if (cinfo->bufferInfo->active) {
                pthread_mutex_lock(&cinfo->bufferInfo->lock);
                if (cinfo->bufferInfo->count > 0) {
                    valueRead = cinfo->bufferInfo->buffer[cinfo->bufferInfo->out];
                    printf("consumer %d read %d\n", cinfo->myid, valueRead);
                    cinfo->bufferInfo->out = (cinfo->bufferInfo->out + 1) % SHBUFLEN;
                    cinfo->bufferInfo->count = cinfo->bufferInfo->count - 1;
                    gotValue = 1;
                    cinfo->numRead = cinfo->numRead + 1;
                    if (valueRead == -1) {
                        cinfo->bufferInfo->active = 0;
                    }
                }
                pthread_mutex_unlock(&cinfo->bufferInfo->lock);
            }
            if (cinfo->bufferInfo->active == 0) {
                done = 1;
            }
        }
    }
    printf("consumer %d is exiting\n", cinfo->myid);
    pthread_exit(NULL);
} // consumer()

//--------------------------------------------------

int main() {
    BufferInfo bufferInfo;
    bufferInfo.in = 0;
    bufferInfo.out = 0;
    bufferInfo.count = 0;
    bufferInfo.active = 1;
    pthread_mutex_init(&bufferInfo.lock, NULL);

    ProducerInfo producerInfo;
    producerInfo.myid = 0;
    producerInfo.numWritten = 0;
    producerInfo.bufferInfo = &bufferInfo;

    ConsumerInfo consumerInfo[NUM_CONSUMERS];
    pthread_t consumerTid[NUM_CONSUMERS];
    pthread_t producerTid;

    pthread_create(&producerTid, NULL, producer, &producerInfo);

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumerInfo[i].myid = i;
        consumerInfo[i].numRead = 0;
        consumerInfo[i].bufferInfo = &bufferInfo;
        pthread_create(&consumerTid[i], NULL, consumer, &consumerInfo[i]);
    }

    pthread_join(producerTid, NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumerTid[i], NULL);
    }

    printf("producer wrote %d\n", producerInfo.numWritten);
    int totalRead = 0;
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        printf("consumer %d read %d\n", i, consumerInfo[i].numRead);
        totalRead += consumerInfo[i].numRead;
    }

    printf("consumers read %d\n", totalRead);

    return 0;
} // main()
