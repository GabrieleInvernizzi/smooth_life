#include "thread_pool.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>


typedef struct {
    TaskFn task;
    void* args;
} TPTask;

typedef struct {
    TPTask* buffer; 
    unsigned int front_i, back_i;
    size_t capacity;
} TPTaskQueue;

struct ThreadPool {
    size_t n_threads;
    pthread_t* threads;
    pthread_mutex_t task_mutex;
    pthread_cond_t task_cond;
    pthread_cond_t wait_cond;
    TPTaskQueue task_queue;
    size_t working_count;
    bool running;
};


// task queue
static int tp_task_queue_init(TPTaskQueue* tq, size_t cap) {
    tq->capacity = cap + 1;
    tq->buffer = malloc(tq->capacity * sizeof(TPTask));
    if (!tq->buffer) return -1;
    tq->front_i = 0;
    tq->back_i = 0;

    return 0;
}

static void tp_task_queue_deinit(TPTaskQueue* tq) {
    if (tq->buffer) free(tq->buffer);
    tq->capacity = 0;
}

static inline bool tp_task_queue_is_empty(TPTaskQueue* tq) {
    return tq->front_i == tq->back_i;
}

static int tp_task_queue_insert(TPTaskQueue* tq, TPTask task) {
    unsigned int new_back_i = (tq->back_i + 1) % tq->capacity;
    if (new_back_i == tq->front_i)
        return -1;
    
    tq->buffer[new_back_i] = task;
    tq->back_i = new_back_i;

    return 0;
}

static int tp_task_queue_remove(TPTaskQueue* tq, TPTask* task) {
    if (tp_task_queue_is_empty(tq))
        return -1;
        
    tq->front_i = (tq->front_i + 1) % tq->capacity;
    if (task)
        *task = (TPTask)tq->buffer[tq->front_i];
    return 0;
}


// Thread pool
static void* tp_worker(void *arg) {
    ThreadPool* tp = arg;
    TPTask task;
    int removed = -1;

    while (1) {
        pthread_mutex_lock(&(tp->task_mutex));

        while (tp_task_queue_is_empty(&tp->task_queue) && tp->running)
            pthread_cond_wait(&(tp->task_cond), &(tp->task_mutex));

        if (!tp->running)
            break;

        removed = tp_task_queue_remove(&tp->task_queue, &task);
        tp->working_count++;

        pthread_mutex_unlock(&(tp->task_mutex));

        if (removed == 0)
            task.task(task.args);

        pthread_mutex_lock(&(tp->task_mutex));
        tp->working_count--;
        if (tp->running && tp->working_count == 0 && tp_task_queue_is_empty(&tp->task_queue))
            pthread_cond_signal(&(tp->wait_cond));
        pthread_mutex_unlock(&(tp->task_mutex));
    }


    tp->n_threads--;
    pthread_cond_signal(&(tp->wait_cond));
    pthread_mutex_unlock(&(tp->task_mutex));
    return NULL;
}


ThreadPool* tp_init(size_t n_threads, size_t max_tasks) {
    if (n_threads < 1 || max_tasks < 1) return NULL;

    ThreadPool *tp = malloc(sizeof(ThreadPool));
    if (!tp) return NULL;
    tp->n_threads = n_threads;
    tp->threads = malloc(n_threads * sizeof(pthread_t));
    if (!tp->threads) {
        free(tp);
        return NULL;
    }
    tp->working_count = 0;

    pthread_mutex_init(&(tp->task_mutex), NULL);
    pthread_cond_init(&(tp->task_cond), NULL);
    pthread_cond_init(&(tp->wait_cond), NULL);

    if (tp_task_queue_init(&tp->task_queue, max_tasks) == -1) {
        tp_deinit(tp);
        return NULL;
    }

    tp->running = true;

    for (size_t i = 0; i < n_threads; i++) {
        pthread_create(&tp->threads[i], NULL, tp_worker, tp);
        pthread_detach(tp->threads[i]);
    }

    return tp;
}


void tp_deinit(ThreadPool* tp) {
    if (!tp)
        return;

    pthread_mutex_lock(&(tp->task_mutex));
    // Empty the queue
    while (tp_task_queue_remove(&tp->task_queue, NULL) == 0);
    
    tp->running = false;
    pthread_cond_broadcast(&(tp->task_cond));
    pthread_mutex_unlock(&(tp->task_mutex));

    tp_wait(tp);

    pthread_mutex_destroy(&(tp->task_mutex));
    pthread_cond_destroy(&(tp->task_cond));
    pthread_cond_destroy(&(tp->wait_cond));

    tp_task_queue_deinit(&tp->task_queue);

    free(tp->threads);
    free(tp);
}


int tp_add_task(ThreadPool* tp, TaskFn f, void* args) {
    TPTask task = {
        .task = f,
        .args = args
    };

    if (!tp) return -1;

    pthread_mutex_lock(&(tp->task_mutex));
    if (tp_task_queue_insert(&tp->task_queue, task) == -1) {
       pthread_mutex_unlock(&(tp->task_mutex));
       return -1; 
    }
    pthread_cond_broadcast(&(tp->task_cond));
    pthread_mutex_unlock(&(tp->task_mutex));

    return 0;
}


void tp_wait(ThreadPool* tp) {
    if (!tp) return;

    pthread_mutex_lock(&(tp->task_mutex));
    while (1) {
        if ((tp->running && tp->working_count != 0) || 
            (!tp->running && tp->n_threads != 0)) {
            pthread_cond_wait(&(tp->wait_cond), &(tp->task_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tp->task_mutex));
}