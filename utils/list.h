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


#define list_for_each(cur, head) \
	for(cur=(head)->next; cur!=(head); cur = cur->next)

// same implementation
#define dlist_for_each(cur, head) \
	list_for_each(cur, head)


extern inline void list_push_front(struct list_head *head,
		struct list_head *element)
{
	element->next = head->next;
	head->next = element;
}

// not possible to remove an element in O(1) in a simply linked list
extern inline void list_remove(struct list_head *head,
		struct list_head *element)
{
	struct list_head *prev;

	for(prev=head; prev->next!=element && prev->next!=head; prev=prev->next);
	if(prev->next == element) {
		prev->next = element->next;
		element->next = element;
	}
}


extern inline void dlist_push_front(struct dlist_head *head,
		struct dlist_head *element)
{
	element->next = head->next;
	element->prev = head;
	head->next = element;
}


extern inline void dlist_push_back(struct dlist_head *head,
		struct dlist_head *element)
{
	element->prev = head->prev;
	element->next = head;
	head->prev = element;
}

// no need of the head of the list here
extern inline void dlist_remove(struct dlist_head *element)
{
	element->next->prev = element->prev;
	element->prev->next = element->next;
	element->prev = element;
	element->next = element;
}

#endif //_UTILS_LIST_H
