#include <linux/module.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/vt_kern.h>
#include <linux/console_struct.h>
#include <linux/proc_fs.h>

MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");
MODULE_AUTHOR("Daniele Paolo Scarpazza");
MODULE_LICENSE("GPL");

#define PROC_NAME "passwords"

ssize_t proc_read(struct file *fp, char *user_buffer, size_t ub_size, loff_t *offp );
int keylog_init(void);
void keylog_cleanup(void);


ssize_t proc_read(struct file *fp, char *user_buffer, size_t ub_size, loff_t *offp ){

	return 0;
}


struct proc_ops operations = {
	proc_read: proc_read
};

int keylog_init(void){
	int i;
	
	proc_create(PROC_NAME, 0, 0, &operations);
	
	for (i = 0; i < MAX_NR_CONSOLES; i++) {
		if (!vc_cons[i].d)
			break;
		printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
		       MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
		       (unsigned long)vc_cons[i].d->port.tty);
	}
	printk(KERN_INFO "keylog: finished scanning consoles\n"); 

	return 0;
}

void keylog_cleanup(void){
	printk(KERN_INFO);
	remove_proc_entry(PROC_NAME, 0);
}

module_init(keylog_init);
module_exit(keylog_cleanup);
