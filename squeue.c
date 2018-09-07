/* ----------------------------------------------- DRIVER gmem --------------------------------------------------
 
 Basic driver example to show skelton methods for several file operations.
 
 ----------------------------------------------------------------------------------------------------------------*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>

#include<linux/init.h>
#include<linux/moduleparam.h>

#define DEVICE1_NAME                 "QUEUE1"  // device name to be created and registered
#define DEVICE2_NAME                 "QUEUE2"
#define DEVICE_NAME                  "QUEUE"
#define length                       10

/* per device structure */

struct queue_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	int f;
   int r;
   struct message *m[length];
} *queue1_devp,*queue2_devp;

struct message {
   unsigned long long int  msg_id;
   int  src_id;
   unsigned long long int que_t;
   double pi;
}*mptr;

static dev_t queue_dev_number;      /* Allotted device number */
struct class *queue_dev_class;          /* Tie with the device model */
static struct device *queue1_dev_device,*queue2_dev_device;


/*
* Open gmem driver
*/
int queue_driver_open(struct inode *inode, struct file *file)
{
	struct queue_dev *queue_devp;
//	printk("\nopening\n");

	/* Get the per-device structure that contains this cdev */
	queue_devp = container_of(inode->i_cdev, struct queue_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = queue_devp;
	printk("\n%s is openning \n", queue_devp->name);
	return 0;
}

/*
 * Release gmem driver
 */
int queue_driver_release(struct inode *inode, struct file *file)
{
	struct queue_dev *queue_devp = file->private_data;
	
	printk("\n%s is closing\n", queue_devp->name);
	
	return 0;
}


int isEmpty(struct queue_dev *queue_devp)
{
  if(queue_devp->f == queue_devp->r)
    return -1;
  else
    return 0;
}
/*End of isEmpty()*/


int isFull(struct queue_dev *queue_devp)
{
  if(queue_devp->f == (queue_devp->r + 1)%length) //To define whether the queue is full or not by comparing the front and rear pointers
    return -1;
  else
    return 0;
}

/*
 * Write to gmem driver
 */
ssize_t queue_driver_write(struct file *file, const char *buff, size_t count, loff_t *ppos)
{
	struct queue_dev *queue_devp = file->private_data;
	struct message *ms = kmalloc(sizeof(struct message),GFP_KERNEL);
  
  copy_from_user(ms,(struct message*)buff,sizeof(struct message));
	
  if(isFull(queue_devp) == -1)
    printk(KERN_ALERT"Queue is Full!\n\n");

	queue_devp->m[queue_devp->r] = ms;

  if(isFull(queue_devp) == -1)	
  		queue_devp->f = (queue_devp->f + 1)%length;

  queue_devp->r = (queue_devp->r + 1)%length;
  
  printk(KERN_INFO "squeue Received message from the user\n");
  return 0;
}
/*
 * Read to gmem driver
 */
ssize_t queue_driver_read(struct file *file, char *buff, size_t count, loff_t *ppos)
{
	struct queue_dev *queue_devp = file->private_data;

	if(isEmpty(queue_devp) == -1)
   { 
    printk(KERN_ALERT"Queue is Empty!\n\n"); 
    return 0;
   }

  
    copy_to_user(buff,(char*)queue_devp->m[queue_devp->f],sizeof(struct message));

    kfree(queue_devp->m[queue_devp->f]);

    queue_devp->f = (queue_devp->f + 1)%length;

    if(queue_devp->f > queue_devp->r)
		return(length - (queue_devp->f - queue_devp->r));
	else 
		return(queue_devp->r - queue_devp->f); 

}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations queue_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= queue_driver_open,        /* Open method */
    .release	= queue_driver_release,     /* Release method */
    .write		= queue_driver_write,       /* Write method */
    .read		= queue_driver_read,        /* Read method */
};

/*
 * Driver Initialization
 */
int __init queue_driver_init(void)
{
	int ret;
	

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&queue_dev_number, 0, 2, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	queue_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* Allocate memory for the per-device structure */
	queue1_devp = kmalloc(sizeof(struct queue_dev), GFP_KERNEL);
	if (!queue1_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	queue2_devp = kmalloc(sizeof(struct queue_dev), GFP_KERNEL);
		
	if (!queue2_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	queue1_devp->f=0;
	queue1_devp->r=0;
	queue2_devp->f=0;
	queue2_devp->r=0;

	/* Request I/O region */
	sprintf(queue1_devp->name, DEVICE1_NAME);
	sprintf(queue2_devp->name, DEVICE2_NAME);

	/* Connect the file operations with the cdev */
	cdev_init(&queue1_devp->cdev, &queue_fops);
	cdev_init(&queue2_devp->cdev, &queue_fops);
	queue1_devp->cdev.owner = THIS_MODULE;
	queue2_devp->cdev.owner = THIS_MODULE;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&queue1_devp->cdev, MKDEV(MAJOR(queue_dev_number),0), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	ret = cdev_add(&queue2_devp->cdev, MKDEV(MAJOR(queue_dev_number),1), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	queue1_dev_device = device_create(queue_dev_class, NULL, MKDEV(MAJOR(queue_dev_number), 0), NULL, DEVICE1_NAME);
	queue2_dev_device = device_create(queue_dev_class, NULL, MKDEV(MAJOR(queue_dev_number), 1), NULL, DEVICE2_NAME);		
	// device_create_file(gmem_dev_device, &dev_attr_xxx);

	
	printk("queue driver initialized.\n");// '%s'\n",gmem_devp->in_string);
	return 0;
}
/* Driver Exit */
void __exit queue_driver_exit(void)
{
	// device_remove_file(gmem_dev_device, &dev_attr_xxx);
	/* Release the major number */
	unregister_chrdev_region((queue_dev_number), 1);

	/* Destroy device */
	device_destroy (queue_dev_class, MKDEV(MAJOR(queue_dev_number), 0));
	device_destroy (queue_dev_class, MKDEV(MAJOR(queue_dev_number), 1));
	cdev_del(&queue1_devp->cdev);
	cdev_del(&queue2_devp->cdev);
	kfree(queue1_devp);
	kfree(queue2_devp);
	
	/* Destroy driver_class */
	class_destroy(queue_dev_class);

	printk("queue driver removed.\n");
}



module_init(queue_driver_init);
module_exit(queue_driver_exit);
MODULE_LICENSE("GPL v2");
