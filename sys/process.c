#include "process.h"

// Test purpose, like a kernel process
static process_t mock_process =
{
	.pid = 0,
	.asid = 0xFF,
	.vm = {
		.direct = {{0}, {0}, {0}},
		.indir1 = (void*)0
	},
	.stack_addr = (void*)0,
	.pc_addr = (void*)0,
	.base_stack = (void*)0,
	.state = PROCESS_RUN
};



process_t *process_from_asid(unsigned char asid)
{
	// TODO real implementation!!!!
	return &mock_process;
}
