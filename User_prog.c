// Author : Vishwakumar Doshi && Nisarg Trivedi
//Subject : Embedded system Programming
//Course code: CSE 438

/* The code makes two queues and 7 total threads out of which 2 are Aperiodic and 4 are Periodic
and one is the receiver thread. The Periodic threads have a defined run time and the Aperiodic
threads run when the Mouse event is recognised depending on which mouse click is done. Later the elements are
stored in the queue and read from them using POSIX threads. A Pi value is stored in the nodes with a few other
attributes. At last the program gives an average and standard deviation of the total time taken from queueing and dequeuing
of the values.*/

//Define the libraries and the global constants

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>
#include <fcntl.h>
#include "Queue.h"

#define BASE_PERIOD 1000// Base period of 1000 micro seconds
unsigned long long int MS_500;


//############################


//Defining Thread Source IDs:

#define A1_ID 1
#define A2_ID 2
#define P1_ID 3
#define P2_ID 4
#define P3_ID 5
#define P4_ID 6


// Defining Thread Priorities:

#define A_PRIORITY 10
#define P_PRIORITY 20
#define R_PRIORITY 30

const int p_period[4] = {12,32,18,28}; //The periods to be used for the implementation of the Periodic threads
const int r_period=40;

int fd; // Initialize the File Descriptor
int p_exit = 0;  // Variable which will be set to 1 on double click to initialize exit sequence
pthread_t tid[7]; //Thread id of the 7 threads that are to be made

void enqueue(int Q,int sid);

int bytes;


sem_t sem; // Semaphore variable that is to be made for making the queue thread safe
sem_t semL;//Semaphore variable to ensure aperiodic thread works only on left click
sem_t semR;//Semaphore variable to ensure aperiodic thread works only on right click

//##########################################
// List of function declaration to be executed by threads





//#########################################

 unsigned long long int rdtsc(void); // Function to read the time stamp Counter

int main(void)
  {
    sq_create(Q1); //Create the queues using the self made library
    sq_create(Q2);
    node trav;

    printf("What is your Processor's clock speed in GHz?\n"); // Prompting user to enter the CPU clock freq for accurate time Calculation
    scanf("%f",&tick2time);
    MS_500 = (tick2time/2) * 1000000000ul;
    double mean = 0.0,stdsum = 0.0,stddev = 0, msum = 0;




//Setting thread affinity to core 0
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0,&cpuset);

    int bytes;
    unsigned long long int t;
     
    sem_init(&sem,0,1); //Semaphore initialization of sem objects
    sem_init(&semL,0,1);
    sem_init(&semR,0,1);
    
    void* (*f[7])(void*) = {A1,A2,P1,P2,P3,P4,R}; //Pointer array to thread functions

    pthread_attr_t attr[7]; //Declaring the attribute array

    struct sched_param param; //Declaring structure object param of type sched_param

     

    // Open Mouse
    fd = open(mDevice,O_RDWR);

    if(fd == -1) //Because open returns -1 on occurence of error
      {
        printf("ERROR Opening %s\n", mDevice);
      }

       
   // #############################
// initialization of pthread attributes    

    for(int j=0;j<7;j++)
      {
         pthread_attr_init(&attr[j]);
         pthread_attr_setinheritsched(&attr[j], PTHREAD_EXPLICIT_SCHED);
         pthread_attr_setschedpolicy(&attr[j], SCHED_FIFO);
         if(j<2)
         	param.sched_priority=A_PRIORITY;
         else if((j>=2)&&(j!=6))
         	param.sched_priority = P_PRIORITY;
         else if(j==6)
         	param.sched_priority = R_PRIORITY;
         pthread_attr_setschedparam(&attr[j],&param);
         
      }
sem_wait(&semL); // Initially semaphores are locked and are unlocked only upon detection of clicks in the code below
sem_wait(&semR);
  
//################################################

// Pthread creation

    for(int i=0;i<7;i++)
      {
         pthread_create(&tid[i],&attr[i],*f[i],NULL);
         pthread_setaffinity_np(tid[i],sizeof(cpu_set_t),&cpuset);
      }

// ###############################################

      // Waiting for mouse clicks and detecting double click 


t=rdtsc();    
while(p_exit==0)
{
   bytes = read(fd, &ie, sizeof(struct input_event)); 

          if(bytes > 0)
          {
            click = ie.code;

           if ((click == LEFT) && (ie.value == 1))       
              {
                 if((rdtsc() - t) < MS_500) // Tests double-click
                	{
                	  p_exit = 1;
                	  sem_post(&semL);
                	  sem_post(&semR);
                	  break;
                	}
                 sem_post(&semL);
                 t=rdtsc();
               }

           else if((click == RIGHT) && (ie.value == 1))
           	 sem_post(&semR);
         }
}

//#####################################################################
  // START OF TERMINATION SEQUENCE
  //Function to join all the threds at the end of execution
  
  pthread_join(tid[6],NULL);

  sem_destroy(&sem);
  sem_destroy(&semL);
  sem_destroy(&semR);

  for(int y = 0; y< 180 ; y++)
     printf("#");

  printf("\n\nInitializing EXIT SEQUENCE :\n");
  
  printf("All threads terminated successfully\n\n");

  
  for(int y = 0; y< 180 ; y++)
     printf("#");

  printf("\nSUMMARY:\n");

  trav = head;
  while(trav->next != NULL)//Traversing of the LinkedList
  { 
    msum += trav->time;
    stdsum += (trav->time * trav->time);
    trav = trav->next; 
  }
  

  //Formulation of the mean and the standard devaition
  mean = msum/msgrecd;

  stddev = sqrt((stdsum/msgrecd) - (pow(mean,2)));

  for(int y = 0; y< 180 ; y++)
     printf("#");

  
  printf("\n\nTotal number of messages sent = %lu\n\n",msgsent);
  printf("Total number of messages received = %lu\n",msgrecd);

  printf("\nMean of Queueing times = %lf\n\nStandard deviation of Queueing times = %lf\n\n",mean,stddev);
  
  

  
  printf("Good Bye :)\n");
      return 0;
}

//############################################
//Different function definations of the threads: Periodic and Aperiodic

  void* A1(void* arg)
   {
     
     while(1)
      {
        sem_wait(&semL);
        if(p_exit!=0)
        {
        	sem_post(&semL);
        	break;
        }
        sem_wait(&sem);//Locks the Semaphore

                //Defining attributes of the message structure
        enqueue(Q1,A1_ID); 
        printf("A1 Sending message  ----> SRC- ID = %d ---- > MSG-ID = %llu\n\n",A1_ID,mptr->msg_id);
        free(mptr);
        sem_post(&sem);  //Unlock Semaphore
        
      }          
     
     pthread_exit(NULL);
   }

  void* A2(void* arg)
   {
     while(1)
      {
         // Read Mouse     
              sem_wait(&semR);
              if(p_exit!=0)
              	{
              		sem_post(&semR);
              		break;
              	}
              sem_wait(&sem);
              enqueue(Q2,A2_ID);
              printf("A2 Sending message  ----> SRC- ID = %d ---- > MSG-ID = %llu\n\n",A2_ID,mptr->msg_id);
              free(mptr);
              sem_post(&sem);
              
       }
      

     pthread_exit(NULL);
   }

//#################################################
//The 4 funcion defintions of Periodic Threads

  void* P1(void* arg)
   {
    struct timespec time,period; //Declaring objects of type timespec structure
    period.tv_sec=0;
    period.tv_nsec= p_period[0]*BASE_PERIOD*1000; //Defining the period in ns
    clock_gettime(CLOCK_MONOTONIC, &time); //Storing the current value of CLOCK_MONOTONIC in object time
    while(p_exit == 0)
       {
        time.tv_nsec+= period.tv_nsec; //Defining the absolute wake up time of thread
        if(time.tv_nsec > 1000000000l)
          {
            time.tv_sec++;
            time.tv_nsec=0;
          } 
          
        sem_wait(&sem);

        enqueue(Q1,P1_ID);
        printf("P1 Sending message  ----> SRC- ID = %d ---- > MSG-ID = %llu\n\n",P1_ID,mptr->msg_id);
        free(mptr);
                 
        sem_post(&sem);

        clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&time,0); //Takes time provided in time object as absolue time and thread goes to sleep
        }

      pthread_exit(NULL);
   }


  void* P2(void* arg)
     {
      struct timespec time,period;
      period.tv_sec=0;
      period.tv_nsec= p_period[1]*BASE_PERIOD*1000;

      clock_gettime(CLOCK_MONOTONIC, &time);

      while(p_exit == 0)
         {
          time.tv_nsec+= period.tv_nsec;
          if(time.tv_nsec > 1000000000l)
            {
              time.tv_sec++;
              time.tv_nsec=0;
            } 
            
          sem_wait(&sem);

          enqueue(Q1,P2_ID);
          printf("P2 Sending message  ----> SRC- ID = %d ---- > MSG-ID = %llu\n\n",P2_ID,mptr->msg_id);
          free(mptr);
                
          sem_post(&sem);

          clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&time,0);
         }

       pthread_exit(NULL);
     }

  void* P3(void* arg)
     {
      struct timespec time,period;
      period.tv_sec=0;
      period.tv_nsec= p_period[2]*BASE_PERIOD*1000;

      clock_gettime(CLOCK_MONOTONIC, &time);

      while(p_exit == 0)
         {
          time.tv_nsec+= period.tv_nsec;
          if(time.tv_nsec > 1000000000l)
            {
              time.tv_sec++;
              time.tv_nsec=0;
            } 
            
          sem_wait(&sem);

          enqueue(Q2,P3_ID);
          printf("P3 Sending message  ----> SRC- ID = %d ---- > MSG-ID = %llu\n\n",P3_ID,mptr->msg_id);
          free(mptr);
                
          sem_post(&sem);

          clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&time,0);
         }

       pthread_exit(NULL);
     }

  void* P4(void* arg)
     {
      struct timespec time,period;
      period.tv_sec=0;
      period.tv_nsec= p_period[3]*BASE_PERIOD*1000;

      clock_gettime(CLOCK_MONOTONIC, &time);

      while(p_exit == 0)
         {
          time.tv_nsec+= period.tv_nsec;
          if(time.tv_nsec > 1000000000l)
            {
              time.tv_sec++;
              time.tv_nsec=0;
            } 
            
          sem_wait(&sem);

          enqueue(Q2,P4_ID);
          printf("P4 Sending message  ----> SRC- ID = %d ---- > MSG-ID = %llu\n\n",P4_ID,mptr->msg_id);
          free(mptr);
                
          sem_post(&sem);

          clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&time,0);
         }

       pthread_exit(NULL);
     }

     //##########################################################
    //defintion of a receiver thread function

  void* R(void* arg)
     {
      struct timespec time,period;
      period.tv_sec=0;
      period.tv_nsec= r_period*BASE_PERIOD*1000;

      clock_gettime(CLOCK_MONOTONIC, &time);

      while(p_exit == 0)
         {
          time.tv_nsec+= period.tv_nsec;
          if(time.tv_nsec > 1000000000l)
            {
              time.tv_sec++;
              time.tv_nsec=0;
            } 
            
          
          sq_read(Q1); // reads the data queue and adds the message structure object members to linked list
          sq_read(Q2);

          
          clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&time,0);
         }

         sq_read(Q1); // reads the data queue and adds the message structure object members to linked list
         sq_read(Q2);

 //Function to join all the threads at the end of execution

       for(int k=0;k<6;k++)
       {
        pthread_join(tid[k],NULL);
       }

       pthread_exit(NULL);
     }


// Iterative algorithm for the calculation of Pi

  double pi(void)
     {
      double n, i;         // Number of iterations and control variable
       double pi = 4;

       n=(rand())%41 + 10; 

       for(i = 3; i <= (n + 2); i+=2)    
          {
            pi = pi * ((i - 1) / i) * (( i + 1) / i);
          }

       return pi;
     }

  void enqueue(int Q,int sid)
  {
    mptr = (struct message*)malloc(sizeof(struct message));
    mptr->msg_id = msgsent++;
    mptr->src_id = sid;
    mptr->pi = pi();
    mptr->que_t = rdtsc();
    sq_write(mptr,Q);
  }