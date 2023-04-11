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
This is an explanation of the `--graphics` flag, since other than the `--graphics` flag my ouput looks similar to the assignments

## Memory
every `+`, or `-` in 1 line of ouput represents the virtual memory usage as a percent, so \
`++++` is 4% virtual memory usage, and so is `----` \
`+` indicates an increase in memory usage, and `-` indicates a decrease in memory usage, relative to the previous poll

## CPU
each `|` represents a percent of cpu usage, so `||||` is 4% cpu usage \
at the end a `-` is printed as well the current cpu usage in brackets

## Non-sequential output
Since non-sequential ouput is buffered (read from pipes and store result in buffer), we essentially just print the contents of the buffers every poll, because of this the output might grow differently from what you would expect \
On Poll 1 ouput might look like this: \
mem use 1 \
+-----------+ \
sess use 1 \
+-----------+ \
cpu use 1 \
+-----------+ \
sys stats

On Poll 2 output grows downwards instead of having space on the console pre allocated: \
mem use 1 \
mem use 2 \
+-----------+ \
sess use 2 \
+-----------+ \
cpu use 1 \
cpu use 2 \
+-----------+ \
sys stats


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
## cpuPoll
`int fetchNCores ();` \
fetch the number of cores by reading /proc/cpuinfo \
\
`void async_processCPU_use (void* args, biDirPipe* pipe);` \
handlle CPU polling, output can be read from the biDirPipe \
\
`void printCPUHeader (CPUstats* prev, biDirPipe* pipe);` \
Print cpu usage \
prev = previous cpu usage stats, used to calculate the current cpu usage \
\
`double calculateCPUusage(CPUstats stats);` \
calculate cpu usage based on values provided by stats \
\
`double getCPUstats (CPUstats* prev);` \
prev = pointer to a CPUstat to be updated with new usage stats \
\
`void processCPU_use (CPUstats* prevStats, bool fancy, biDirPipe* pipe);`\
Print cpu stats with --graphics enabled or disabled \
prevStats = previous raw cpu usage stats, used to calculate current cpu stats (assumes cpu stats are upto date) \
fancy = specifies if anyting is to printed

## IPC
`void printStr (biDirPipe* pipe);` \
print a string from pipe \
\
`int writeStr (char* str, biDirPipe* pipe);` \
write a string to pipe \
\
`void* readPacket (biDirPipe* in);` \
read a packet from (biDirPipe*)in \
the first sizeof(int) bits determine package size and the rest is read and inputed into a buffer and returned \
\
`void killPipe (biDirPipe** pipe);` \
free all FDs in the biDirPipe \
and free the pipe, and set it *pipe to NULL \
\
`biDirPipe* genChild (job childTask, void* args);` \
fork wrapper \
the child   - runs the function pointed by childTask, and is passed args and a biDirPipe \
the parent  - exits genChild, and is given the other end of the biDirPipe \
\
After exiting the child expects an biDirPipe* [NPIPES] array, which \
are all the other biDirPipes allocated in the main program. This is done \
to prevent memory leaks (since the child receives a duplicate of the parents memory) \
\
## main
`void terminate (int signum);` \
signal handler the SIGUSR1 signal \
\
`void confirmExit (int signum);` \
signal handler for the ctrl-c signal asks the user if they wish to exit and accepts only 2 responses y, or n loops otherwise \
\
`int printHeader (unsigned int samples, unsigned int delay)'` \
Print the number of samples to be taken and their interval \
\
`void printSequential (bool fancy, char stats, unsigned int samples, unsigned int delay);` \
print based on the arguments passed to the program, this handles the case where sequential output is requested \
fancy 	- whether the --graphics flag has been requested \
stats 	- 0 - print system and user, 1 - print system only, 2 - print user only \
samples - number of poll(cycle) to perform before averaging out the results (assumed to be an unsigned int) \
delay 	- time in-between each poll in seconds (assumed to be an unsigned int) \
\
`void printNotSequential (bool fancy, char stats, unsigned int samples, unsigned int delay);` \
printSequential except the memory and cpu usage is buffered, to account for users that logging in which would  change where we place the cpu usage. Also uses escape characters unlike printSequential. \
Handles the case where the --sequential flag has not been specified \

## misc
`void colExtract (unsigned int* ret, int sz, char* bff);` \
extract integers in /proc files \
used in memPoll and cpuPoll \
\
`int buffFRead (char* buff, char* path, int sz);` \
try to read sz bytes of path into buff \
return how much of path has been read into buff \
\
`char* filterString (char* in, int sz);` \
filter all non-numerical characters in (char* in) so that it can be processed by colExtract. output should look something like /proc/[pid]/statm \
\
`void stringMult (char multend, int n, char* out);` \
equivalent to python's string multiplication thing \
ex. 'a' * 5 == aaaaa \
output is written into out, never checks if writing is inbounds \
also null terminates \
\
`int isNum (char* chk);` \
Check if chk is a number \
chk("70") == 1; chk("70sdsfd") == false; chk("slkdf98") == 0 \
\
`int strToInt (char* in);` \
strtol wrapper, output should be similar to strtol except it returns -1 if the first character is not a number \
in = our string (assumed to be null terminated) \
\
`int processFlag (int off, int argOff, int argc, char** argv);` \
process flags where an argument is required (--samples and --tdelay) \

## sessPoll
`void async_processSess_use (void* args, biDirPipe* pipe);` \
handle session polling, write output to biDirPipe \
\
`int printSessUse (biDirPipe* pipe);` \
read from pipe and write to stdout until it encounters a single '\0' character which exits and returns the number of lines printed \
\
`void fetchProcName (char* bff, int bffSz, int pid);` \
fetch commandline arguments for pid, return value will be written into bff and bytes written will be < bffSz \
always null terminated \
\
`sessInfo* processUTMP (struct utmp* u);` \
create a sessInfo struct and fill each entry with null terminated strings \
n.tty = ut_line \
n.user = ut_user \
n.host = ut_host \
n.procName = /proc/ut_pid/cmdline \
\
`int processSess_Use (biDirPipe* pipe);` \
process utmp entries \
only print user processes and print lines in this format: \
ut_user ut_line s, \
where s = ut_host if not NULL, otherwise it will be the string /proc/ut_pid/cmdline \
\
return number of lines printed to screen to be used by pollUse \
at the end send to the pipe a single '\0' character which indicates the end of the current poll \

## sysPoll
`void fetchSysInfo (biDirPipe* pipe);` \
fetch system info, output should be similar to uname -a except formatted better \
write ouput to biDirPipe \
\
`void async_processSys_stats (void* args, biDirPipe* pipe);` \
handle system info polling, output is written to passed biDirPipe \