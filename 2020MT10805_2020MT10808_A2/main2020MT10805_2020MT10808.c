#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <time.h>
#include <math.h>

#define MAX_QUEUE_SIZE 100
#define MAX_PROCESSES 10
#define MAX_SCHEDULER_CONTEXT_COUNT 100000


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
//char *inputFileName;
//char *outputFileName;

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
    
    for(int i=0; i<processCount; i++){
        printf("%.3f %s %.3f | ",startTime[i], sortedProcesses[scheduledJobs[i]].Id, endTime[i]);
    }
    printf("\nAverage Turnaround Time: %.3f, Average Response Time: %.3f for FCFS\n", averageturnaroundTime, averageresponseTime);
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
    int scheduledContextJobs[MAX_SCHEDULER_CONTEXT_COUNT] ;
    double schedulerContextIn[MAX_SCHEDULER_CONTEXT_COUNT], schedulerContextOut[MAX_SCHEDULER_CONTEXT_COUNT] ;
    int schedulerContextCount = 0 ;

    Queue* roundRobinQueue = createQueue(MAX_QUEUE_SIZE);
    enqueue( roundRobinQueue, 0) ;
    double currTime = sortedProcesses[0].initTime ;
    startingTime[0] = currTime ;
    firstRun[0] = false ;
    int lastAdded = 0 ;
    while ( !isEmpty(roundRobinQueue) ) {
        int currContext = dequeue( roundRobinQueue) ;
        scheduledContextJobs[schedulerContextCount] = currContext ;
        schedulerContextIn[schedulerContextCount] = currTime ;

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

        schedulerContextOut[schedulerContextCount++] = currTime ;

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
    //printf("RR Schedule size, %d \n", schedulerContextCount) ;
    // for ( int i=0 ; i<schedulerContextCount ; ++i ) {
    //     printf("Scheduled %s, started at %f, ended at %f \n", sortedProcesses[scheduledContextJobs[i]].Id, schedulerContextIn[i], schedulerContextOut[i]) ;
    // }
    // FILE *file = fopen(outputFileName, "a");
    // if (file == NULL) {
    //     perror("Error opening file");
    //     return; 
    // }

    for(int i=0; i<schedulerContextCount; i++){
        printf("%.3f %s %.3f | ",schedulerContextIn[i], sortedProcesses[scheduledContextJobs[i]].Id, schedulerContextOut[i]);
    }

    double turnaroundTime[processCount], responseTime[processCount];
    for ( int i=0 ; i<processCount ; ++i ) {
        turnaroundTime[i] = endingTime[i] - sortedProcesses[i].initTime ;
        responseTime[i] = startingTime[i] - sortedProcesses[i].initTime ;
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    printf("\n");
    printf("Average Turnaround Time: %.3f, Average Response Time: %.3f for Round Robin\n", averageturnaroundTime, averageresponseTime);
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

    // for ( int i=0 ; i<processCount ; ++i ) {
    //     printf("i = %d, job = %s \n", i, sortedProcesses[scheduledJobs[i]].Id) ;
    // }

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

    //printing
    // FILE *file = fopen(outputFileName, "a");
    // if (file == NULL) {
    //     perror("Error opening file");
    //     return; 
    // }
    
    for(int i=0; i<processCount; i++){
        printf("%.3f %s %.3f | ", startTime[i], sortedProcesses[scheduledJobs[i]].Id, endTime[i]);
    }
    printf("\n");
    printf("Average Turnaround Time: %.3f, Average Response Time: %.3f for Shortest Job first\n", averageturnaroundTime, averageresponseTime);
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
    // FILE *file = fopen(outputFileName, "a");
    // if (file == NULL) {
    //     perror("Error opening file");
    //     return; 
    // }

    for(int i=0; i<schedulerContextCount; i++){
        printf("%.3f %s %.3f | ", schedulerContextIn[i], sortedProcesses[scheduledContextJobs[i]].Id, schedulerContextOut[i]);
    }

    double turnaroundTime[processCount], responseTime[processCount];
    for ( int i=0 ; i<processCount ; ++i ) {
        turnaroundTime[i] = endingTime[i] - sortedProcesses[i].initTime ;
        responseTime[i] = startingTime[i] - sortedProcesses[i].initTime ;
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    printf("\n");
    printf("Average Turnaround Time: %.3f, Average Response Time: %.3f for Shortest remaining time first\n", averageturnaroundTime, averageresponseTime);
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
    int scheduledContextJobs[MAX_SCHEDULER_CONTEXT_COUNT] ;
    double schedulerContextIn[MAX_SCHEDULER_CONTEXT_COUNT], schedulerContextOut[MAX_SCHEDULER_CONTEXT_COUNT] ;
    int schedulerContextCount = 0 ;

    Queue* MLFQ1 = createQueue(MAX_QUEUE_SIZE);
    Queue* MLFQ2 = createQueue(MAX_QUEUE_SIZE);
    Queue* MLFQ3 = createQueue(MAX_QUEUE_SIZE);

    enqueue( MLFQ1, 0) ;
    double currTime = sortedProcesses[0].initTime ;
    startingTime[0] = currTime ;
    firstRun[0] = false ;
    int lastAdded = 0 ;
    double nextBoost = boostParameter ;
    bool boosted = false ;
    while ( !isEmpty(MLFQ1) || !isEmpty(MLFQ2) || !isEmpty(MLFQ3) ) {

        while ( !isEmpty(MLFQ1) ) {
            int currContext = dequeue( MLFQ1) ;
            double nextTime = FLT_MAX ;
            nextTime = min( nextTime, currTime + min( remainingTime[currContext], timeSliceQ1)) ;     

            scheduledContextJobs[schedulerContextCount] = currContext ;
            schedulerContextIn[schedulerContextCount] = currTime ;

            if ( firstRun[currContext] ) {
                startingTime[currContext] = currTime ;
                firstRun[currContext] = false ;
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

            schedulerContextOut[schedulerContextCount] = currTime ;
            if ( schedulerContextCount > 0 && scheduledContextJobs[schedulerContextCount - 1] == currContext ) {
                schedulerContextOut[schedulerContextCount - 1] = currTime ;
            }
            else schedulerContextCount++ ;

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

            scheduledContextJobs[schedulerContextCount] = currContext ;
            schedulerContextIn[schedulerContextCount] = currTime ;

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

            schedulerContextOut[schedulerContextCount] = currTime ;
            if ( schedulerContextCount > 0 && scheduledContextJobs[schedulerContextCount - 1] == currContext ) {
                schedulerContextOut[schedulerContextCount - 1] = currTime ;
            }
            else schedulerContextCount++ ;

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

            scheduledContextJobs[schedulerContextCount] = currContext ;
            schedulerContextIn[schedulerContextCount] = currTime ;

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

            schedulerContextOut[schedulerContextCount] = currTime ;
            if ( schedulerContextCount > 0 && scheduledContextJobs[schedulerContextCount - 1] == currContext ) {
                schedulerContextOut[schedulerContextCount - 1] = currTime ;
            }
            else schedulerContextCount++ ;

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

        if ( boosted ) {
            while ( !isEmpty(MLFQ2) ) enqueue( MLFQ1, dequeue(MLFQ2)) ;
            while ( !isEmpty(MLFQ3) ) enqueue( MLFQ1, dequeue(MLFQ3)) ;
            boosted = false ;
        }

        for ( int i=lastAdded + 1 ; i<processCount ; ++i ) {
            if ( sortedProcesses[i].initTime <= currTime ) {
                enqueue( MLFQ1, i) ;
                lastAdded = i ;
            }
        }

        if ( !(!isEmpty(MLFQ1) || !isEmpty(MLFQ2) || !isEmpty(MLFQ3)) && lastAdded + 1 < processCount ) {
            lastAdded = lastAdded + 1 ;
            enqueue( MLFQ1, lastAdded) ;
            currTime = sortedProcesses[lastAdded].initTime ;
        }
    }

    for(int i=0; i<schedulerContextCount; i++){
        printf("%.3f %s %.3f | ", schedulerContextIn[i], sortedProcesses[scheduledContextJobs[i]].Id, schedulerContextOut[i]);
    }

    double turnaroundTime[processCount], responseTime[processCount];
    for ( int i=0 ; i<processCount ; ++i ) {
        turnaroundTime[i] = endingTime[i] - sortedProcesses[i].initTime ;
        responseTime[i] = startingTime[i] - sortedProcesses[i].initTime ;
    }
    double averageturnaroundTime = calcAverage(turnaroundTime, processCount);
    double averageresponseTime = calcAverage(responseTime, processCount);

    printf("\n");
    printf("Average Turnaround Time: %.3f, Average Response Time: %.3f for Multi-level feedback queue\n", averageturnaroundTime, averageresponseTime);
}

float generateRandomfloat(){
    float n;
    n = (float)rand()/RAND_MAX;
    return n;
}

float generateRandomnum(){
    float randomNum = (rand()%25)+1;
    return (float)randomNum;
}

float exponentialrand(float lambda){
    float u = (float)rand()/RAND_MAX;
    return -log(1-u)/lambda;
}

int main(){
    int processCount = 5 ;
    float lambda = 0.1 ;
    while ( lambda <= 5 ) {
        printf("\n====================================================================================\n\n") ;
        printf("The Parameter for Exponential Distribution = %f \n\n", lambda) ;

        srand(110*lambda);
        float currTime = generateRandomfloat();

        for(int i=0; i<processCount; i++){
            char temp[2048] = "P";
            char num[10];
            sprintf(num, "%d", (i+1));
            strcat(temp, num);
            strcpy(processes[i].Id, temp);
            processes[i].initTime = currTime;
            processes[i].length = generateRandomnum();
            currTime = currTime + exponentialrand(lambda);
        }

        for (int i = 0; i < processCount; i++) {
            sortedProcesses[i] = processes[i];
        }

        printf("Processes Generated \n") ;
        for(int i=0; i<processCount; i++){
            printf("PId: %s, arrival time: %f, burst time: %f\n", processes[i].Id, processes[i].initTime, processes[i].length);
        }

        printf("\nFCFS : \n"); 
            FirstComeFirstServe(processCount) ;

        printf("\n ----------------------------------------------------------- \n") ;

        printf("\nShortest Job First : \n");            
            shortestJobFirstScheduler(processCount) ;

        printf("\n ----------------------------------------------------------- \n") ;

        printf("\nShortest Remaining Time First : \n"); 
            shortestRemainingTimeFirst(processCount);

        printf("\n ----------------------------------------------------------- \n") ;

        for( int i = 2 ; i <= 6 ; i++ ) {
            printf("\nRound Robin, Time Slice = %d\n", i) ;
                roundRobin(processCount, i) ;
            printf("\n ----------------------------------------------------------- \n") ;
        }

        for( int i = 1, j = 2, k = 4, b = 7 ; i <= 3 ; i += 1, j += 1, k += 1, b += 2 ) {
            printf("\nMulti-Level Feedback Queue, TS1 = %d, TS2 = %d, TS3 = %d, BS = %d\n", i, j, k, b);
                multiLevelFeedbackQueue( processCount, i, j, k, b) ;
            printf("\n ----------------------------------------------------------- \n") ;
        }

        printf("\n====================================================================================\n") ;
        lambda = lambda * 10 ;
    }
    return 0;
}