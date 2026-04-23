/*****************************************************************************
 *  PRODUCER-CONSUMER PROBLEM SIMULATION
 *  --------------------------------------------------------------------------
 *  Operating Systems project - POSIX threads + mutex + counting semaphores.
 *
 *  MANDATORY REQUIREMENTS implemented:
 *    1. Fixed-size shared buffer   -> circular int array of size N
 *    2. Multiple producer threads  -> pthread_create() in a loop
 *    3. Multiple consumer threads  -> pthread_create() in a loop
 *    4. Mutex for mutual exclusion -> pthread_mutex_t mutex
 *    5. Semaphores for sync        -> sem_t empty_sem, full_sem
 *
 *  ADDITIONAL FEATURES implemented (6 of 7):
 *    A. Variable production/consumption rates (random usleep delays)
 *    B. Logging of thread activity (timestamped output)
 *    C. Dynamic buffer size (prompted from user at runtime)
 *    D. Graceful shutdown (pthread_join + destroy + free)
 *    E. Thread statistics (per-thread + totals + summary)
 *    F. Buffer visualization (live ASCII snapshot after every event)
 *
 *  Compile:  gcc -Wall -o pc pc.c -lpthread
 *  Run:      ./pc
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       /* usleep() */
#include <pthread.h>      /* POSIX threads + mutex */
#include <semaphore.h>    /* POSIX semaphores */
#include <time.h>         /* time() for random seed and timestamps */

/* =========================================================================
 * Global configuration (filled in from user input at runtime).
 * Making these run-time variables instead of #define values is what gives us
 * the "dynamic buffer size" feature.
 * ========================================================================= */
int BUFFER_SIZE;       /* max items the buffer can hold                    */
int NUM_PRODUCERS;     /* how many producer threads to create              */
int NUM_CONSUMERS;     /* how many consumer threads to create              */
int ITEMS_PER_PROD;    /* how many items each producer will produce        */

/* =========================================================================
 * Shared resources (the critical section protects these).
 * ========================================================================= */
int *buffer;           /* the bounded buffer (allocated with size N)       */
int in  = 0;           /* index where the NEXT item will be inserted       */
int out = 0;           /* index from which the NEXT item will be removed   */
int count = 0;         /* current number of items in buffer (0..N)         */

/* =========================================================================
 * Synchronization primitives.
 *   mutex     -> guarantees only ONE thread inside the critical section
 *   empty_sem -> counts EMPTY slots (producers wait here when buffer full)
 *   full_sem  -> counts FILLED slots (consumers wait here when buffer empty)
 * ========================================================================= */
pthread_mutex_t mutex;
sem_t empty_sem;
sem_t full_sem;

/* =========================================================================
 * Statistics (also updated only inside the critical section).
 * ========================================================================= */
int total_produced = 0;
int total_consumed = 0;
int *producer_counts;   /* items produced by each producer thread          */
int *consumer_counts;   /* items consumed by each consumer thread          */

/* -------------------------------------------------------------------------
 * Helper: print current wall-clock timestamp [HH:MM:SS] for log lines.
 * ------------------------------------------------------------------------- */
static void print_timestamp(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}

/* -------------------------------------------------------------------------
 * Helper: visualise the buffer. MUST be called while holding the mutex.
 * Shows filled slots with their values and empty slots as dots.
 * ------------------------------------------------------------------------- */
static void display_buffer(void) {
    printf("Buffer [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (i < count) {
            int idx = (out + i) % BUFFER_SIZE;  /* walk from 'out' forward */
            printf(" %4d ", buffer[idx]);
        } else {
            printf("  .   ");                    /* empty slot */
        }
    }
    printf("]  %d/%d\n", count, BUFFER_SIZE);
}

/* =========================================================================
 * PRODUCER THREAD FUNCTION
 *   1. Produce an item (simulated with random delay).
 *   2. sem_wait(empty_sem) -> block if no empty slot available.
 *   3. mutex_lock           -> enter critical section.
 *   4. Put item in buffer, update indices & stats, print log.
 *   5. mutex_unlock         -> leave critical section.
 *   6. sem_post(full_sem)   -> signal consumers that a new item is ready.
 * ========================================================================= */
void *producer(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < ITEMS_PER_PROD; i++) {
        /* 1. Produce: simulate some work with a random 200-700 ms delay. */
        int item = id * 1000 + i;                     /* unique item tag  */
        usleep(((rand() % 500) + 200) * 1000);

        /* 2. Wait for an empty slot (prevents OVERFLOW). */
        sem_wait(&empty_sem);

        /* 3. Enter critical section. */
        pthread_mutex_lock(&mutex);

        /* ---------- CRITICAL SECTION START ---------- */
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;            /* circular wrap-around   */
        count++;
        total_produced++;
        producer_counts[id]++;

        print_timestamp();
        printf("Producer %d PRODUCED item %-5d -> ", id, item);
        display_buffer();
        /* ---------- CRITICAL SECTION END ------------ */

        pthread_mutex_unlock(&mutex);

        /* 4. Signal: one more filled slot is available for consumers. */
        sem_post(&full_sem);
    }

    print_timestamp();
    printf(">>> Producer %d SHUTTING DOWN (produced %d items total)\n",
           id, producer_counts[id]);
    return NULL;
}

/* =========================================================================
 * CONSUMER THREAD FUNCTION
 *   Consumers divide the total work evenly among themselves.
 *   Steps mirror the producer but with empty/full swapped.
 * ========================================================================= */
void *consumer(void *arg) {
    int id = *(int *)arg;

    /* Evenly split total production across consumers;
       the first 'remainder' consumers each take one extra item.         */
    int total_items = NUM_PRODUCERS * ITEMS_PER_PROD;
    int base        = total_items / NUM_CONSUMERS;
    int remainder   = total_items % NUM_CONSUMERS;
    int my_quota    = base + (id < remainder ? 1 : 0);

    for (int i = 0; i < my_quota; i++) {
        /* 1. Wait for a filled slot (prevents UNDERFLOW). */
        sem_wait(&full_sem);

        /* 2. Enter critical section. */
        pthread_mutex_lock(&mutex);

        /* ---------- CRITICAL SECTION START ---------- */
        int item = buffer[out];
        buffer[out] = 0;                        /* clear (cosmetic)       */
        out = (out + 1) % BUFFER_SIZE;          /* circular wrap-around   */
        count--;
        total_consumed++;
        consumer_counts[id]++;

        print_timestamp();
        printf("Consumer %d CONSUMED item %-5d <- ", id, item);
        display_buffer();
        /* ---------- CRITICAL SECTION END ------------ */

        pthread_mutex_unlock(&mutex);

        /* 3. Signal: one more empty slot is available for producers. */
        sem_post(&empty_sem);

        /* 4. "Use" the item: simulate processing with 300-1000 ms delay. */
        usleep(((rand() % 700) + 300) * 1000);
    }

    print_timestamp();
    printf(">>> Consumer %d SHUTTING DOWN (consumed %d items total)\n",
           id, consumer_counts[id]);
    return NULL;
}

/* =========================================================================
 * MAIN: get config, initialise, launch threads, join, clean up, summarise.
 * ========================================================================= */
int main(void) {
    srand((unsigned)time(NULL));

    printf("====================================================\n");
    printf("     PRODUCER - CONSUMER  SIMULATION  (pthreads)    \n");
    printf("====================================================\n\n");

    /* ---- Dynamic configuration from the user ---- */
    printf("Enter buffer size            : ");
    if (scanf("%d", &BUFFER_SIZE) != 1) { fprintf(stderr, "bad input\n"); return 1; }
    printf("Enter number of producers    : ");
    if (scanf("%d", &NUM_PRODUCERS) != 1) { fprintf(stderr, "bad input\n"); return 1; }
    printf("Enter number of consumers    : ");
    if (scanf("%d", &NUM_CONSUMERS) != 1) { fprintf(stderr, "bad input\n"); return 1; }
    printf("Enter items per producer     : ");
    if (scanf("%d", &ITEMS_PER_PROD) != 1) { fprintf(stderr, "bad input\n"); return 1; }

    if (BUFFER_SIZE <= 0 || NUM_PRODUCERS <= 0 ||
        NUM_CONSUMERS <= 0 || ITEMS_PER_PROD <= 0) {
        fprintf(stderr, "\nAll values must be positive integers.\n");
        return 1;
    }

    int total_items = NUM_PRODUCERS * ITEMS_PER_PROD;

    printf("\n----------------------------------------------------\n");
    printf(" Buffer size      : %d\n", BUFFER_SIZE);
    printf(" Producers        : %d (each produces %d items)\n",
           NUM_PRODUCERS, ITEMS_PER_PROD);
    printf(" Consumers        : %d\n", NUM_CONSUMERS);
    printf(" Total items      : %d\n", total_items);
    printf("----------------------------------------------------\n\n");

    /* ---- Allocate memory for buffer and statistics ---- */
    buffer           = calloc(BUFFER_SIZE,    sizeof(int));
    producer_counts  = calloc(NUM_PRODUCERS,  sizeof(int));
    consumer_counts  = calloc(NUM_CONSUMERS,  sizeof(int));
    if (!buffer || !producer_counts || !consumer_counts) {
        fprintf(stderr, "Out of memory.\n");
        return 1;
    }

    /* ---- Initialise synchronisation primitives ---- */
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty_sem, 0, BUFFER_SIZE);   /* all slots empty initially */
    sem_init(&full_sem,  0, 0);             /* no filled slots initially */

    /* ---- Create producer & consumer threads ---- */
    pthread_t *producers = malloc(NUM_PRODUCERS * sizeof(pthread_t));
    pthread_t *consumers = malloc(NUM_CONSUMERS * sizeof(pthread_t));
    int       *pids      = malloc(NUM_PRODUCERS * sizeof(int));
    int       *cids      = malloc(NUM_CONSUMERS * sizeof(int));

    printf("### SIMULATION STARTS ###\n\n");

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pids[i] = i;
        pthread_create(&producers[i], NULL, producer, &pids[i]);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        cids[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &cids[i]);
    }

    /* ---- Wait for all threads to finish (graceful shutdown step 1) ---- */
    for (int i = 0; i < NUM_PRODUCERS; i++) pthread_join(producers[i], NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++) pthread_join(consumers[i], NULL);

    /* ---- Destroy synchronisation primitives (graceful shutdown step 2) - */
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty_sem);
    sem_destroy(&full_sem);

    /* ---- Print statistics summary ---- */
    printf("\n====================================================\n");
    printf("                 SIMULATION SUMMARY                 \n");
    printf("====================================================\n");
    printf(" Total items produced : %d\n", total_produced);
    printf(" Total items consumed : %d\n", total_consumed);
    printf(" Items left in buffer : %d\n", count);
    printf("----------------------------------------------------\n");
    for (int i = 0; i < NUM_PRODUCERS; i++)
        printf(" Producer %d produced  : %d items\n", i, producer_counts[i]);
    for (int i = 0; i < NUM_CONSUMERS; i++)
        printf(" Consumer %d consumed  : %d items\n", i, consumer_counts[i]);
    printf("====================================================\n");

    /* ---- Free all heap memory (graceful shutdown step 3) ---- */
    free(buffer);
    free(producer_counts);
    free(consumer_counts);
    free(producers);
    free(consumers);
    free(pids);
    free(cids);

    return 0;
}
