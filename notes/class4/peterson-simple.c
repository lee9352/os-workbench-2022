#include "thread.h"

#define A 1
#define B 2

atomic_int nested;
atomic_long count;
//执行一会还是会触发assert，在于while好像判断不是原子的
//破案了，问题不是while，而是要用barrier确保执行while之前，对标志的修改都已经写入缓存
void critical_section() {
  long cnt = atomic_fetch_add(&count, 1);
  assert(atomic_fetch_add(&nested, 1) == 0);
  atomic_fetch_add(&nested, -1);
}

int volatile x = 0, y = 0, turn = A;

void TA() {
    while (1) {
/* PC=1 */  x = 1;
/* PC=2 */  turn = B;
// 加个barrier
__sync_synchronize();
/* PC=3 */  while (y && turn == B) ;
            critical_section();
/* PC=4 */  x = 0;
    }
}

void TB() {
  while (1) {
/* PC=1 */  y = 1;
/* PC=2 */  turn = A;
// 加个barrier
__sync_synchronize();
/* PC=3 */  while (x && turn == A) ;
            critical_section();
/* PC=4 */  y = 0;
  }
}

int main() {
  create(TA);
  create(TB);
}
