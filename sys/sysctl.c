#include "sysctl.h"
#include <utils/strutils.h>

// defined by linker script
extern const struct ctl_object sysctl_objs_begin;
extern const struct ctl_object sysctl_objs_end;


SYSCTL_OBJECT(ctl__hw) = {
	.parent = &ctl__root,
	.name = "hw",
	.id = CTL_HW,
	.type = CTL_TYPE_NODE
};


SYSCTL_OBJECT(ctl__kern) = {
	.parent = &ctl__root,
	.name = "kern",
	.id = CTL_KERN,
	.type = CTL_TYPE_NODE
};




static int access_kern_ostype(void *oldbuf, size_t *oldlen, const void *newbuf,
		size_t newlen)
{
	if(oldlen != NULL) {
		if(*oldlen >= sizeof("FiXos")) {
			*oldlen = sizeof("FiXos");
			if(oldbuf != NULL) {
				memcpy(oldbuf, "FiXos", sizeof("FiXos"));	
				return 0;
			}
		}
	}
	return -1;
}

static SYSCTL_OBJECT(ctl_kern_ostype) = {
	.parent = &ctl__kern,
	.name = "ostype",
	.id = KERN_OSTYPE,
	.type = CTL_TYPE_OPAQUE,
	.access.data = &access_kern_ostype
};



void ctl_init() {
	// TODO for now the tables are not created to avoid dynamic allocation
	// of node data structures (so a lot of sysctl may be an important
	// performance issue as the whole .sysctl.objs is parsed many times)
}


static const struct ctl_object * ctl_find_object(const int *name,
		size_t name_len)
{
	// for now, no precomputed tables
	const struct ctl_object *curparent;
	int curlevel;
	int error = 0;

	curparent = &ctl__root;
	for(curlevel=0; curlevel < name_len && !error; curlevel++) {
		const struct ctl_object *cur;
		
		for(cur = &sysctl_objs_begin; cur < &sysctl_objs_end; cur++) {
			if(cur->parent == curparent && cur->id == name[curlevel]) {
				break;
			}
		}

		if(cur < &sysctl_objs_end) {
			if(cur->type == CTL_TYPE_NODE) {
				curparent = cur;
			}
			// for indexed data
			else if(cur->type == CTL_TYPE_OPAQUE_NDX 
					&& curlevel+2 == name_len)
			{
				curlevel++;
				curparent = cur;
			}
			else if(cur->type == CTL_TYPE_OPAQUE
					&& curlevel+1 == name_len)
			{
				curparent = cur;
			}
			else {
				error = 1;
			}
		}
		else {
			error = 1;
		}
	}

	return error == 0 ? curparent : NULL;
}


int sys_sysctl_read(const int *name, size_t name_len, void *buf, size_t *len)
{
	const struct ctl_object *obj;

	obj = ctl_find_object(name, name_len);
	if(obj != NULL) {
		if(obj->type == CTL_TYPE_OPAQUE) {
			return obj->access.data(buf, len, NULL, 0);
		}
		else if(obj->type == CTL_TYPE_OPAQUE_NDX) {
			return obj->access.indexed(buf, len, NULL, 0, name[name_len-1]);
		}
	}

	return -1;
}


int sys_sysctl_write(const int *name, size_t name_len, const void *buf,
		size_t *len)
{
	(void)name;
	(void)name_len;
	(void)buf;
	(void)len;
	return -1;
}

