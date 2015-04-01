/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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


static inline void list_push_front(struct list_head *head,
		struct list_head *element)
{
	element->next = head->next;
	head->next = element;
}

// not possible to remove an element in O(1) in a simply linked list
static inline void list_remove(struct list_head *head,
		struct list_head *element)
{
	struct list_head *prev;

	for(prev=head; prev->next!=element && prev->next!=head; prev=prev->next);
	if(prev->next == element) {
		prev->next = element->next;
		element->next = element;
	}
}


static inline void dlist_push_front(struct dlist_head *head,
		struct dlist_head *element)
{
	element->next = head->next;
	element->prev = head;
	head->next->prev = element;
	head->next = element;
}


static inline void dlist_push_back(struct dlist_head *head,
		struct dlist_head *element)
{
	element->prev = head->prev;
	element->next = head;
	head->prev->next = element;
	head->prev = element;
}

// no need of the head of the list here
static inline void dlist_remove(struct dlist_head *element)
{
	element->next->prev = element->prev;
	element->prev->next = element->next;
	element->prev = element;
	element->next = element;
}

#endif //_UTILS_LIST_H
