#include "virtual_memory.h"


vm_page_t *vm_find_vpn(vm_table_t *table, unsigned int vpn)
{
	int i;
	vm_page_t *ret = (void*)0;
	struct _vm_table_page *vtp;

	// for now the method is a stupid implementation of the problem...

	// direct access:
	for(i=0; i<3 && ret==(void*)0; i++)
	{
		if(table->direct[i].valid && table->direct[i].vpn == vpn)
			ret = &(table->direct[i]);
	}

	if(ret != (void*)0 && (vtp = table->indir1) )
	{
		// indirect access:
		for(i=0; i<(VM_DEFAULT_BYTES/sizeof(vm_page_t)) && ret==(void*)0; i++)
		{
			if(vtp->direct[i].valid && vtp->direct[i].vpn == vpn)
				ret = &(vtp->direct[i]);
		}
	}

	return ret;
}


int vm_add_entry(vm_table_t *table, vm_page_t *page)
{
	int i;
	vm_page_t *found = (void*)0;
	struct _vm_table_page *vtp;

	// for now the method is a stupid implementation of the problem...

	// direct access:
	for(i=0; i<3 && found==(void*)0; i++)
	{
		if(! table->direct[i].valid)
			found = &(table->direct[i]);
	}

	if(found != (void*)0 && (vtp = table->indir1) != (void*)0 )
	{
		// indirect access:
		for(i=0; i<(VM_DEFAULT_BYTES/sizeof(vm_page_t)) && found==(void*)0; i++)
		{
			if(! vtp->direct[i].valid)
				found = &(vtp->direct[i]);
		}
	}

	if(found)
	{
		(*found) = *page;
		return 0;
	}

	return -1;
}
