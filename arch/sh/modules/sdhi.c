#include <device/sd_card.h>
#include <arch/sh/7705_Casio.h>
#include <arch/sh/kdelay.h>

// debug
#include <utils/log.h>


// timeout delay in 1/64 seconds
#define SD_TIMEOUT_COMMAND	80

// used in sdhi_fun8() and other (UnknownByte2)
// seems to indicate if the card is SD (1) or MMC(0)
static int _sd_sdmode = 1;


static void sd_init_ports();

void sd_init_registers();

static void sd_inhibit_interupt();

static void sd_allow_interupt();

int sd_send_command(int cmd, unsigned short arg1, unsigned short arg2);

static void sd_sdhi_fun4(int mode) {
	if( (mode & 1) != 0) {
		SDHI.config2 &= 0x0EFF;
	}
}

// replace call to sdhi_fun8(1)
static void sd_sdhi_fun8_1() {
	if(_sd_sdmode != 0) {
		SDHI.config2 = (SDHI.config2 & 0x0100) | 0x0020;
	}
	else {
		SDHI.config2 = (SDHI.config2 & 0x0100) | 0x0080;
	}
}



// pseudo async timer using RTC 64Hz reg
// contain previous read value and counter
struct rtc64_timer {
	unsigned char prev_val;
	int counter;
};

// initialize rtc counter struct, equivalent of sdhi_fun9(-1)
void rtc_init_timer(struct rtc64_timer *timer) {
	timer->prev_val = RTC.R64CNT;
	timer->counter = 0;
}

// increase the counter if rtc 64Hz counter changed from last call
// equivalent of sdhi_fun9(0)
// WARNING : if 64Hz counter increased of more than 1, only 1 will be added
// to the counter, so use it only in a row if you want to be sure to have
// real values!
// Returns counter value.
int rtc_update_timer(struct rtc64_timer *timer) {
	unsigned char rtc64cnt = RTC.R64CNT;

	if(timer->prev_val != rtc64cnt) {
		timer->prev_val = rtc64cnt;
		timer->counter++;
	}

	return timer->counter;
}



// replace sdhi_fun2() call
int sd_isready_0() {
	if( (SDHI.config2 & 0x0100) == 0) {

		// replace sdhi_fun8(0) call
		SDHI.config2 = (SDHI.config2 & 0x0EFF) | 0x0100;
		return 1;
	}
	return 0;
}


int sd_init() {
	sd_init_registers();

	// init commands
	
	int retvalue = 0;

	int tmp;

	struct sd_resp32 resp32;

	int fun2ret = sd_isready_0();

	_sd_sdmode = 1;

	sd_sdhi_fun8_1();
	
	printk("Begin of init\n");

	// send CMD0(0,0) sdhi_fun3
	sd_send_command(0, 0, 0);

	sd_get_resp32(&resp32);
	printk("CMD0 resp = {%p}\n", (void*)(resp32.w0 + (resp32.w1 << 16) ));

	// send CMD55(0, 0)
	tmp = sd_send_command(55, 0, 0);

	sd_get_resp32(&resp32);
	printk("CMD55 resp = {%p}\n", (void*)(resp32.w0 + (resp32.w1 << 16) ));
	printk("ret = %d\n", tmp);

	if( tmp == 0) {
		int loopquit = 0;
		do {
			// send ACMD41(0x001F, 0x8000)
			// voltage window is (3.6v to 3.0v)
			tmp = sd_send_command(105 /*41 + 0x40*/, 0x001F /*31*/, 0x8000);

			if(tmp != 0) {
				sd_sdhi_fun4(fun2ret);
				return 0;
			}
		
			// read response
			sd_get_resp32(&resp32);

			printk("ACMD41 resp = {%p}\n", (void*)(resp32.w0 + (resp32.w1 << 16) ));

			// check a part of the response
			if( (resp32.w1 & 0x8000) == 0) {
				
				// CMD55(0, 0)
				tmp = sd_send_command(55, 0, 0);

				if(tmp != 0) {
					sd_sdhi_fun4(fun2ret);
					return 0;
				}
			}
			else {
				_sd_sdmode = 1; 	

				retvalue = 1;
				loopquit = 1;
			}
		} while(!loopquit);
	}

	else if(tmp == 0x00C9) {
		SDHI.config4 = 0xC0E0;

		do {
			// CMD1(0x0031, 0x8000), MMC_SEND_OP_COND
			tmp = sd_send_command(1, 0x001F, 0x8000);

			if(tmp != 0) {
				sd_sdhi_fun4(fun2ret);
				return 0; //??
			}

			sd_get_resp32(&resp32);
		} while( (resp32.w1 & 0x8000) == 0);

		_sd_sdmode = 0;
		retvalue = 1;	
	}


	sd_sdhi_fun8_1();

	sd_sdhi_fun4(fun2ret);

	return retvalue;
}



void sd_init_registers() {
	sd_init_ports();

	SDCLKCR.BIT.u1 = 0;
	UKNPORT.BIT.u1 = 0;

	kdelay(10); // sdhi_fun11(10) is used

	SDCLKCR.BIT.u1 = 1;
	TOTALY_UNKNOWN_1 = 0;

	// Don't know exactly what is done here :
	SDHI.word_far_2 = 0;
	kdelay(1);
	SDHI.word_far_2 = 1;

	kdelay(10); //10

	SDHI.config2 = 0x0020;
	SDHI.config3 = 0x0200;
	SDHI.word_far_1 = 0;
	SDHI.word_far_2 = 0x00C0;
	SDHI.config4 = 0x40E0;
	SDHI.config0 = 0x0305;
	SDHI.config1 = 0x837F;

	sd_allow_interupt();
}


void sd_init_ports() {
	// like sdhi_fun13()
	PE.DR.BYTE &= 0xDF; // bit 5 clear
	PE.DR.BYTE &= 0xEF; // bit 4 clear

	SCP.DR.BYTE &= 0xFD; // bit 1 clear

	// Why? Present in Casio's code...
	PFC.PECR.WORD = PFC.PECR.WORD;
	PFC.PECR.WORD = PFC.PECR.WORD;
	PFC.SCPCR.WORD = PFC.SCPCR.WORD;

	sd_inhibit_interupt();
}



int sd_get_resp32(struct sd_resp32 *resp) {
	resp->w0 = SDHI.resp0;
	resp->w1 = SDHI.resp1; 

	return 0;
}



void sd_inhibit_interupt() {
	IPRI.BIT._SDI = 0;
}


void sd_allow_interupt() {
	IPRI.BIT._SDI = 0xF;
}


int sd_send_command(int cmd, unsigned short arg1, unsigned short arg2) {
	short funret; // 0

	struct rtc64_timer timeout;

	int retval = 0;
	int check = 0;

	// command should be 8 bits sized... so why?
	if( (cmd & 0xFF00) != 0) {

		//funret = sdhi_fun9(-1);
		rtc_init_timer(&timeout);

		//while( sdhi_fun10(funret, sdhi_fun9(0), 0x0080) == 0) {
		while( rtc_update_timer(&timeout) < SD_TIMEOUT_COMMAND) {
			if( (SDHI.word_u15 & 0x4000) == 0) {
				check = 0;
				break;
			}
			check = 0x00CD;
		} 

		if(check != 0x00CD) {
			return 0x00CD;
		}
	} // if [...]

	SDHI.word_u14 &= 0xFFFA;

	SDHI.word_u15 &= 0xFF80;
	funret = sd_isready_0();

	//sdhi_fun11(10);
	kdelay(10);

	// WARNING!!! low word is set in 
	SDHI.arg1 = arg1;
	SDHI.arg0 = arg2;
	SDHI.command = cmd;

	//int funret2 = sdhi_fun9(-1);
	rtc_init_timer(&timeout);

	//while( sdhi_fun10(funret2, sdhi_fun9(0), 0x0080) == 0) {
	while( rtc_update_timer(&timeout) < SD_TIMEOUT_COMMAND) {
		if( (SDHI.word_u14 & 0x0001) != 0)
			break;
	}

	if( (SDHI.word_u15 & 0x0040) != 0) {
		retval = 0x00C9;
	}
	if( (SDHI.word_u15 & 0x0002) != 0) {
		retval = 0x00CA;
	}
	if( (SDHI.word_u15 & 0x0001) != 0) {
		retval = 0x00D0;
	}


	// only executed for commands CMD7 and CMD0 (SELECT_DESELECT and GO_IDLE_STATE)
	if( cmd == 0 || cmd == 7) {

		//funret2 = sdhi_fun9(-1);
		rtc_init_timer(&timeout);

		//while( sdhi_fun10(funret2, sdhi_fun9(0), 0x0080) == 0) {
		while( rtc_update_timer(&timeout) < SD_TIMEOUT_COMMAND) {
			if( (SDHI.word_u15 & 0x4000) == 0)
				break;
		}

		// TODO remove this strange thing??
		int i;
		for(i=10; i!=0; i--) {
			//sdhi_fun11(i);
			kdelay(i);
		}
	}

	SDHI.word_u14 &= 0xFFFA;
	SDHI.word_u15 &= 0xFF80;

	sd_sdhi_fun4(funret);

	return retval;
}
