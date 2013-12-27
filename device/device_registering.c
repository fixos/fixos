#include "device_registering.h"


// entry of register vector, not used if dev == NULL
struct device_entry {
	struct device *dev;
	uint16 major;
};

static struct device_entry _registered_devices[DEVICE_REGISTER_MAX];


void dev_init() {
	int i;
	
	for(i=0; i<DEVICE_REGISTER_MAX; i++)
		_registered_devices[i].dev = NULL;
}


int dev_register_device(struct device *dev, uint16 major_id) {
	int i;

	// get the first free entry
	for(i=0; i<DEVICE_REGISTER_MAX && _registered_devices[i].dev != NULL; i++);

	if(i<DEVICE_REGISTER_MAX) {
		_registered_devices[i].dev = dev;
		_registered_devices[i].major = major_id;
		return 0;
	}

	return -1;
}


struct device* dev_device_from_major(uint16 major_id) {
	int i;

	for(i=0; i<DEVICE_REGISTER_MAX; i++) {
		if(_registered_devices[i].dev != NULL && _registered_devices[i].major == major_id)
			return _registered_devices[i].dev;
	}

	return NULL;
}


void dev_unregister_device(uint16 major_id) {
	int i;

	// do for every entry (not only he first)
	for(i=0; i<DEVICE_REGISTER_MAX; i++) {
		if(_registered_devices[i].major == major_id)
			_registered_devices[i].dev = NULL;
	}
}


