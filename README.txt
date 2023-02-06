Usage
 To run and build the project run:
  make

 To build the project run:
  make build

 To run the project:
  ./ttop.out

 Arguments:
  --sequential
  --graphics
  --system
  --user
  --samples=N
  --tdelay=T
  --DEBUG

  --DEBUG:
    print processed command line arguments

  Positional arguments:
  The last 2 arguments are positional arguments (samples tdelay).
  If there is only 1 positional argument then that will specify how many samples that are to be taken
  Positional arguments are accepted iff --samples or --tdelay is not specified

  Defaults:
    --samples=10 --tdelay=1

  Ex.
    ./ttop.out --graphical --user --sequential 100 1

  Specifying values for --samples and --tdelay:
    --samples=100
    --samples 100
    --samples100
    --samples= 100

Implementation:
  Functionality is split into seperate modules:
    main - Main program responsible for process command line arguments and wrapping together all modules
    cpuPoll - Process CPU usage
    memPoll - Process memory usage
    misc - Miscellaneous functions
    sessPoll - Fetch user sessions

  Printing is handled by pollUse, which decides how and what is to be printed the screen. jump keeps track of the number
  of lines printed to the screen and is used to jump up by back to the start.
  Command line argument processing is handled by looping through all of the arguments checking if they match the flags and
  if needed are passed to processFlag which handles flags like --samples=N

  CPU usage is calculated by reading in /proc/stat, and processing the numbers into and unsigned int array.
  active cpu time  = user + nice + system + irq + softirq
  total cpu time =  = user + nice + system + irq + softirq + idle + iowait
  These are subtracted by the last time we calculcated the cpu time, divided and then multiplied by 100:
    cpu usage = ((active cpu time - previous cpu time) / (total cpu time - previous total cpu time)) * 100

  steal, guest, guest_nice are not factored in because it comes from virtualization and is not technically from the
  current operating system. CPU usage seems to match how polybar calculates its CPU usage

  General memory usage is calculated by processing /proc/meminfo
  physical memory usage = MemTotal - MemFree - Buffers - Cached - SReclaimable + Shmem (used + shared in free)
  swap usage = SwapTotal - SwapFree
  virtual memory usage = physical memory usage + swap usage
  Total (Physical, Swap) Memory are calculated by reading in their respective fields
  Total Virtual Memory = Total Physical Memory + Total Swap Memory
  Implementation generally follows the implementation of free in the free man page except when calculating physical used

  User sessions are fetched using getutent to scroll through utmp. Entries are printed only if they are user processes and
  then printed formatted like: ut_user ut_line ut_host (if ut_host is empty, the contents of /proc/ut_pid/cmdline is printed)
  Output follows who

Function Documentation:
  double calculateCPUusage(CPUstats stats)
    calculate cpu usage based on values provided by stats

  double getCPUstats (CPUstats* prev)
    prev = pointer to a CPUstat to be updated with new usage stats

  int processCPU_use (CPUstats* prevStats, bool fancy)
    Print cpu stats with --graphics enabled or disabled
	  prevStats = previous raw cpu usage stats, used to calculate current cpu stats (assumes cpu stats are upto date)
	  fancy = specifies if anyting is to printed
	  returns 1

  int fetchSysInfo ()
    fetch system info, output should be similar to uname -a except formatted better


  void curJump (int l, bool sequential)
    sequential = allows use of escape codes (true = no escape codes, false = use escape codes)
	  move cursor up or down depending on the sign of l iff sequential == false
	  does nothing if sequential == true

  int printCPUHeader (CPUstats* prev)
    Print cpu usage
	  prev = previous cpu usage stats, used to calculate the current cpu usage

  int printHeader (bool sequential, bool fancy, char stat, unsigned int samples, unsigned int delay, bool debug)
    Print the number of samples to be taken and their interval
	  If in debug mode print arguments passed to pollUse

  void pollUse (bool sequential, bool fancy, char stats, unsigned int samples, unsigned int delay, bool debug)
    Main loop for printing to screen. Takes in a series of arguments
    sequential = print without escapes codes as though output is being redirected into a file
    fancy = --graphics (print all data)
    stats = 0 - print system and user, 1 - print system only, 2 - print user only
    samples = number of poll(cycle) to perform before averaging out the results (assumed to be an unsigned int)
    delay = time in-between each poll in seconds (assumed to be an unsigned int)
    debug = print command line arguments

  int strToInt (char* in)
    strtol wrapper, output should be similar to strtol except it returns -1 if the first character is not
	  a number
	  in = our string (assumed to be null terminated)

  int processFlag (int argLen, int argPos, int argc, char** argv)
    process flags where an argument is required (--samples and --tdelay)


  memstat - store usage and total memory usage for physical, swap and virtual

  memstat* fetchMemStats ()
    report RAM, swap, and virtual memory usage in gigabytes, returns a memstat* containing this information

  void processMem_use (memstat** prev, bool fancy)
    print memory usage and if requested a bar representing the percentage of memory being used and the change in memory use
    this additional information is specified by fancy


  void colExtract (unsigned int* ret, int sz, char* bff)
    extract integers in /proc files
    used in memPoll and cpuPoll

  int buffFRead (char* buff, char* path, int sz)
    try to read sz bytes of path into buff
    return how much of path has been read into buff

  char* filterString (char* in, int sz)
    filter all non-numerical characters in (char* in)
    so that it can be processed by colExtract.
    output should look something like /proc/[pid]/statm
    or like:
    1 2 3 4 5 6

  void stringMult (char multend, int n, char* out)
    equivalent to python's string multiplication thing
	  ex. 'a' * 5 == aaaaa
	  output written into out, never checks if writing is inbounds
	  also null terminates


  void fetchProcName (char* bff, int bffSz, int pid)
    fetch commandline arguments for pid,
    return value will be written into bff and bytes written will be < bffSz
    always null terminated

  sessInfo* processUTMP (struct utmp* u)
    create a sessInfo struct and fill each entry with null terminated strings
    n.tty = ut_line
    n.user = ut_user
    n.host = ut_host
    n.procName = /proc/ut_pid/cmdline

  int processSess_Use ()
    process utmp entries
    only print user processes and print lines in this format:
    ut_user ut_line s,
    where s = ut_host if not NULL, otherwise it will be the string /proc/ut_pid/cmdline

    return number of lines printed to screen to be used by pollUse