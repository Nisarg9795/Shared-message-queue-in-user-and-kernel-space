# Shared-message-queue-in-user-and-kernel-space
Creates a kernel module for linux in Intel Galileo Gen-2 to make pseudo FIFO devices in kernel space; also contains a user program to enqueue and dequeue messages in queue devices.

CSE- 438

AUTHORS:  Team-1

Vishwakumar Dhaneshbhai Doshi (1213322381)

Nisarg Trivedi (1213314867)
_______________________________________________________________________________________________________________________________________________________________________________________________________

INCLUDED IN REPOSITORY:

-> User_prog.c (Program Source code)

-> squeue.c(Kernel module code)

-> Queue.h (library)

-> rdtsc.h (library)

-> Makefile 
_______________________________________________________________________________________________________________________________________________________________________________________________________

ABOUT: 

This project demonstrates a process including creation of periodic and aperiodic(mouse event triggered) pthreads, which use a character device driver to access two ring buffer data queues which in fact are implemented as characer devices in kernel space to store messages including calculated value of pi; as shared resources in critical section using Semaphore. Messages are dequeued and stored in user-space in a Linked-List. The enqueue and dequeue times of messages are also calculated using TSC and the proces undergoes a systematic termination sequence on occurrence of a double-click event. 
_______________________________________________________________________________________________________________________________________________________________________________________________________

SYSTEM REQUIREMENTS:

-> Linux CPU for host. Compilation must be done in host and object code is copied and run in Galileo board.

-> LINUX KERNEL : Minimum version of 2.6.19 is expected.

-> SDK: iot-devkit-glibc-x86_64-image-full-i586-toolchain-1.7.2

-> GCC: Minimum version of 4.8 is required to run -pthread option while compiling.

-> Intel Galileo Gen2

-> USB mouse
_______________________________________________________________________________________________________________________________________________________________________________________________________

SETUP:

->You are required to know the CPU clock speed(or clock frequency) of your processor in Giga-Hertz for accurate time calculations. To know that you can type in terminal:

           sudo cat /proc/cpuinfo

-> If your SDK is installed in other location than default one, change the path accordingly in makefile.

-> You must boot Galileo from SD card with linux version 3.19.

-> Open 2 terminal windows, one will be used for host commands and other will be used to send command to Galileo board.

-> Connect the FTDI cable and on one terminal type:

           sudo screen /dev/ttyUSB0 115200

 to communicate with board. This will be used for board comminication and is referred to as screen terminal, while other window is used for host commands. 

-> Connect ethernet cable and choose board ip address 192.168.1.100 and host ip address 192.168.1.105 (last number can be any from 0-255 but different from Host's number) by typing:

           ifconfig enp2s0 192.168.1.105 netmask 255.255.0.0 up (on host terminal)

           ifconfig enp0s20f6 192.168.1.100 netmask 255.255.0.0 up (on screen terminal)

-> You are required to know the /dev/input/event# number corresponding to mouse in board(generally event2 or event3). For that in screen terminal type:

           cd /dev/input
           ls by-path 

and note the event number corresponding to mouse event and change the path in mDevice variable Queue.h library accordingly.

_______________________________________________________________________________________________________________________________________________________________________________________________________

COMPILATION:

-> On the host, open directory in terminal in which files are present and type make.

-> To copy files to Galileo board type in host terminal:

           scp /home/vishva/FIFO_Driver/user_prog.o root@192.168.1.100:/home/vishva (only an example use your user name and corresponding path name)
 
           scp /home/vishva/FIFO_Driver/squeue.ko root@192.168.1.100:/home/vishva
 
 -> Now in screen terminal, go to the directory where we copied the object files and type:
 
           insmod squeue.ko
 
_______________________________________________________________________________________________________________________________________________________________________________________________________

EXECUTION:

->In screen terminal type:

           ./user_prog

 to run the code
_______________________________________________________________________________________________________________________________________________________________________________________________________

EXPECTED OUTPUT:

-> On running the object code, user will be prompted to enter CPU clock frequency in GHz. Enter the value in decimals (eg. 2.3) and press Enter. The screen now displays each message sent by sender thread unique source id and message id. Messages received will be displayed by the receiver thread additionally with a 'message queueing time' which is the time each message object spends in the queue. Not all the sent messages will be received due to dropping of messages because of high period of receiver thread. 

-> Along with four periodic threads, there are also two aperiodic threads which activate on left and right clicks respectively.

-> A double-click event is defined as two successive left clicks within 500ms. A double-click will trigger a termination sequence which includes all threads(except main()) exiting, followed by deletion of data queues; each printing a confirmation message on the screen. It then produces a summary of process in the form of:

Total messages sent

Total messages received

Mean and Standard Deviation of Queueing times for received threads 

-> After that the main thread exits and the process ends.
_______________________________________________________________________________________________________________________________________________________________________________________________________

ACKNOWLEDGEMENTS:

-> The iterative algorithm used to calculate the value of pi was Leibniz's algorithm from "https://www.codeproject.com/Articles/813185/Calculating-the-Number-PI-Through-Infinite-Sequenc".

-> The Linked List data-structure implementation, used to store message structure objects in an organised fashion; was highly inspired from "https://www.hackerearth.com/practice/data-structures/linked-list/singly-linked-list/tutorial/". 
