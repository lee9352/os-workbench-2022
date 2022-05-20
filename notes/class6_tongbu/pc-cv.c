#include "thread.h"
#include "thread-sync.h"

int n, count = 0;
mutex_t lk = MUTEX_INIT();
//cond_t cv = COND_INIT();
cond_t cv_pro = COND_INIT();
cond_t cv_con = COND_INIT();

void Tproduce() {
  while (1) {
    mutex_lock(&lk);
    //if (count == n) {
    // if需要改成while
    // 目的是被唤醒之后再检查一次条件是否满足
    while (!(count < n)) {
      //cond_wait(&cv, &lk);
      cond_wait(&cv_pro, &lk);
    }
    printf("("); count++;
    //cond_signal(&cv);
    cond_broadcast(&cv_con);
    mutex_unlock(&lk);
  }
}

void Tconsume() {
  while (1) {
    mutex_lock(&lk);
    //if (count == 0) {
    // if需要改成while
    while (!(count > 0)) {
      //pthread_cond_wait(&cv, &lk);
      cond_wait(&cv_con, &lk);
    }
    printf(")"); count--;
    //cond_signal(&cv);
    cond_broadcast(&cv_pro);
    mutex_unlock(&lk);
  }
}

int main(int argc, char *argv[]) {
  assert(argc == 2);
  n = atoi(argv[1]);
  setbuf(stdout, NULL);
  for (int i = 0; i < 8; i++) {
    //只有一组生产者消费者时，不会报错
    create(Tproduce);
    create(Tconsume);
  }
}
