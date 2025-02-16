#ifndef _thread_pool_h_
#define _thread_pool_h_

/* A simple thread pool

   [Rationalle]

   Thread creation is expensive, so it makes sense to create some threads and keep them around.
   These threads form a pool, which can do arbitrary "tasks" - calling a function with some data.
   In this way, the overhead of thread creation is only happens once, and simple tasks can be multithreaded easily.

   [Usage]

   //Create a thread pool (you can make more than one)
   //Matching the number of hardware threads is a good idea if you have a single pool
   ThreadPool* pool = tpCreate(number of threads);

   //Work size doesn't have to be the same as the number of threads
   //But note that the task dispatcher is quite dumb, and simply goes through the pool round-robin style
   //Therefore, if tasks have very different run times, the efficiency will plummet
   //Consider making multiple thread pools if this is the case
   for (int i = 0; i < work_size; ++i)
     tpRunTask(pool, workFunction, workData[i]);

   //Wait for the tasks to complete
   tpWaitOnAllTasks(pool);

   //Work is ready to use here
   ....

   //Destroy the pool (but remember, creation is expensive, so best to keep it around for a while)
   tpDestroy(pool);

   [Tips]

   Note that these functions aren't thread safe (oh the irony),
   so you should not create pools and schedule tasks on them from different threads.

   [Testing]

   This module was stress tested for:
   Memory leaks (many threads, many create/destroys)
   Race conditions (many tasks accessing the same data with mutex locks)

   Not tested (but should work):
   Multiple concurrent thread pools

*/

/* Function prototype for tasks */
typedef void (*ThreadPoolTask)(void*);


struct ThreadPool;



/* API Begins Here */

//Create a thread pool of X threads - note that this is slow (on the order of milliseconds per thread)
ThreadPool* tpCreate(int num_threads);

//Destroy a thread pool
void tpDestroy(ThreadPool* pool);

//Run a task - a non-busy thread from the pool will be given the task
void tpRunTask(ThreadPool* p, ThreadPoolTask task, void* data);

//Wait until all threads have completed their tasks - your output is fully valid after this call
void tpWaitOnAllTasks(ThreadPool* p);


#endif
