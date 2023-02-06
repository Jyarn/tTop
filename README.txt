Usage
 To run and build the project run:
  make
 To build the project run:
  make build
 
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
  
  Defaults:
    --samples=10 --tdelay=1
    
Implementation:
  Functionality is split into seperate modules:
    main - Main program responsible for process command line arguments and wrapping together all modules
    cpuPoll - Process CPU usage
    memPoll - Process memory usage
    misc - Miscellaneous functions
    sessPoll - Fetch user sessions
    
  Printing handled by pollUse, which decides how and what is to be printed the screen. Keeps track of the number of lines printed   
  to the screen and jumps up by to the start where printing first started.
  Command line argument processing is handled by looping through all of the arguments checking if they match the flags and 
  if needed are passed to processFlag which handles flags like --samples=N
  
  CPU usage is calculated by reading /proc/stat, and processing the numbers into and unsigned int array.
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
  Implementation generally follows the implementation of free in the free man page
  
  User sessions are fetched using getutent to scroll through utmp. Entries are printed only if they are user processes and
  then printed formatted like: ut_user ut_line ut_host (if ut_host is empty, the contents of /proc/ut_pid/cmdline is printed)
