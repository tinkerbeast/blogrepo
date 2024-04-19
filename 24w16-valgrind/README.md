Memory leaks and valgrind
=========================

### What valgrind can and cannot do

* Valgrind was meant for C/C++. It can only instrument and detect `managed` memory. Think `malloc` and `new`.
* From an OS perspective it detects heap erros. However it can't detect leaks directly done my `mmap`.
* For custom memory manager writeen on top of `malloc`, leak detection depends on implementation.
* Kernel buffers are definitely not tracked.

Types of leaks
--------------

### Definitely and still reachable

```
void *p;

int main()
{
    p = malloc(10); // *definitely lost* since p is overwritten.
    p = malloc(100); // *still reachable* since p is file scope var.
    void *m = malloc(50); // *definitely lost* since m is local and main exits
    return 0;
}
```
```
==8539== LEAK SUMMARY:
==8539==    definitely lost: 60 bytes in 2 blocks
==8539==    indirectly lost: 0 bytes in 0 blocks
==8539==      possibly lost: 0 bytes in 0 blocks
==8539==    still reachable: 100 bytes in 1 blocks
==8539==         suppressed: 0 bytes in 0 blocks
```

### Indirectly lost

```
struct node {
    void* val;
    struct node* next;
};

struct node* root;

void case_ll_with_no_head() {
    // Allocate a linked list of length 10.
    struct node sentinel;
    struct node* cur = &sentinel;
    for (int i = 0; i < 10; ++i) {
        cur->val = NULL;
        cur->next = malloc(sizeof(struct node));
        cur = cur->next;
    }
    cur->next = NULL;
    // We forget to keep a track of the head.
    //root = sentinel.next;
}
```
```
==8698== LEAK SUMMARY:
==8698==    definitely lost: 16 bytes in 1 blocks
==8698==    indirectly lost: 144 bytes in 9 blocks
==8698==      possibly lost: 0 bytes in 0 blocks
==8698==    still reachable: 0 bytes in 0 blocks
==8698==         suppressed: 0 bytes in 0 blocks
```

### Possibly lost

```
void case_ll_array() {
    // Allocate a linked list of length 10.
    root = malloc(sizeof(struct node) * 10);
    root[0].val = rand();
    for (int i = 1; i < 10; ++i) {
        root[i].val = rand();
        root[i-1].next = &root[i];
    }
    // Print it
    for (int i = 0; i < 9; ++i) { // NOTE: We moved 9 only.
        printf("%d ", root->val);
        ++root; // Possible loss is happening here.
    }
    printf("\n");

}
```
```
==8612== LEAK SUMMARY:
==8612==    definitely lost: 0 bytes in 0 blocks
==8612==    indirectly lost: 0 bytes in 0 blocks
==8612==      possibly lost: 160 bytes in 1 blocks
==8612==    still reachable: 0 bytes in 0 blocks
==8612==         suppressed: 0 bytes in 0 blocks
```

### Other cases to consider

* Circular lists.
* 2d lists and arrays.
* Memory pools.

Leaks example
-------------

### C++ producer consumer

```
struct Producer {
    uint64_t item = 0;

    [[nodiscard]]
    uint64_t* produce() {
        uint64_t* out = new uint64_t(item);
        ++item;
        return out;
    }
};

struct Consumer {
    void consume(uint64_t* in) {
        if (rand() % 100 < 50) { // 50% chance of memory-leak
            // do nothing (return, exception, etc)
        } else {
            delete in;
        }
    }
};


int main() {
    Producer p;
    Consumer c;

    while (true) {
        c.consume(p.produce());
    }
    return EXIT_SUCCESS;
}
```
```
==8832== LEAK SUMMARY:
==8832==    definitely lost: 39,632 bytes in 4,954 blocks
==8832==    indirectly lost: 0 bytes in 0 blocks
==8832==      possibly lost: 0 bytes in 0 blocks
==8832==    still reachable: 0 bytes in 0 blocks
==8832==         suppressed: 0 bytes in 0 blocks
```

### Python producer consumer

```
class Item:
    def __init__(self, i:int):
        self.i = i
        self.nex = None
        self.mem = list(range(10000))

class Producer:
    def __init__(self):
        self.item = 0;
    def produce(self):
        out = Item(self.item)
        self.item += 1
        #time.sleep(0.01)
        return out;

class Consumer:
    def consume(self, i):
        if random.randint(0, 100) % 100 < 50: # 50% chance of memory-leak
            x = Item(-1)
            x.nex = i
            i.nex = x
        else:
            pass # Python automatically garbage collects


if __name__ == '__main__':
    p = Producer()
    c = Consumer()
    while True:
        c.consume(p.produce())
```
```
==8919== LEAK SUMMARY:
==8919==    definitely lost: 0 bytes in 0 blocks
==8919==    indirectly lost: 0 bytes in 0 blocks
==8919==      possibly lost: 0 bytes in 0 blocks
==8919==    still reachable: 490,430 bytes in 113 blocks
==8919==         suppressed: 0 bytes in 0 blocks
```

Running massif:

```
valgrind --tool=massif python3 pymemleak.py
ms_print  massif.out.8938
```
```
    MB
37.33^                                                                      # 
     |                                                                @     #:
     |                                                      @     @@  @     #:
     |                                              @@      @     @   @     #:
     |                                       @@@@@  @       @     @   @     #:
     |                                       @ @ @  @       @     @ : @     #:
     |                       ::@@@ ::::::::::@ @ @  @       @   ::@ : @     #:
     |                  @   :::@  :: ::: ::: @ @ @  @       @   : @ : @     #:
     |      ::          @   :::@  :: ::: ::: @ @ @ :@   : ::@   : @ : @:::: #:
     |      :       ::  @   :::@  :: ::: ::: @ @ @ :@   : : @   : @ ::@: :  #:
     |   :: :   :   :   @  ::::@  :: ::: ::: @ @ @ :@   : : @:: : @ ::@: : :#:
     |   :  :   :   :   @::::::@  :: ::: ::: @ @ @ :@   : : @: :: @ ::@: : :#:
     |   :  :   :   :   @: ::::@  :: ::: ::: @ @ @ :@   ::: @: :: @ ::@: : :#:
     |   :  :  ::   :   @: ::::@  :: ::: ::: @ @ @::@ ::::: @: :: @ ::@: : :#:
     |   : @:  ::   : ::@: ::::@  :: ::: ::: @ @ @::@ : ::: @: :: @ ::@: : :#:
     |  :: @: ::::  : : @: ::::@  :: ::: ::: @ @ @::@ : ::: @: :: @ ::@: : :#:
     |  :: @: ::::  : : @: ::::@  :: ::: ::: @ @ @::@ : ::: @: :: @ ::@: : :#:
     |  :: @: ::::::: : @: ::::@  :: ::: ::: @ @ @::@ : ::: @: :: @ ::@: : :#:
     |  :: @: ::::: : : @: ::::@  :: ::: ::: @ @ @::@ : ::: @: :: @ ::@: : :#:
     |  :: @: ::::: : : @: ::::@  :: ::: ::: @ @ @::@ : ::: @: :: @ ::@: : :#:
   0 +----------------------------------------------------------------------->Gi
     0                                                                   570.2
```
```
99.94% (13,522,924B) (heap allocation functions) malloc/new/new[], --alloc-fns, etc.
->90.85% (12,293,147B) 0x22D2FA: PyMem_Malloc (in /usr/bin/python3.10)
| ->90.46% (12,240,000B) 0x285CEB: ??? (in /usr/bin/python3.10)
| | ->90.46% (12,240,000B) 0x2DA175: ??? (in /usr/bin/python3.10)
| | | ->90.46% (12,240,000B) 0x24B26C: _PyEval_EvalFrameDefault (in /usr/bin/python3.10)
| | |   ->90.46% (12,240,000B) 0x257C13: _PyObject_FastCallDictTstate (in /usr/bin/python3.10)
| | |     ->90.46% (12,240,000B) 0x26CA63: ??? (in /usr/bin/python3.10)
| | |       ->90.46% (12,240,000B) 0x258A1B: _PyObject_MakeTpCall (in /usr/bin/python3.10)
| | |         ->90.46% (12,240,000B) 0x251095: _PyEval_EvalFrameDefault (in /usr/bin/python3.10)
| | |           ->90.46% (12,240,000B) 0x2629FB: _PyFunction_Vectorcall (in /usr/bin/python3.10)
| | |             ->90.46% (12,240,000B) 0x24B45B: _PyEval_EvalFrameDefault (in /usr/bin/python3.10)
| | |               ->90.46% (12,240,000B) 0x2479C5: ??? (in /usr/bin/python3.10)
| | |                 ->90.46% (12,240,000B) 0x33D255: PyEval_EvalCode (in /usr/bin/python3.10)
| | |                   ->90.46% (12,240,000B) 0x368107: ??? (in /usr/bin/python3.10)
| | |                     ->90.46% (12,240,000B) 0x3619CA: ??? (in /usr/bin/python3.10)
| | |                       ->90.46% (12,240,000B) 0x367E54: ??? (in /usr/bin/python3.10)
| | |                         ->90.46% (12,240,000B) 0x367337: _PyRun_SimpleFileObject (in /usr/bin/python3.10)
| | |                           ->90.46% (12,240,000B) 0x366F82: _PyRun_AnyFileObject (in /usr/bin/python3.10)
| | |                             ->90.46% (12,240,000B) 0x359A5D: Py_RunMain (in /usr/bin/python3.10)
| | |                               ->90.46% (12,240,000B) 0x33002C: Py_BytesMain (in /usr/bin/python3.10)
| | |                                 ->90.46% (12,240,000B) 0x49D0D8F: (below main) (libc_start_call_main.h:58)
```

### Case 1: Python over C

```
// gcc -g -shared -rdynamic -o liballoc_only.so alloc_only.c 
#include <stdlib.h>

int do_alloc(int count, int size) {
    int leaked = 0;
    for (int i = 0; i < count; ++i) {
        void * mem = malloc(size);
        if (rand() % 100  < 50) { // leak 50%
            ++leaked;
        } else {
            free(mem);
        }
    }
    return leaked;
}
```
Python usage:
```
import ctypes
import random

c_lib = ctypes.CDLL('./liballoc_only.so')

if __name__ == '__main__':
    print(c_lib.do_alloc(10, 8))
```
```
==10835== LEAK SUMMARY:
==10835==    definitely lost: 32 bytes in 4 blocks
==10835==    indirectly lost: 0 bytes in 0 blocks
==10835==      possibly lost: 568 bytes in 1 blocks
==10835==    still reachable: 575,010 bytes in 167 blocks
==10835==         suppressed: 0 bytes in 0 blocks
```
```
==10937== 32 bytes in 4 blocks are definitely lost in loss record 1 of 127
==10937==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==10937==    by 0x4870187: do_alloc (alloc_only.c:6)
==10937==    by 0x4869E2D: ??? (in /usr/lib/x86_64-linux-gnu/libffi.so.8.1.0)
==10937==    by 0x4866492: ??? (in /usr/lib/x86_64-linux-gnu/libffi.so.8.1.0)
==10937==    by 0x585C3E8: ??? (in /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so)
==10937==    by 0x585B9FF: ??? (in /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so)
==10937==    by 0x258A7A: _PyObject_MakeTpCall (in /usr/bin/python3.10)
==10937==    by 0x251628: _PyEval_EvalFrameDefault (in /usr/bin/python3.10)
==10937==    by 0x2479C5: ??? (in /usr/bin/python3.10)
==10937==    by 0x33D255: PyEval_EvalCode (in /usr/bin/python3.10)
==10937==    by 0x368107: ??? (in /usr/bin/python3.10)
==10937==    by 0x3619CA: ??? (in /usr/bin/python3.10)
```
### Case 2: Python over C

C backend:
```
// gcc -g -shared -rdynamic -o liballoc_free.so alloc_free.c
#include <stdlib.h>

void* do_alloc(int size) {
    return malloc(size);
}

void do_free(void* mem)  {
    free(mem);
}
```
Python usage:
```
import ctypes
import random

c_lib = ctypes.CDLL('./liballoc_free.so')

class Item:
    def __init__(self, mem: ctypes.c_void_p):
        self.mem = mem

class Producer:
    def produce(self):
        out = Item(c_lib.do_alloc(32))
        return out;

class Consumer:
    def consume(self, i):
        if random.randint(0, 100) % 100 < 50: # 50% chance of memory-leak
            pass
        else:
            c_lib.do_free(i.mem)

if __name__ == '__main__':
    p = Producer()
    c = Consumer()
    for i in range(100):
        c.consume(p.produce())
```
```
==10873== LEAK SUMMARY:
==10873==    definitely lost: 1,600 bytes in 50 blocks
==10873==    indirectly lost: 0 bytes in 0 blocks
==10873==      possibly lost: 568 bytes in 1 blocks
==10873==    still reachable: 575,010 bytes in 167 blocks
==10873==         suppressed: 0 bytes in 0 blocks
```
```
==10896== 1,920 bytes in 60 blocks are definitely lost in loss record 104 of 127
==10896==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==10896==    by 0x4870154: do_alloc (alloc_free.c:4)
==10896==    by 0x4869E2D: ??? (in /usr/lib/x86_64-linux-gnu/libffi.so.8.1.0)
==10896==    by 0x4866492: ??? (in /usr/lib/x86_64-linux-gnu/libffi.so.8.1.0)
==10896==    by 0x585C3E8: ??? (in /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so)
==10896==    by 0x585B9FF: ??? (in /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so)
==10896==    by 0x258A7A: _PyObject_MakeTpCall (in /usr/bin/python3.10)
==10896==    by 0x251628: _PyEval_EvalFrameDefault (in /usr/bin/python3.10)
==10896==    by 0x2629FB: _PyFunction_Vectorcall (in /usr/bin/python3.10)
==10896==    by 0x24B45B: _PyEval_EvalFrameDefault (in /usr/bin/python3.10)
==10896==    by 0x2479C5: ??? (in /usr/bin/python3.10)
==10896==    by 0x33D255: PyEval_EvalCode (in /usr/bin/python3.10)
```

Appendix 1: Unkown symbols
--------------------------

```
==10896==    by 0x4869E2D: ??? (in /usr/lib/x86_64-linux-gnu/libffi.so.8.1.0)
==10896==    by 0x4866492: ??? (in /usr/lib/x86_64-linux-gnu/libffi.so.8.1.0)
==10896==    by 0x585C3E8: ??? (in /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so)
==10896==    by 0x585B9FF: ??? (in /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so)
```

Appendix 2: Valgrind commands
-----------------------------

Typical usage: `valgrind --leak-check=full --show-leak-kinds=all -v ./a.out`

`--trace-malloc=yes`  - Good for `malloc` to `free` correspondance.
`--track-origins=yes` - For uninited values.

Appendix 3: Blocks concept and memory overwrite
-----------------------------------------------

* What are blocks? TODO
* How can `memcpy` cause memory leak? TODO

Appendix 3: Refs
----------------

###  Manuals
https://valgrind.org/docs/manual/manual.html
https://valgrind.org/info/tools.html

### Types of leaks
https://developers.redhat.com/blog/2021/04/23/valgrind-memcheck-different-ways-to-lose-your-memory#

### TODO: Possible ways to detect pure python leaks
https://www.fugue.co/blog/diagnosing-and-fixing-memory-leaks-in-python.html
https://docs.python.org/3/library/tracemalloc.html
