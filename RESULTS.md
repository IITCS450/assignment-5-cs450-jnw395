# Results

## Context-Switching Approach

- This implementation uses cooperative user-level threads on xv6 x86. Threads voluntarily yield the CPU by calling `thread_yield()`.

- Context switching is handled by `uswtch()`
    1. Saves the current thread's callee-saved registers (`ebp`, `ebx`, `esi`, `edi`) onto its stack and stores the stack pointer(sp).
    2. Loads the next thread's stack pointer.
    3. Restores its registers before returning.
    - On a thread's first run, `ret` lands in `thread_stub`, which calls the thread function with its arguments and marks the thread as ZOMBIE when it returns.
- The scheduling uses a round-robin policy: `pick_next()` searches forward from the current thread's index, wrapping around, and returns the first RUNNABLE thread it finds.

## Limitations

- **Maximum threads:** 8 (including the main thread), as `MAX_THREADS`.
- **Stack size:** Each thread gets a fixed 4096-byte stack. There is no guard page or overflow detection.
- **No preemption:** Threads must call `thread_yield()` explicitly. 
- **No priority:** The scheduler is strictly round-robin with no support for priorities or deadlines.
- **x86 only:** The `uswtch` assembly is written for 32-bit x86 and will not work on other architectures.
- **Single-core only:** Designed for xv6's uniprocessor environment with no synchronization of the thread table itself for multicore use.
