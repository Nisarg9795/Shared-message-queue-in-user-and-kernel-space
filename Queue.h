/*Author: Vishwa Doshi && Nisarg Trivedi*/

#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include "rdtsc.h"

#define length 10 //Define the constant which is the size of the queue

#define DEVICE_1 "/dev/QUEUE1"
#define DEVICE_2 "/dev/QUEUE2"

#define LEFT 272
#define RIGHT 273

#define Q1 0
#define Q2 1

typedef struct LinkedList *node; //Define node as pointer of data type struct LinkedList

struct input_event ie;            // Structure to store mouse event values

const char *mDevice = "/dev/input/event3";//Storing path of mice device

node head = NULL;  //The first node of the linked list

float tick2time;  //Multiplier for queuing an ddequeuing time

unsigned long int msgsent=0,msgrecd=0; // Counters for message sent and received

int click = 0;


void* A1(void* arg);
void* A2(void* arg);
void* P1(void* arg);
void* P2(void* arg);
void* P3(void* arg);
void* P4(void* arg);
void* R(void* arg);

double pi(void);// Function to calculate the Pi value from Iterative algorithm



int isEmpty();  /*Declare functions to check the queue */
int isFull();

int fd1,fd2;  // File descriptors for devices

//Function for reading the time stamp counter
 

//Define structure with the 4 attributes of the message
struct message {
   unsigned long long int  msg_id;
   int  src_id;
   unsigned long long int que_t;
   double pi;
}*mptr;



//Make a queue and define all its attributes

void sq_create(int d)
  {
    if(d == Q1)
      fd1 = open(DEVICE_1,O_RDWR);
  
    else if (d == Q2)
      fd2 = open(DEVICE_2,O_RDWR);
      
  }

//Define the function to write in the queue
void sq_write(struct message *info, int f)
{
  int e;
  char *c = (char*)info;
  if(f == Q1)
    {
      e = write(fd1,c,sizeof(struct message));
      if(e<0)
        printf("Error in writing to Q1\n");
    }
  else if(f==Q2)
    {
      e = write(fd2,c,sizeof(struct message));
      if(e<0)
        printf("Error in writing to Q2\n");
    }

}



   

//Define the linked list structure for the read operation to use
struct LinkedList
{
    unsigned long long int time;
    struct LinkedList *next;
 };



node createNode(void)
{
    node temp; // declare a node
    temp = (node)malloc(sizeof(struct LinkedList)); // allocate memory using malloc()
    temp->next = NULL;// make next point to NULL
    return temp;//return the new node
}

node addNode(node head, unsigned long long int t)
{
    node temp,p;// declare two nodes temp and p
    temp = createNode();//createNode will return a new node with data = value and next pointing to NULL.
    temp->time = t; // add element's value to data part of node
    if(head == NULL){
        head = temp;     //when linked list is empty
    }
    else{
        p  = head;//assign head to p
        while(p->next != NULL){
            p = p->next;//traverse the list until p is the last node.The last node always points to NULL.
        }
        p->next = temp;//Point the previous last node to the new node created.
    }
    return head;
}


//Define the function for taking the elements out of the queue or reading
void sq_read(int k)
{   int b=2;
    struct message *a;
    char *d;
    a = (struct message*)malloc(sizeof(struct message));

   
	 while(b>0)
	 {
	   d=(char*)a;
	    
	   if(k== Q1)
	   { 
	   	b= read(fd1,d,sizeof(struct message)); 
	   }

	   else if(k == Q2)
	   {
	    b= read(fd2,d,sizeof(struct message));
	   }


	    if(b>0)
	    	{
	    		msgrecd++;
          a= (struct message*)d;
          a->que_t = (rdtsc() - a->que_t)/(tick2time * 1000000ul); //to find the difference between the queueing and dequeueing time
			    printf("Message Received -------> SRC- ID = %d ------>MSG-ID = %llu ---> Queueing Time = %llu -------------> bytes = %d\n\n",a->src_id,a->msg_id,a->que_t,b);
          head = addNode(head,a->que_t);  //Add nodes to the linked List
	      }
	   }

			
}



