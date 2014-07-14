#ifndef _UTILS_LIST_H
#define _UTILS_LIST_H

/**
 * Generic linked list implementation.
 * All lists are considered as circular linked list to make easier insertion/deletion,
 * but its easy to see them as 
 */


struct list_head {
	struct list_head *next;
};

struct dlist_head {
	struct dlist_head *next;
	struct dlist_head *prev;
};


#define LIST_HEAD_INIT(name) \
	{ &(name) }

#define INIT_LIST_HEAD(ptr) \
	(ptr)->next = (ptr)


#define DLIST_HEAD_INIT(name) \
	{ .next=&(name), .prev=&(name) }

#define INIT_DLIST_HEAD(ptr) \
	(ptr)->next = (ptr); \
	(ptr)->prev = (ptr)



extern inline void list_push_front(struct list_head *head,
		struct list_head *element)
{
	element->next = head->next->next;
	head->next = element;
}



extern inline void dlist_push_front(struct dlist_head *head,
		struct dlist_head *element)
{
	element->next = head->next->next;
	element->prev = head;
	head->next = element;
}


extern inline void dlist_push_back(struct dlist_head *head,
		struct dlist_head *element)
{
	element->prev = head->prev->prev;
	element->next = head;
	head->prev = element;
}
#endif //_UTILS_LIST_H
