#include <stdlib.h>
#include "thread_pool.h"
#include "tinycthread/tinycthread.h"
#include <stdio.h>

#if 0
class wtf
{
private:
	boost::mutex mutex_;
	boost::condition_variable condition_;
	unsigned long count_;

public:
	semaphore()
		: count_()
	{}

	void notify()
	{
		boost::mutex::scoped_lock lock(mutex_);
		++count_;
		condition_.notify_one();
	}

	void wait()
	{
		boost::mutex::scoped_lock lock(mutex_);
		while (!count_)
			condition_.wait(lock);
		--count_;
	}
};
#endif

struct sem_t {
	mtx_t mutex;
	cnd_t cond;
	int count = 0;
};

void sem_init(sem_t* sem, int count) {
	mtx_init(&sem->mutex, mtx_plain);
	cnd_init(&sem->cond);
	sem->count = count;
}
void sem_destroy(sem_t* sem) {
	cnd_destroy(&sem->cond);
	mtx_destroy(&sem->mutex);
}
void sem_post(sem_t* sem) {
	mtx_lock(&sem->mutex);
	++sem->count;
	cnd_signal(&sem->cond);
	mtx_unlock(&sem->mutex);
}
void sem_wait(sem_t* sem) {
	mtx_lock(&sem->mutex);
	while (!sem->count)
		cnd_wait(&sem->cond, &sem->mutex);
	--sem->count;
	mtx_unlock(&sem->mutex);
}
void sem_value(sem_t* sem) {
	mtx_lock(&sem->mutex);
	while (!sem->count)
		cnd_wait(&sem->cond, &sem->mutex);
	mtx_unlock(&sem->mutex);
}


struct WorkerThreadData {
	sem_t run, done;
	volatile bool exit;

	ThreadPoolTask task;
	void* data;
};

struct ThreadPool {
	int num_threads;
	thrd_t* worker_threads;
	WorkerThreadData* worker_data;
	int curr_worker;
};

static int workerThread(void *data) {
	WorkerThreadData* d = (WorkerThreadData*)data;
	printf("Worker ready\n");

	while (true) {
		sem_wait(&d->run);

		if (d->exit)
			break;
		else
			d->task(d->data);

		sem_post(&d->done);
	}
	return 0;
}

ThreadPool* tpCreate(int num_threads) {
	if (num_threads <= 0) return NULL;
	ThreadPool* p = (ThreadPool*)malloc(sizeof(ThreadPool));
	p->num_threads = num_threads;
	p->worker_data = (WorkerThreadData*)malloc(num_threads*sizeof(WorkerThreadData));
	p->worker_threads = (thrd_t*)malloc(num_threads*sizeof(thrd_t));
	p->curr_worker = 0;
	for (int i = 0; i < num_threads; ++i) {
		sem_init(&p->worker_data[i].run, 0);
		sem_init(&p->worker_data[i].done, 1);
		p->worker_data[i].exit = false;

		p->worker_data[i].task = NULL;
		p->worker_data[i].data = NULL;

		thrd_create(&p->worker_threads[i], workerThread, &p->worker_data[i]);
	}
	return p;
}
void tpDestroy(ThreadPool* p) {
	for (int i = 0; i < p->num_threads; ++i) {
		WorkerThreadData& w = p->worker_data[i];
		sem_wait(&w.done);
		w.exit = true;
		sem_post(&w.run);
		thrd_join(p->worker_threads[i], NULL);
	}
	for (int i = 0; i < p->num_threads; ++i) {
		sem_destroy(&p->worker_data[i].run);
		sem_destroy(&p->worker_data[i].done);
	}
	free(p->worker_data);
	free(p->worker_threads);
	free(p);
}
void tpRunTask(ThreadPool* p, ThreadPoolTask task, void* data) {
	WorkerThreadData& w = p->worker_data[p->curr_worker];
	sem_wait(&w.done);
	w.task = task;
	w.data = data;
	sem_post(&w.run);
	p->curr_worker = (p->curr_worker+1)%p->num_threads;
}
void tpWaitOnAllTasks(ThreadPool* p) {
	for (int i = 0; i < p->num_threads; ++i) {
		WorkerThreadData& w = p->worker_data[i];
		sem_value(&w.done);
	}
}
