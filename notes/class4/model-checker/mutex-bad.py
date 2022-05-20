class Mutex:
    locked = ''

    @thread
    def t1(self):
        while True:
            while self.locked == '🔒':
                pass
            self.locked = '🔒'# 这个代码之所以不对，在于观察锁与上锁是分开的两步，有可能两个线程都能看到没上锁
            cs = True
            del cs
            self.locked = ''

    @thread
    def t2(self):
        while True:
            while self.locked == '🔒':
                pass
            self.locked = '🔒'
            cs = True
            del cs
            self.locked = ''

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
