#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>

#define MAX_QUEUE_SIZE 100000
#define MAX_PROCESSES 100000
#define MAX_SCHEDULER_CONTEXT_COUNT 100000
#define MAX_INPUT_LINE 33000
#define MAX_LINE_LENGTH 50

typedef struct {
    int front, rear, size;
    int capacity;
    int* array;
} Queue;

typedef struct {
    int *heap;
    int size;
    int capacity;
} PriorityQueue;

typedef struct {
    char Id[2048] ;
    double initTime ;
    double length ;
} Process ;

Process processes[MAX_PROCESSES] ;
Process sortedProcesses[MAX_PROCESSES];  // Processes Sorted in the order of Arrival Time 
char *inputFileName;
char *outputFileName;

double min( double f1, double f2) {
    if ( f1 <= f2 ) return f1 ;
    return f2 ;
}

double max( double f1, double f2) {
    if ( f1 >= f2 ) return f1 ;
    return f2 ;
}

PriorityQueue *createPriorityQueue(int capacity) {
    PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    pq->capacity = capacity;
    pq->size = 0;
    pq->heap = (int *)malloc(sizeof(int) * (capacity + 1)); // Index 0 not used
    return pq;
}

double calcAverage(double a[], int n){
    double sum = 0;
    for(int i=0; i<n ; i++) sum += a[i] ;
    double average = sum/n ;
    return average ;
}

void push(PriorityQueue *pq, int idx) {
    if (pq->size >= pq->capacity) {
        printf("Priority queue is full.\n");
        return;
    }

    int currentIndex = ++pq->size;
    while (( currentIndex != 1 && sortedProcesses[idx].length < sortedProcesses[pq->heap[currentIndex / 2]].length) || ( currentIndex != 1 && sortedProcesses[idx].length == sortedProcesses[pq->heap[currentIndex / 2]].length && sortedProcesses[idx].initTime <= sortedProcesses[pq->heap[currentIndex / 2]].initTime )) {
        pq->heap[currentIndex] = pq->heap[currentIndex / 2];
        currentIndex /= 2;
    }
    pq->heap[currentIndex] = idx;

    // printf("After Enque ....\n") ;
    // for ( int i=1 ; i<=pq->size ; ++i ) {
    //     printf("i = %d, pq1[i] = %d, len = %f \n", i, pq->heap[i], sortedProcesses[pq->heap[i]].length) ;
    // }
}

int pop(PriorityQueue *pq) {
    if (pq->size <= 0) {
        printf("Priority queue is empty.\n");
        return 0.0; // Return a default value (you can handle this differently)
    }

    int minValue = pq->heap[1];
    int lastValue = pq->heap[pq->size--];

    int currentIndex = 1;
    int childIndex = 2;

    while (childIndex <= pq->size) {
        if ( (childIndex < pq->size && sortedProcesses[pq->heap[childIndex]].length > sortedProcesses[pq->heap[childIndex + 1]].length) || ( childIndex < pq->size && sortedProcesses[pq->heap[childIndex]].length == sortedProcesses[pq->heap[childIndex + 1]].length && sortedProcesses[pq->heap[childIndex]].initTime > sortedProcesses[pq->heap[childIndex + 1]].initTime )) {
            childIndex++;
        }
        if ( (sortedProcesses[lastValue].length < sortedProcesses[pq->heap[childIndex]].length ) || ( sortedProcesses[lastValue].length == sortedProcesses[pq->heap[childIndex]].length && sortedProcesses[lastValue].initTime <= sortedProcesses[pq->heap[childIndex]].initTime )) {
            break;
        }
        pq->heap[currentIndex] = pq->heap[childIndex];
        currentIndex = childIndex;
        childIndex *= 2;
    }

    pq->heap[currentIndex] = lastValue;
    // printf("After Deque ....\n") ;
    // for ( int i=1 ; i<=pq->size ; ++i ) {
    //     printf("i = %d, pq1[i] = %d, len = %f \n", i, pq->heap[i], sortedProcesses[pq->heap[i]].length) ;
    // }
    return minValue;
}

Queue* createQueue(unsigned capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(queue->capacity * sizeof(int));
    return queue;
}

int isFull(Queue* queue) {
    return (queue->size == queue->capacity);
}

int isEmpty(Queue* queue) {
    return (queue->size == 0);
}

void enqueue(Queue* queue, int item) {
    if (isFull(queue)) {
        printf("Queue is full. Cannot enqueue.\n");
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size++;
    // printf("%d enqueued to the queue.\n", item);
}

int dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty. Cannot dequeue.\n");
        return -1; // Return a default value (you can handle this differently)
    }
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return item;
}

int front(Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty. No front element.\n");
        return -1; // Return a default value (you can handle this differently)
    }
    return queue->array[queue->front];
}

int compareProcesses(const void *a, const void *b) {
    const Process procA = *(const Process*)a ;
    const Process procB = *(const Process*)b ;

    if (procA.initTime < procB.initTime) return -1;
    else if (procA.initTime > procB.initTime) return 1;
    else if ( procA.length < procB.length ) return -1 ;
    else if ( procA.length > procB.length ) return 1 ;
    else if ( strcmp(procA.Id, procB.Id) < 0 ) return -1;
    else if ( strcmp(procA.Id, procB.Id) > 0 ) return 1;
    return 0 ;
}

void FirstComeFirstServe(int processCount){
    int scheduledJobs[processCount];
    double startTime[processCount], endTime[processCount];
    double turnaroundTime[processCount], responseTime[processCount];
    for(int i=0; i<processCount; i++){
        scheduledJobs[i] = i;
    }

    double current_time = 0;
    for(int i=0; i<processCount; i++){
        current_time = max(current_time, sortedProcesses[scheduledJobs[i]].initTime);
        startTime[i] = current_time;
        endTime[i] = current_time + sortedProcesses[scheduledJobs[i]].length;
        turnaroundTime[i] = endTime[i] - sortedProcesses[scheduledJobs[i]].initTime;
        responseTime[i] = startTime[i] - sortedProcesses[scheduledJobs[i]].initTime;
        current_time = endTime[i];
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    //printing
    FILE *file = fopen(outputFileName, "a");
    if (file == NULL) {
        perror("Error opening file");
        return; 
    }
    for(int i=0; i<processCount; i++){
        fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledJobs[i]].Id, startTime[i], endTime[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "%.3f %.3f\n", averageturnaroundTime, averageresponseTime);
    fclose(file) ;
}

void roundRobin(int processCount, double roundRobinTimeSlice) {
    bool completedJob[processCount] ;
    bool firstRun[processCount] ;
    double remainingTime[processCount] ;
    for ( int i=0 ; i<processCount ; ++i ) {
        remainingTime[i] = sortedProcesses[i].length ;
        completedJob[i] = false ;
        firstRun[i] = true ;
    }

    double endingTime[processCount], startingTime[processCount] ;
    int scheduledContextJobCurr = -1 ;
    double schedulerContextInCurr = -1, schedulerContextOutCurr = -1 ;
    int lastPrinted = -1, schedulerContextCountCurr = -1 ;

    Queue* roundRobinQueue = createQueue(MAX_QUEUE_SIZE);

    enqueue( roundRobinQueue, 0) ;
    double currTime = sortedProcesses[0].initTime ;
    startingTime[0] = currTime ;
    scheduledContextJobCurr = 0 ;
    schedulerContextInCurr = currTime ;
    schedulerContextCountCurr = 0 ;
    firstRun[0] = false ;
    int lastAdded = 0 ;

    FILE *file = fopen(outputFileName, "a");
    if (file == NULL) {
        perror("Error opening file");
        return; 
    }

    while ( !isEmpty(roundRobinQueue) ) {
        int currContext = dequeue( roundRobinQueue) ;
        if ( scheduledContextJobCurr == currContext ) ;
        else {
            fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledContextJobCurr].Id, schedulerContextInCurr, schedulerContextOutCurr);
            lastPrinted = schedulerContextCountCurr ;
            schedulerContextCountCurr++ ;
            scheduledContextJobCurr = currContext ;
            schedulerContextInCurr = currTime ;
        }

        if ( firstRun[currContext] ) {
            firstRun[currContext] = false ;
            startingTime[currContext] = currTime ;
        }

        if ( remainingTime[currContext] <= roundRobinTimeSlice ) {
            completedJob[currContext] = true ;
            currTime = currTime + remainingTime[currContext] ;
            remainingTime[currContext] = 0 ;
            endingTime[currContext] = currTime ;
        }
        else {
            currTime = currTime + roundRobinTimeSlice ;
            remainingTime[currContext] = remainingTime[currContext] - roundRobinTimeSlice ;
        }

        schedulerContextOutCurr = currTime ;

        for ( int i=lastAdded + 1 ; i<processCount ; ++i ) {
            if ( sortedProcesses[i].initTime <= currTime ) {
                enqueue( roundRobinQueue, i) ;
                lastAdded = i ;
            }
        }

        if ( isEmpty(roundRobinQueue) && lastAdded + 1 < processCount && completedJob[currContext] ) {
            lastAdded = lastAdded + 1 ;
            enqueue( roundRobinQueue, lastAdded) ;
            currTime = sortedProcesses[lastAdded].initTime ;
        }
        if ( !completedJob[currContext] ) enqueue( roundRobinQueue, currContext) ;
    }

    if ( lastPrinted < schedulerContextCountCurr ) {
        fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledContextJobCurr].Id, schedulerContextInCurr, schedulerContextOutCurr);
        lastPrinted++ ;
    }

    double turnaroundTime[processCount], responseTime[processCount];
    for ( int i=0 ; i<processCount ; ++i ) {
        turnaroundTime[i] = endingTime[i] - sortedProcesses[i].initTime ;
        responseTime[i] = startingTime[i] - sortedProcesses[i].initTime ;
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    fprintf(file, "\n");
    fprintf(file, "%.3f %.3f\n", averageturnaroundTime, averageresponseTime);
    fclose(file) ;
}

void shortestJobFirstScheduler(int processCount) {
    PriorityQueue *pq1 = createPriorityQueue(processCount) ;

    push(pq1, 0) ;
    int scheduledJobs[processCount] ;
    int completedJob = 0, lastAdded = 0 ;
    double currTime = sortedProcesses[0].initTime ;
    while ( completedJob < processCount ) {
        int currJob = pop(pq1) ;
        scheduledJobs[completedJob] = currJob ;
        completedJob++ ;

        currTime = currTime + sortedProcesses[currJob].length ;
        while ( lastAdded + 1 < processCount && sortedProcesses[lastAdded + 1].initTime <= currTime ) {
            push( pq1, lastAdded + 1) ;
            lastAdded++ ;
        }

        if ( pq1->size == 0 && lastAdded + 1 < processCount ) {
            push(pq1, lastAdded + 1) ;
            lastAdded++ ;
            currTime = sortedProcesses[lastAdded].initTime ;
        }
    }

    currTime = 0;
    double startTime[processCount], endTime[processCount];
    double turnaroundTime[processCount], responseTime[processCount];
    for(int i=0; i<processCount; i++){
        currTime = max(currTime, sortedProcesses[scheduledJobs[i]].initTime);
        startTime[i] = currTime;
        endTime[i] = currTime + sortedProcesses[scheduledJobs[i]].length;
        turnaroundTime[i] = endTime[i] - sortedProcesses[scheduledJobs[i]].initTime;
        responseTime[i] = startTime[i] - sortedProcesses[scheduledJobs[i]].initTime;
        currTime = endTime[i];
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    FILE *file = fopen(outputFileName, "a");
    if (file == NULL) {
        perror("Error opening file");
        return; 
    }
    for(int i=0; i<processCount; i++){
        fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledJobs[i]].Id, startTime[i], endTime[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "%.3f %.3f\n", averageturnaroundTime, averageresponseTime);
    fclose(file) ;
}

void shortestRemainingTimeFirst(int processCount){
    bool firstRun[processCount] ;
    double burstTime[processCount];
    for(int i=0; i<processCount; i++){
        burstTime[i] = sortedProcesses[i].length;
        firstRun[i] = true;
    }
    double startingTime[processCount], endingTime[processCount];
    int scheduledContextJobs[MAX_SCHEDULER_CONTEXT_COUNT];
    double schedulerContextIn[MAX_SCHEDULER_CONTEXT_COUNT], schedulerContextOut[MAX_SCHEDULER_CONTEXT_COUNT] ;
    int schedulerContextCount = 0;
    double currTime = sortedProcesses[0].initTime;
    int lastadded = 0;
    startingTime[0] = currTime;
    firstRun[0] = false;
    PriorityQueue *pq = createPriorityQueue(processCount) ;
    push(pq, 0) ;
    scheduledContextJobs[0] = 0;
    schedulerContextIn[0] = currTime;
    schedulerContextCount++;
    while(pq->size>0){
        int currJob = pop(pq);
        if(firstRun[currJob]){
            firstRun[currJob] = false;
            startingTime[currJob] = currTime;
        }
        if(scheduledContextJobs[schedulerContextCount-1]!=currJob){
            schedulerContextOut[schedulerContextCount-1] = currTime;
            scheduledContextJobs[schedulerContextCount] = currJob;
            schedulerContextIn[schedulerContextCount] = currTime;
            schedulerContextCount++;
        }

        double nextTime = 0;
        if(lastadded+1>=processCount) nextTime = FLT_MAX;
        else nextTime = sortedProcesses[lastadded+1].initTime;
        if(currTime + sortedProcesses[currJob].length <= nextTime){
            currTime = currTime + sortedProcesses[currJob].length;
            sortedProcesses[currJob].length = 0;
            endingTime[currJob] = currTime;
            schedulerContextOut[schedulerContextCount-1] = currTime;
        }

        else{
            if(currTime+sortedProcesses[currJob].length <= nextTime + sortedProcesses[lastadded+1].length){
                sortedProcesses[currJob].length -= (nextTime-currTime);
                currTime = nextTime;
                push(pq, currJob);
                push(pq, lastadded+1);
                lastadded++;
            }
            else{ //context switch
                sortedProcesses[currJob].length -= (nextTime-currTime);
                currTime = nextTime;
                push(pq, currJob);
                push(pq, lastadded+1);
                lastadded++;
            }
        }
        if(pq->size == 0 && lastadded+1<processCount){
            push(pq, lastadded+1);
            currTime = sortedProcesses[lastadded + 1].initTime ;
            lastadded++;
        }
    }
    for(int i=0; i<processCount; i++){
        sortedProcesses[i].length = burstTime[i];
    }
    FILE *file = fopen(outputFileName, "a");
    if (file == NULL) {
        perror("Error opening file");
        return; 
    }
    for(int i=0; i<schedulerContextCount; i++){
        fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledContextJobs[i]].Id, schedulerContextIn[i], schedulerContextOut[i]);
    }

    double turnaroundTime[processCount], responseTime[processCount];
    for ( int i=0 ; i<processCount ; ++i ) {
        turnaroundTime[i] = endingTime[i] - sortedProcesses[i].initTime ;
        responseTime[i] = startingTime[i] - sortedProcesses[i].initTime ;
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    fprintf(file, "\n");
    fprintf(file, "%.3f %.3f\n", averageturnaroundTime, averageresponseTime);
    fclose(file) ;
}

void multiLevelFeedbackQueue(int processCount, double timeSliceQ1, double timeSliceQ2, double timeSliceQ3, double boostParameter) {
    bool completedJob[processCount] ;
    bool firstRun[processCount] ;
    double remainingTime[processCount] ;
    for ( int i=0 ; i<processCount ; ++i ) {
        remainingTime[i] = sortedProcesses[i].length ;
        completedJob[i] = false ;
        firstRun[i] = true ;
    }

    double endingTime[processCount], startingTime[processCount] ;
    int scheduledContextJobCurr = -1 ;
    double schedulerContextInCurr = -1, schedulerContextOutCurr = -1 ;
    int lastPrinted = 0, schedulerContextCountCurr = 0 ;

    Queue* MLFQ1 = createQueue(MAX_QUEUE_SIZE);
    Queue* MLFQ2 = createQueue(MAX_QUEUE_SIZE);
    Queue* MLFQ3 = createQueue(MAX_QUEUE_SIZE);

    enqueue( MLFQ1, 0) ;
    double currTime = sortedProcesses[0].initTime ;
    startingTime[0] = currTime ;
    firstRun[0] = false ;
    int lastAdded = 0 ;
    scheduledContextJobCurr = 0 ;
    schedulerContextInCurr = currTime ;
    schedulerContextCountCurr = 1 ;

    double nextBoost = boostParameter ;
    bool boosted = false ;

    FILE *file = fopen(outputFileName, "a");
    if (file == NULL) {
        perror("Error opening file");
        return; 
    }

    while ( !isEmpty(MLFQ1) || !isEmpty(MLFQ2) || !isEmpty(MLFQ3) ) {

        while ( !isEmpty(MLFQ1) ) {
            int currContext = dequeue( MLFQ1) ;
            double nextTime = FLT_MAX ;
            nextTime = min( nextTime, currTime + min( remainingTime[currContext], timeSliceQ1)) ;     

            if ( scheduledContextJobCurr == currContext ) ;
            else {
                fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledContextJobCurr].Id, schedulerContextInCurr, schedulerContextOutCurr);
                lastPrinted = schedulerContextCountCurr ;
                schedulerContextCountCurr++ ;
                scheduledContextJobCurr = currContext ;
                schedulerContextInCurr = currTime ;
            }

            if ( firstRun[currContext] ) {
                startingTime[currContext] = currTime ;
                firstRun[currContext] = false ;
            }

            if ( nextTime >= nextBoost ) {
                if ( remainingTime[currContext] <= nextBoost - currTime ) {
                    remainingTime[currContext] = 0 ;
                    endingTime[currContext] = nextBoost ;
                    completedJob[currContext] = true ;
                }
                else {
                    remainingTime[currContext] -= nextBoost - currTime ;
                    enqueue( MLFQ2, currContext) ;
                }

                currTime = nextBoost ;
                schedulerContextOutCurr = currTime ;
                nextBoost += boostParameter ;
                boosted = true ;
                break ;
            }

            if ( remainingTime[currContext] <= timeSliceQ1 ) {
                currTime += remainingTime[currContext] ;
                remainingTime[currContext] = 0 ;
                endingTime[currContext] = currTime ;
            }
            else {
                remainingTime[currContext] -= timeSliceQ1 ;
                currTime += timeSliceQ1 ;
                if ( currTime >= nextBoost ) enqueue( MLFQ3, currContext) ;
                else enqueue( MLFQ2, currContext) ;
            }

            schedulerContextOutCurr = currTime ;

            for ( int i=lastAdded + 1 ; i<processCount ; ++i ) {
                if ( sortedProcesses[i].initTime <= currTime ) {
                    enqueue( MLFQ1, i) ;
                    lastAdded = i ;
                }
            }

            if ( currTime >= nextBoost ) {
                nextBoost += boostParameter ;
                boosted = true ;
                break ;
            }
        }

        while( isEmpty(MLFQ1) && !isEmpty(MLFQ2) && !boosted ) {
            int currContext = dequeue( MLFQ2) ;
            double nextTime = FLT_MAX ;
            nextTime = min( nextTime, currTime + min( remainingTime[currContext], timeSliceQ2)) ;

            if ( scheduledContextJobCurr == currContext ) ;
            else {
                fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledContextJobCurr].Id, schedulerContextInCurr, schedulerContextOutCurr);
                lastPrinted = schedulerContextCountCurr ;
                schedulerContextCountCurr++ ;
                scheduledContextJobCurr = currContext ;
                schedulerContextInCurr = currTime ;
            }

            if ( nextTime >= nextBoost ) {
                if ( remainingTime[currContext] <= nextBoost - currTime ) {
                    remainingTime[currContext] = 0 ;
                    endingTime[currContext] = nextBoost ;
                    completedJob[currContext] = true ;
                }
                else {
                    remainingTime[currContext] -= nextBoost - currTime ;
                    enqueue( MLFQ3, currContext) ;
                }
                
                currTime = nextBoost ;
                schedulerContextOutCurr = currTime ;
                nextBoost += boostParameter ;
                boosted = true ;
                break ;
            }

            if ( remainingTime[currContext] <= timeSliceQ2 ) {
                currTime += remainingTime[currContext] ;
                remainingTime[currContext] = 0 ;
                endingTime[currContext] = currTime ;
            }
            else {
                remainingTime[currContext] -= timeSliceQ2 ;
                currTime += timeSliceQ2 ;
                enqueue( MLFQ3, currContext) ;
            }

            schedulerContextOutCurr = currTime ;

            for ( int i=lastAdded + 1 ; i<processCount ; ++i ) {
                if ( sortedProcesses[i].initTime <= currTime ) {
                    enqueue( MLFQ1, i) ;
                    lastAdded = i ;
                }
            }

            if ( currTime >= nextBoost ) {
                nextBoost += boostParameter ;
                boosted = true ;
                break ;
            }
        }

        while( isEmpty(MLFQ1) && isEmpty(MLFQ2) && !isEmpty(MLFQ3) && !boosted ) {
            int currContext = dequeue( MLFQ3) ;
            double nextTime = FLT_MAX ;
            nextTime = min( nextTime, currTime + min( remainingTime[currContext], timeSliceQ3)) ;

            if ( scheduledContextJobCurr == currContext ) ;
            else {
                fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledContextJobCurr].Id, schedulerContextInCurr, schedulerContextOutCurr);
                lastPrinted = schedulerContextCountCurr ;
                schedulerContextCountCurr++ ;
                scheduledContextJobCurr = currContext ;
                schedulerContextInCurr = currTime ;
            }

            if ( nextTime >= nextBoost ) {
                if ( remainingTime[currContext] <= nextBoost - currTime ) {
                    remainingTime[currContext] = 0 ;
                    endingTime[currContext] = nextBoost ;
                    completedJob[currContext] = true ;
                }
                else {
                    remainingTime[currContext] -= nextBoost - currTime ;
                    enqueue( MLFQ3, currContext) ;
                }
                
                currTime = nextBoost ;
                schedulerContextOutCurr = currTime ;
                nextBoost += boostParameter ;
                boosted = true ;
                break ;
            }

            if ( remainingTime[currContext] <= timeSliceQ3 ) {
                currTime += remainingTime[currContext] ;
                remainingTime[currContext] = 0 ;
                endingTime[currContext] = currTime ;
            }
            else {
                remainingTime[currContext] -= timeSliceQ3 ;
                currTime += timeSliceQ3 ;
                enqueue( MLFQ3, currContext) ;
            }

            schedulerContextOutCurr = currTime ;

            for ( int i=lastAdded + 1 ; i<processCount ; ++i ) {
                if ( sortedProcesses[i].initTime <= currTime ) {
                    enqueue( MLFQ1, i) ;
                    lastAdded = i ;
                }
            }

            if ( currTime >= nextBoost ) {
                nextBoost += boostParameter ;
                boosted = true ;
                break ;
            }
        }

        for ( int i=lastAdded + 1 ; i<processCount ; ++i ) {
            if ( sortedProcesses[i].initTime <= currTime ) {
                enqueue( MLFQ1, i) ;
                lastAdded = i ;
            }
        }

        if ( boosted ) {
            while ( !isEmpty(MLFQ2) ) enqueue( MLFQ1, dequeue(MLFQ2)) ;
            while ( !isEmpty(MLFQ3) ) enqueue( MLFQ1, dequeue(MLFQ3)) ;
            boosted = false ;
        }

        if ( !(!isEmpty(MLFQ1) || !isEmpty(MLFQ2) || !isEmpty(MLFQ3)) && lastAdded + 1 < processCount ) {
            lastAdded = lastAdded + 1 ;
            enqueue( MLFQ1, lastAdded) ;
            currTime = sortedProcesses[lastAdded].initTime ;
            while ( nextBoost < currTime ) nextBoost += boostParameter ;
        }
    }

    if ( lastPrinted < schedulerContextCountCurr ) {
        fprintf(file, "%s %.3f %.3f ",sortedProcesses[scheduledContextJobCurr].Id, schedulerContextInCurr, schedulerContextOutCurr);
        lastPrinted++ ;
    }

    double turnaroundTime[processCount], responseTime[processCount];
    for ( int i=0 ; i<processCount ; ++i ) {
        turnaroundTime[i] = endingTime[i] - sortedProcesses[i].initTime ;
        responseTime[i] = startingTime[i] - sortedProcesses[i].initTime ;
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    fprintf(file, "\n");
    fprintf(file, "%.3f %.3f\n", averageturnaroundTime, averageresponseTime);
    fclose(file) ;
}

void refineInput(int currFileInputSize,char fileInput[MAX_INPUT_LINE][MAX_LINE_LENGTH]) {
    for ( int i=0 ; i<currFileInputSize ; ++i ) {
        const char delimiter[] = " " ;
        char *token = strtok(fileInput[i], delimiter);
        strcpy(processes[i].Id, token) ;

        token = strtok(NULL, delimiter);
        char *endptr ;
        processes[i].initTime = strtof(token, &endptr) ;

        token = strtok(NULL, delimiter);
        processes[i].length = strtof(token, &endptr) ;
    }
    
    return ;
}

int readInput(char* inputFilename, char fileInput[MAX_INPUT_LINE][MAX_LINE_LENGTH]) {
    inputFilename[strcspn(inputFilename, "\n")] = '\0';

    FILE *file = fopen(inputFilename, "r");

    if (file == NULL) {
        perror("Error opening file");
        return 1; 
    }
    
    int currFileInputSize = 0 ;
    while (fgets(fileInput[currFileInputSize], sizeof(fileInput[currFileInputSize]), file) != NULL) {
        // printf("%d = %s", currFileInputSize, fileInput[currFileInputSize]);
        currFileInputSize++ ;
    }
    fclose(file) ;
    return currFileInputSize ;
}

int main(int argc, char *argv[]) {

    if( argc != 8 ) {
        printf("Incomplete Set of Arguments \n");
        return 0;
    }

    inputFileName = argv[1];
    outputFileName = argv[2];

    char *endptr ;
    double roundRobinTimeSlice = strtof(argv[3], &endptr);
    double timeSliceQ1 = strtof(argv[4], &endptr);
    double timeSliceQ2 = strtof(argv[5], &endptr);
    double timeSliceQ3 = strtof(argv[6], &endptr);
    double boostParameter = strtof(argv[7], &endptr);

    char fileInput[MAX_INPUT_LINE][MAX_LINE_LENGTH] ;
    int processCount = readInput( inputFileName, fileInput) ;
    refineInput( processCount, fileInput) ;

    for (int i = 0; i < processCount; i++) {
        sortedProcesses[i] = processes[i];
    }
    qsort(sortedProcesses, processCount, sizeof(Process), compareProcesses);

    FirstComeFirstServe(processCount) ;                                 // printf("Printed FCFS \n") ;
    roundRobin(processCount, roundRobinTimeSlice) ;                     // printf("Printed RR \n") ;
    shortestJobFirstScheduler(processCount) ;                           // printf("Printed SJF \n") ;
    shortestRemainingTimeFirst(processCount);                           // printf("Printed SRTF \n") ;
    multiLevelFeedbackQueue(processCount, timeSliceQ1, timeSliceQ2, timeSliceQ3, boostParameter) ;

    return 0;
}


