#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#define NTHREAD 64
enum { T_FREE = 0, T_LIVE, T_DEAD, };
//线程结构体
//线程id
struct thread {
  int id, status;
  pthread_t thread;
  void (*entry)(int); //函数指针,如果没有括号 void *entry(int)声明的是一个返回空指针的函数,这里表示的是指针名叫entry，指向一个可以接收int参数,返回值为void的函数
};
//线程池，最多创建64个线程，有一个指针指向这个结构体数组的最后一个空项，等待创建，创建完毕后，tptr++
struct thread tpool[NTHREAD], *tptr = tpool;

//把输入的void指针转换成struct thread指针,线程函数把线程id作为参数接收
void *wrapper(void *arg) {
  struct thread *thread = (struct thread *)arg;
  thread->entry(thread->id);
  return NULL;
}

// create new thread, 
void create(void *fn) {
  assert(tptr - tpool < NTHREAD);
  *tptr = (struct thread) {
    .id = tptr - tpool + 1,
    .status = T_LIVE,
    .entry = fn,
  };
// pthread_create，第一个参数是返回给的新线程，传给tptr->thread，第二个参数不知道是啥，第三个参数是新线程要执行的函数，第四个参数是新函数的参数
// pthread_create执行的函数是wrapper，这个wrapper干的事情就是执行结构体中的entry指向的函数，而entry指向的则是外面的create传入的函数指针

  // 自定义stack size
  /*pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  pthread_attr_setstacksize(&thread_attr, 2*1024*1024);
  pthread_create(&(tptr->thread), &thread_attr, wrapper, tptr);
  pthread_attr_destroy(&thread_attr);*/
  pthread_create(&(tptr->thread), NULL, wrapper, tptr);
  ++tptr;
}

// 这个join，是主函数调用的，用于等待线程池中的所有线程执行完毕
void join() {
  // 遍历线程池，找到状态为T_LIVE的，等待他们
  for (int i = 0; i < NTHREAD; i++) {
    struct thread *t = &tpool[i];
    if (t->status == T_LIVE) {
      pthread_join(t->thread, NULL);
      t->status = T_DEAD;
    }
  }
}

__attribute__((destructor)) void cleanup() {
  join();
}
