#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#define DEST  '+'
#define EMPTY '.'

struct move {
  int x, y, ch;
} moves[] = {
  { 0, 1, '>' },
  { 1, 0, 'v' },
  { 0, -1, '<' },
  { -1, 0, '^' },
};

char map[][512] = {
  "#######",
  "#.#.#+#",
  "#.....#",
  "#.....#",
  "#...#.#",
  "#######",
  "",
};

void display();

void dfs(int x, int y) {
  if (map[x][y] == DEST) {
    display();
  } else {
    int nfork = 0;

    for (struct move *m = moves; m < moves + 4; m++) {
      int x1 = x + m->x, y1 = y + m->y;
      if (map[x1][y1] == DEST || map[x1][y1] == EMPTY) {
        int pid = fork(); assert(pid >= 0);
        if (pid == 0) { // map[][] copied
          map[x][y] = m->ch;// 这里修改了map，相当于标记了visited，确保不会再次访问
          dfs(x1, y1);
          exit(0); // clobbered map[][] discarded
        } else {
          nfork++;
          waitpid(pid, NULL, 0); // wait here to serialize the search
        }
      }
    }

    while (nfork--) wait(NULL);
  }
}

int main() {
  dfs(1, 1);
}

void display() {
  for (int i = 0; ; i++) {
    for (const char *s = map[i]; *s; s++) {
      switch (*s) {
        case EMPTY: printf("   "); break;
        case DEST : printf(" ○ "); break;
        case '>'  : printf(" → "); break;
        case '<'  : printf(" ← "); break;
        case '^'  : printf(" ↑ "); break;
        case 'v'  : printf(" ↓ "); break;
        default   : printf("▇▇▇"); break;
      }
    }
    printf("\n");
    if (strlen(map[i]) == 0) break;
  }
  fflush(stdout);
  sleep(1); // to see the effect of parallel search
}
