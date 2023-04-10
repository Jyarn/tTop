# Usage
`make` - run build task \
The program should only be run by launching the executable directly: \
./ttop.out .... \
Otherwise, the ctrl-z binding will not work since it sends the signal to make and not the program.

# Signal Handling
Entering:
`ctrl-c` - asks the user if they want to exit. Entering (y) exits the program (see note), entering (n) continues the program\
`ctrl-z` - does nothing

## Note about exiting from the signal handler
since `free` is not in the list of async signal safe functions, there is a chance out program could crash if we try to call free from within our signal handler, if we don't want to have any memory leaks when we exit then we would have to allocate everything on the stack, which is not very efficient since we have to replace all of our `malloc` calls with some kind of buffer that we pass in to the function; this would increase our memory usage and also we require me to change a lot of my original code base \
TL;DR \
When we exit from the signal handler it will probably leak memory but that memory is freed up by the OS so its fine

# Explanation of Output


# Implementation
sesPoll.* and memPoll.* are mostly unchanged \

## IPC
IPC is done using bi-directional pipes (biDirPipe), which is created using the `genChild` function.
`genChild` takes a "task", which is a function pointer to a function accepting a `biDirPipe*` and a `void*`. `genChild` then creates 2 pipes, mallocs a empty `biDirPipe` struct, forks and fills the struct. It then runs the child task passing the `void*` passed to `genChild` and the created `biDirPipe`.\
Writes to a `biDirPipe` are handled by the `writePacket` function, which writes the size of the object we are writing into memory and the object itself. This makes reading from the pipe signifcantly easier and more flexible \
Reads are handled by `readPacket`, which reads an int from the `biDirPipe`, mallocs a bff of with the size of the int we just read, and reads from the `biDirPipe` into the buffer

## Differences in polling
A large part of the changes are just making each of conncurent (the async family of functions), and compatible with the pipes (since they don't print directly to stdout).

### cpuPoll
I also changed the way cpu usage calculations are handled to match the assignment requirments. Active cpu is calculated first, then total cpu is calculated by adding idle cpu to active cpu. This is different from the assignment which calculates total cpu then subtracts idle from total to get active cpu\
The core count also matches the amount of cores and not the amount of logical cores. This is done something like `cat /proc/cpuinfo | grep "cpu cores"`, with of course of more processing afterwards to get the actual core count as a number. If this fails, it will use the logical cores \

### memPoll
Mostly unchanged \
Values are collected by reading and processing /proc/meminfo \
physical memory usage = MemTotal - MemFree - Buffers - Cached - SReclaimable + Shmem (used + shared in free) \
swap usage = SwapTotal - SwapFree \
virtual memory usage = physical memory usage + swap usage \
Total Virtual Memory = Total Physical Memory + Total Swap Memory

### sessPoll
Mostly unchanged \
User sessions are fetched using getutent to scroll through utmp. Entries are printed only if they are user processes and then printed formatted like: ut_user ut_line ut_host (if ut_host is empty, the contents of /proc/ut_pid/cmdline is printed) \
Output is similar to the `who` command

### sysPoll
Its functions were originally declared in main but was moved here mainly for readability

## Differences in printing
Printing is handled by 2 functions: `printSequential` and `printNotSequential` \
`printSequential` creates 4 pipes (for cpuPoll, memPoll, sessPoll, and sysPoll), reads from these pipes and prints them to stdout without escape characters \
`printNotSequential` is `printSequential` except instead of printing directly to stdout, we read from all of the pipes (except for sysPoll) and into a buffer, then print the buffer to stdout, and also printing the result from sysPoll at the end. `printNotSequential` also uses escape characters to jump back to the start and clear all of the lines below it.


# Functions

