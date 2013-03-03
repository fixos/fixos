#ifndef _FS_PROTOFS_PRIMITIVES_H
#define _FS_PROTOFS_PRIMITIVES_H


#define PROTOFS_MAX_NAME	16

// special type flags to indicate a free node
#define PROTOFS_TYPE_EMPTY	(1<<14)

// root node identifier
#define PROTOFS_ROOT_NODE		0xFFFFFFFF
#define PROTOFS_INVALID_NODE	0xEEEEEEEE

// structure of the protofs node
struct _protofs_node {
	char name[PROTOFS_MAX_NAME];
	void *parent;
	
	uint16 type_flags;
	uint16 mode;

	union {
		struct {
			uint16 major;
			uint16 minor;
		} dev;
	} special;
};

typedef struct _protofs_node protofs_node_t;

#define PROTOFS_PER_PAGE	(PM_PAGE_BYTES/sizeof(protofs_node_t))

#define PROTOFS_NODE_NB(addr) ((int)(addr) - (int)_protofs_page) / sizeof(protofs_node_t)

#define PROTOFS_NODE_ADDR(nb) ((void*) ((nb) == PROTOFS_ROOT_NODE ? 0 : (nb) * sizeof(protofs_node_t) + (int)_protofs_page) )

#endif //_FS_PROTOFS_PRIMITIVES_H
