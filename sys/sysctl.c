#include "sysctl.h"
#include <utils/strutils.h>
#include <interface/version.h>
#include <utils/log.h>

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
		return ctl_access_read_helper(oldbuf, oldlen, "FiXos", sizeof("FiXos"));
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


#define STRINGIZER(x)	#x
#define TO_STRING(x)	STRINGIZER(x)
#define ARCH_STRING		TO_STRING(CONFIG_ARCH)
#define MODEL_STRING	TO_STRING(CONFIG_MODEL)


static int access_hw_machine(void *oldbuf, size_t *oldlen, const void *newbuf,
		size_t newlen)
{
	if(oldlen != NULL) {
		return ctl_access_read_helper(oldbuf, oldlen, ARCH_STRING,
				sizeof(ARCH_STRING));
	}
	return -1;
}

static SYSCTL_OBJECT(ctl_hw_machine) = {
	.parent = &ctl__hw,
	.name = "machine",
	.id = HW_MACHINE,
	.type = CTL_TYPE_OPAQUE,
	.access.data = &access_hw_machine
};


static int access_hw_model(void *oldbuf, size_t *oldlen, const void *newbuf,
		size_t newlen)
{
	if(oldlen != NULL) {
		return ctl_access_read_helper(oldbuf, oldlen, MODEL_STRING,
				sizeof(MODEL_STRING));
	}
	return -1;
}

static SYSCTL_OBJECT(ctl_hw_model) = {
	.parent = &ctl__hw,
	.name = "model",
	.id = HW_MODEL,
	.type = CTL_TYPE_OPAQUE,
	.access.data = &access_hw_model
};


#define OSRELEASE		( TO_STRING(FIXOS_VERSION_MAJOR) "." \
	TO_STRING(FIXOS_VERSION_MINOR) )

#define OSDATE			( __DATE__ " " __TIME__ )

static int access_kern_osrelease(void *oldbuf, size_t *oldlen, const void *newbuf,
		size_t newlen)
{
	if(oldlen != NULL) {
		return ctl_access_read_helper(oldbuf, oldlen, OSRELEASE,
				sizeof(OSRELEASE));
	}
	return -1;
}

static SYSCTL_OBJECT(ctl_kern_osrelease) = {
	.parent = &ctl__kern,
	.name = "osrelease",
	.id = KERN_OSRELEASE,
	.type = CTL_TYPE_OPAQUE,
	.access.data = &access_kern_osrelease
};



static int access_kern_osbuilddate(void *oldbuf, size_t *oldlen, const void *newbuf,
		size_t newlen)
{
	if(oldlen != NULL) {
		return ctl_access_read_helper(oldbuf, oldlen, OSDATE,
				sizeof(OSDATE));
	}
	return -1;
}

static SYSCTL_OBJECT(ctl_kern_osbuilddate) = {
	.parent = &ctl__kern,
	.name = "osbuilddate",
	.id = KERN_OSBUILDDATE,
	.type = CTL_TYPE_OPAQUE,
	.access.data = &access_kern_osbuilddate
};


void ctl_init() {
	// TODO for now the tables are not created to avoid dynamic allocation
	// of node data structures (so a lot of sysctl may be an important
	// performance issue as the whole .sysctl.objs is parsed many times)
}


int ctl_access_read_helper(void *oldbuf, size_t *oldlen, const void *data,
		size_t datalen)
{
	size_t maxlen = *oldlen;

	*oldlen = datalen;
	if(maxlen >= datalen) {
		if(oldbuf != NULL) {
			memcpy(oldbuf, data, datalen);	
			return 0;
		}
	}
	return -1;
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

	printk("sysctl: name not found {");
	int i;
	for(i=0; i<name_len; i++)
		printk("%d, ", name[i]);
	printk("}\n");
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




// return the child named strname in parent node if any, NULL if not found
static const struct ctl_object * ctl_find_child_byname (
		const struct ctl_object *parent, const char *strname)
{
	const struct ctl_object *cur;
	
	for(cur = &sysctl_objs_begin; cur < &sysctl_objs_end; cur++) {
		if(cur->parent == parent && (strcmp(cur->name, strname) == 0)) {
			return cur;
		}
	}

	return NULL;
}

#define NAMEPART_MAX	20
int sys_sysctl_mibname(const char *strname, int *name, int *name_len) {
	if(strname!=NULL && name!=NULL && name_len!=NULL) {
		int end;
		int curlen;
		const struct ctl_object *curparent;

		curparent = &ctl__root;
		for(end=0, curlen=0; !end; curlen++) {
			char part[NAMEPART_MAX];
			int plen;

			// split each part of the string and search each one
			for(plen=0; plen<(NAMEPART_MAX-1) && strname[plen]!='.'
					&& strname[plen]!='\0'; plen++)
			{
				part[plen]=strname[plen];
			}

			if(plen < (NAMEPART_MAX-1)) {
				if(strname[plen]=='\0')
					end = 1; // stop search after this one
				else {
					strname += plen+1;	
				}

				// look for the corresponding object if any
				part[plen] = '\0';
				curparent = ctl_find_child_byname(curparent, part);
				if(curparent == NULL || *name_len < curlen+1) {
					printk("sysctl: not found '%s'\n", part);
					return -1;
				}

				name[curlen] = curparent->id;
			}
			else {
				// too long
				printk("sysctl: string name too long\n");
				return -1;
			}
		}
		*name_len = curlen;
		return 0;
	}
	return -1;
}
