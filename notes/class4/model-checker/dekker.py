class Dekker:
    flag = [False, False]
    turn = 0

    @thread
    def t1(self):
        this, another = 0, 1
        while True:
            self.flag[this] = True #先举旗
            while self.flag[another]: #看对面是否举旗
                if self.turn == another: #再看门牌给谁
                    self.flag[this] = False #放下棋子
                    while self.turn == another:
                        pass
                    self.flag[this] = True #对面执行完毕，再举旗
            cs = True
            del cs
            self.turn = another
            self.flag[this] = False
  

    @thread
    def t2(self):
        this, another = 1, 0
        while True:
            self.flag[this] = True
            while self.flag[another]:
                if self.turn == another:
                    self.flag[this] = False
                    while self.turn == another:
                        pass
                    self.flag[this] = True
            cs = True
            del cs
            self.turn = another
            self.flag[this] = False

    @marker
    def mark_t1(self, state):
        if localvar(state, 't1', 'cs'): return 'blue'

    @marker
    def mark_t2(self, state):
        if localvar(state, 't2', 'cs'): return 'green'

    @marker
    def mark_both(self, state):
        if localvar(state, 't1', 'cs') and localvar(state, 't2', 'cs'):
            return 'red'
