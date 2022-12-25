#include <list.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <assert.h>

void
wakeup_proc(struct proc_struct *proc) {
    assert(proc->state != PROC_ZOMBIE && proc->state != PROC_RUNNABLE);
    proc->state = PROC_RUNNABLE;
}

// schedule函数会先清除调度标志，并从当前进程在链表中的位置开始，遍历进程控制块，直到找出处于就绪状态的进程。
// 之后执行proc_run函数，将环境切换至该进程的上下文并继续执行。
void
schedule(void) {
    bool intr_flag;
    list_entry_t *le, *last;
    struct proc_struct *next = NULL;
    local_intr_save(intr_flag); //设置内核栈地址与加载页目录项等这类关键操作不被中断给打断。
    {
        current->need_resched = 0;
        last = (current == idleproc) ? &proc_list : &(current->list_link);
        le = last;
        //遍历进程控制块，直到找出处于就绪状态的进程。
        do {
            if ((le = list_next(le)) != &proc_list) {
                next = le2proc(le, list_link); // get the struct from a ptr
                if (next->state == PROC_RUNNABLE) {
                    break;
                }
            }
        } while (le != last);
        if (next == NULL || next->state != PROC_RUNNABLE) {
            next = idleproc;
        }
        next->runs ++;
        if (next != current) {
            proc_run(next);
        }
    }
    local_intr_restore(intr_flag);
}

