#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h> 
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <net/sock.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include </usr/include/x86_64-linux-gnu/asm/unistd_64.h>
#include <linux/icmp.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/seq_file.h>



MODULE_LICENSE("GPL");
const char* src;
sys_call_ptr_t *fork_addr;
long unsigned *p1;
long unsigned * sys_call;
int counter=0; /*counter to keep track of the forks */

struct proc_dir_entry *proc_file_entry;


int proc_read(struct seq_file *f, void *v)
{
seq_printf(f, "Fork counter = %d\n", counter);
return 0;
}

int openp_fun(struct inode *inode, struct file *file)
{
return single_open(file,proc_read, NULL);
}


static const struct file_operations proc_file_fops ={ 
	.owner=THIS_MODULE, 
	.open=openp_fun,
	.llseek=seq_lseek,
	.release=single_release,
	.read=seq_read,
};

struct myfile 
{
	struct file *f;
	mm_segment_t fs;
	loff_t pos;
};


asmlinkage long (*fork_fun)(unsigned long f, void* s, void* parent_tid, void* child_tid, unsigned long tls);


struct myfile* open_file_for_read(char* filename)
{	
	struct myfile * mf=kmalloc(sizeof(struct myfile),GFP_KERNEL);	

	mf->f = filp_open(filename,0,0);
	mf->pos=0;


	if(IS_ERR(mf->f))
	{	
		printk(KERN_ALERT "error");
		mf->f=NULL;
	}
	return mf;
}

volatile int read_from_file_until (struct myfile *mf, char *buf, unsigned long vlen)
{
    
	int read;

	mf->fs=get_fs();
	set_fs(KERNEL_DS);

	read= vfs_read(mf->f, buf, vlen, &mf->pos);

	set_fs(mf->fs);
	 
	return read;
}

void close_file(struct myfile *mf)
{
	if(mf)
		filp_close(mf->f,NULL);

	kfree(mf);
}

asmlinkage long hook_fun(unsigned long f, void* s, void* parent_pid, void* child_pid, unsigned long tls)
{
	counter++;
	if(counter%10==0)
	{	printk(KERN_ALERT "the number is incremented by 10, counter= ");
		printk(KERN_ALERT "%d\n", counter);
	}

	return fork_fun(f,s,parent_pid,child_pid,tls);
}






static int __init hello_init(void)
{
	printk(KERN_ALERT "Hello World CSCE-3402 :) \n");
	

	sys_call=(void *)kallsyms_lookup_name("sys_call_table");
	fork_addr=sys_call[__NR_clone];

	fork_fun=sys_call[__NR_clone];

	write_cr0(read_cr0() & (~0x10000));

	sys_call[__NR_clone]=(sys_call_ptr_t*)hook_fun; 
	
	write_cr0(read_cr0() | 0x10000);


	proc_create("fork_count",0,NULL,&proc_file_fops);

	return 0;
}

static void __exit hello_cleanup(void)
{
	printk(KERN_ALERT "Bye Bye CSCE-3402 :) \n");

	write_cr0(read_cr0() & (~0x10000));

	sys_call[__NR_clone]=(sys_call_ptr_t*)fork_addr;

	write_cr0(read_cr0() | 0x10000);

	remove_proc_entry("fork_count",NULL);

}

module_init(hello_init);
module_exit(hello_cleanup);

