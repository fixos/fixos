#include "pool_alloc.h"


struct pool_emptyobj {
	// next empty object *in the current page*
	struct pool_emptyobj *next;
};


// private internal functions :

// allocate and prepare a new pool page and return it (or NULL if failled)
static struct pool_page * get_new_pool_page(const struct pool_alloc *pool) {
	struct pool_page *ret;

	ret = mem_pm_get_free_page(MEM_PM_CACHED);
	if(ret != NULL) {
		struct pool_emptyobj *cur;
		struct pool_emptyobj *prev;
		int i;

		ret->nbempty = pool->perpage;
		ret->next = NULL;
		
		// first empty object is at the end of the 'ret' struct
		cur = (void*) (ret + 1);
		ret->first_empty = cur;
		prev = NULL;

		for(i=0; i<pool->perpage; i++, cur = (void*)cur + pool->objsz) {
			if(prev != NULL)
				prev->next = cur;
			prev = cur;
		}
		cur->next = NULL;
	}

	return ret;
}


// return the first non-full page in the linked list (or NULL if all are full)
static struct pool_page * get_first_nonfull(const struct pool_alloc *pool) {
	struct pool_page *ret;

	ret = pool->first_page;
	while(ret != NULL && ret->nbempty <= 0)
		ret = ret->next;

	return ret;
}


// return 1 if the given page is present in the pool page list, 0 else
static int check_page_owned(const struct pool_alloc *pool, 
		const struct pool_page *page)
{
	struct pool_page *cur;

	cur = pool->first_page;
	while(cur != NULL && cur != page)
		cur = cur->next;

	return cur != NULL;
}



void * pool_alloc(struct pool_alloc *pool) {
	struct pool_page *curpage;
	void *ret;

	ret = NULL;
	curpage = pool->first_nonfull;
	if(curpage == NULL) {
		// allocate a new page and add it to the linked list
		curpage = get_new_pool_page(pool);
		if(curpage != NULL) {
			curpage->next = pool->first_page;
			pool->first_page = curpage;
			pool->first_nonfull = curpage;
		}
	}

	if(curpage != NULL) {
		// just return the first available object in this page
		ret = curpage->first_empty;
		curpage->first_empty = curpage->first_empty->next;
		curpage->nbempty--;
		if(curpage->nbempty <= 0) {
			pool->first_nonfull = get_first_nonfull(pool);
		}
	}

	return ret;
}



void pool_free(struct pool_alloc *pool, void *object) {
	// for now, this function use some black magic to get the pool_page
	// pointer from the object pointer : we assume the pool_page structure
	// is always at the beginning of a physical page, so if object is
	// really a pool_alloc return value, its pool_page is 
	// (object - (object % PM_PAGE_BYTES))
	
	struct pool_page *page;
	page = (object - ((unsigned int) object) % PM_PAGE_BYTES);

	// temp check, allow to be sure if a freed object was from this pool
	// (slow down the free operation, need to be replaced later)
	if(check_page_owned(pool, page)) {
		// add it as first free element
		((struct pool_emptyobj *)object)->next = page->first_empty;
		page->first_empty = object;
		page->nbempty++;
	}
}
