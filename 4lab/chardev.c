#include <linux/fs.h> //register_chrdev
#include <linux/init.h> // macros 
#include <linux/module.h>
#include <linux/kernel.h> // pr_info()
#include <linux/device.h> // for creating device
#include <asm/uaccess.h>  // Required for the copy to user function
#define SUCCESS 0
#define DEVICE_NAME "mychardev" /* Dev name as it appears in /proc/devices   */ 
#define BUF_LEN 80 /* Max length of the message from the device */ 

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Creates a char device that says how many times you have read from the dev file and returns ");
MODULE_AUTHOR("Morozov Nikita");

static int major; /* major number assigned to our device driver */ 
static char msg[BUF_LEN + 1]; /* The msg the device will give when asked */ 
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); /* Is device open? Used to prevent multiple access to device */ 
static struct class *cls;

// Prototypes for the chardev
static int device_open(struct inode *, struct file *); 
static int device_release(struct inode *, struct file *); 
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *); 
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *); 

// The file_operations structure from /linux/fs.h
static struct file_operations chardev_fops = { 
    .read = device_read, 
    .write = device_write, 
    .open = device_open, 
    .release = device_release, 
}; 

static int __init chardev_init(void) 
{ 
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops); 

    if (major < 0) { 
        pr_alert("Registering char device failed with %d\n", major); 
        return major; 
    } 
    
    pr_info("I was assigned major number %d.\n", major); 

    cls = class_create(DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return major; 
} 

static void __exit chardev_exit(void)
{ 
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, DEVICE_NAME); 
} 

/* Methods */ 

/* Called when a process tries to open the device file, like 
 * "sudo cat /dev/chardev" */ 
static int device_open(struct inode *inode, struct file *file)
{ 
    static int counter = 0; 

    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 

    printk(KERN_INFO "I already told you %d times Hello world!\n", counter++); 
    try_module_get(THIS_MODULE); 
    return SUCCESS; 
} 

/* Called when a process closes the device file. */ 
static int device_release(struct inode *inode, struct file *file)
{  
    atomic_set(&already_open, CDEV_NOT_USED); 

    module_put(THIS_MODULE); 
    return SUCCESS; 
} 

/* Called when a process, which already opened the dev file, attempts to 
 * read from it. 
 */ 
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */ 
                           char __user *buffer, /* buffer to fill with data */ 
                           size_t length, /* length of the buffer     */ 
                           loff_t *offset) { /* Number of bytes actually written to the buffer */ 
   /* Number of bytes actually written to the buffer */
   int bytes_read = 0;
    const char *msg_ptr = msg;
   
    if (!*(msg_ptr + *offset)) { /* we are at the end of message */
    *offset = 0; /* reset the offset */
    return 0; /* signify end of file */
    }
   
    msg_ptr += *offset;
   
    /* Actually put the data into the buffer */
    while (length && *msg_ptr) {
    /* The buffer is in the user data segment, not the kernel
    * segment so "*" assignment won't work. We have to use
    * put_user which copies data from the kernel data segment to
    * the user data segment.
    */
    put_user(*(msg_ptr++), buffer++);
    length--;
    bytes_read++;
    }
   
    *offset += bytes_read;
   
    /* Most read functions return the number of bytes put into the buffer. */
    return bytes_read;
} 

/* Called when a process writes to dev file: echo "hi" > /dev/hello */ 
static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{ 
    int ind = len - 1;
    int bytes_read = 0;
    memset(msg, '\0', BUF_LEN);
    char tmp[BUF_LEN];
    const char *msg_ptr = msg;

    while (len && msg_ptr)
    {
        tmp[bytes_read++] = buff[ind--];
        len--;
    }
    
    // reverse string
    for (int i = 0; i < bytes_read; i++) 
        // swapping characters
        msg[i] = tmp[bytes_read - i - 1];
    
    return bytes_read;
} 

module_init(chardev_init); 
module_exit(chardev_exit); 
