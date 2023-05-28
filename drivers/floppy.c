#include "low_level.h"
#include "irq.h"
#include "floppy.h"
#include "timer.h"
#include "screen.h"
volatile bool flpydsk_irq_finished = false;

uint8_t _CurrentDrive = 0;

// we statically reserve a totally uncomprehensive amount of memory
// must be large enough for whatever DMA transfer we might desire
// and must not cross 64k borders so easiest thing is to align it
// to 2^N boundary at least as big as the block
#define floppy_dmalen 0x4800
static const char floppy_dmabuf[floppy_dmalen]
                  __attribute__((aligned(8192)));

void flpydsk_handler()
{

	flpydsk_irq_finished = true;

}


void flpydsk_wait_irq()
{

	while (!flpydsk_irq_finished)
		continue;
	flpydsk_irq_finished = false;

}

void flpydsk_wait_for_fdc_data()
{

	while ( !(inb (FLPYDSK_MSR) & FLPYDSK_MSR_MASK_DATAREG) )
		continue;
	//! FDC data reg is not ready

}

void flpydsk_write_dor (uint8_t val ) {
 
	//! write the digital output register
	outb (FLPYDSK_DOR, val);
}

void flpydsk_write_ccr (uint8_t val) {
 
	//! write the configuation control
	outb (FLPYDSK_CTRL, val);
}

void flpydsk_send_command (uint8_t cmd) {
 
	//! wait until data register is ready. We send commands to the data register
	flpydsk_wait_for_fdc_data();
	outb (FLPYDSK_FIFO, cmd);
}
 
uint8_t flpydsk_read_data () {
 
	//! same as above function but returns data register for reading
	flpydsk_wait_for_fdc_data();
	return inb (FLPYDSK_FIFO);
}

int flpydsk_rw_sector (uint8_t head, uint8_t track, uint8_t sector, floppy_dir dir) {
 
	uint32_t _st0, cyl;

	// seek both heads
    // if(flpydsk_seek(cyl, 0)) return -1;
    // if(flpydsk_seek(cyl, 1)) return -1;
 	
 	for(int i = 0; i < 20; i++) {
		//! set the DMA for read transfer
		flpydsk_dma_init(dir);

		wait_milliseconds(20);
	 
		//! read in a sector
		flpydsk_send_command (
			((dir == floppy_dir_read) ? FDC_CMD_READ_SECT : FDC_CMD_WRITE_SECT) | FDC_CMD_EXT_MULTITRACK |
			FDC_CMD_EXT_SKIP | FDC_CMD_EXT_DENSITY);
		flpydsk_send_command ( head << 2 | _CurrentDrive );
		flpydsk_send_command ( track);
		flpydsk_send_command ( head);
		flpydsk_send_command ( sector);
		flpydsk_send_command ( FLPYDSK_SECTOR_DTL_512 );
		// flpydsk_send_command (
		// 	( ( sector + 1 ) >= FLPY_SECTORS_PER_TRACK )
		// 		? FLPY_SECTORS_PER_TRACK : sector + 1 );
		flpydsk_send_command(FLPY_SECTORS_PER_TRACK);
		flpydsk_send_command ( FLPYDSK_GAP3_LENGTH_3_5 );
		flpydsk_send_command ( 0xff );
	 
		//! wait for irq to finish so we can read return data
		flpydsk_wait_irq ();
	 	
		// first read status information
        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        st0 = flpydsk_read_data();
        st1 = flpydsk_read_data();
        st2 = flpydsk_read_data();
        rcy = flpydsk_read_data();
        rhe = flpydsk_read_data();
        rse = flpydsk_read_data();
        // bytes per sector, should be what we programmed in
        bps = flpydsk_read_data();

        int error = 0;

        if(st0 & 0xC0) {
            static const char * status[] =
            { 0, "error", "invalid command", "drive not ready" };
            printf("flpydsk_rw_sector: status = %s\n", status[st0 >> 6]);
            error = 1;
        }
        if(st1 & 0x80) {
            printf("flpydsk_rw_sector: end of cylinder\n");
            error = 1;
        }
        if(st0 & 0x08) {
            printf("flpydsk_rw_sector: drive not ready\n");
            error = 1;
        }
        if(st1 & 0x20) {
            printf("flpydsk_rw_sector: CRC error\n");
            error = 1;
        }
        if(st1 & 0x10) {
            printf("flpydsk_rw_sector: controller timeout\n");
            error = 1;
        }
        if(st1 & 0x04) {
            printf("flpydsk_rw_sector: no data found\n");
            error = 1;
        }
        if((st1|st2) & 0x01) {
            printf("flpydsk_rw_sector: no address mark found\n");
            error = 1;
        }
        if(st2 & 0x40) {
            printf("flpydsk_rw_sector: deleted address mark\n");
            error = 1;
        }
        if(st2 & 0x20) {
            printf("flpydsk_rw_sector: CRC error in data\n");
            error = 1;
        }
        if(st2 & 0x10) {
            printf("flpydsk_rw_sector: wrong cylinder\n");
            error = 1;
        }
        if(st2 & 0x04) {
            printf("flpydsk_rw_sector: uPD765 sector not found\n");
            error = 1;
        }
        if(st2 & 0x02) {
            printf("flpydsk_rw_sector: bad cylinder\n");
            error = 1;
        }
        if(bps != 0x2) {
            printf("flpydsk_rw_sector: wanted 512B/sector, got %d", (1<<(bps+7)));
            error = 1;
        }
        if(st1 & 0x02) {
            printf("flpydsk_rw_sector: not writable\n");
            error = 2;
        }

        if(!error) {
            return 0;
        }
        if(error > 1) {
            printf("flpydsk_rw_sector: not retrying..\n");
            return -2;
        }
	}
}

void flpydsk_drive_data (uint32_t stepr, uint32_t loadt, uint32_t unloadt, bool dma ) {
 
	uint8_t data = 0;
 
	flpydsk_send_command (FDC_CMD_SPECIFY);
 
	data = ( (stepr & 0xf) << 4) | (unloadt >> 4 & 0xf);
	flpydsk_send_command (data);
	data = ((loadt) << 1) | ((dma==true) ? 0 : 1);
	flpydsk_send_command (data);
}

// turn off the floppy disk motor and set it at cylinder 0
int flpydsk_calibrate (uint32_t drive) {
 
	uint32_t st0, cyl;
 
	if (drive >= 4)
		return -2;
 
	//! turn on the motor
	flpydsk_control_motor (true);
 
	for (int i = 0; i < 10; i++) {
 
		//! send command
		flpydsk_send_command ( FDC_CMD_CALIBRATE );
		flpydsk_send_command ( drive );
		flpydsk_wait_irq ();
		flpydsk_check_int ( &st0, &cyl);

		if(st0 & 0xC0) {
            static const char * status[] =
            { 0, "error", "invalid", "drive" };
            printf("floppy_calibrate: status = %s\n", status[st0 >> 6]);
            continue;
        }
 
		//! did we fine cylinder 0? if so, we are done
		if (!cyl) {
 
			flpydsk_control_motor (false);
			return 0;
		}
	}
 
	flpydsk_control_motor (false);
	return -1;
}

// check information on the state of fdc when interrupt is done.
void flpydsk_check_int (uint32_t* st0, uint32_t* cyl) {
 	
	flpydsk_send_command (FDC_CMD_CHECK_INT);
 
	*st0 = flpydsk_read_data ();
	*cyl = flpydsk_read_data ();
}

int flpydsk_seek ( uint32_t cyl, uint32_t head ) {
 
	uint32_t st0, cyl0;
 
	if (_CurrentDrive >= 4)
		return -1;
 
	for (int i = 0; i < 10; i++ ) {
 
		//! send the command
		flpydsk_send_command (FDC_CMD_SEEK);
		flpydsk_send_command ( (head) << 2 | _CurrentDrive);
		flpydsk_send_command (cyl);
 
		//! wait for the results phase IRQ
		flpydsk_wait_irq ();
		flpydsk_check_int (&st0,&cyl0);

		 if(st0 & 0xC0) {
            static const char * status[] =
            { "normal", "error", "invalid", "drive" };
            printf("flpydsk_seek: status = %s\n", status[st0 >> 6]);
            continue;
        }
 
		//! found the cylinder?
		if ( cyl0 == cyl)
			return 0;
	}
 
	return -1;
}

void flpydsk_disable_controller () {
 
	flpydsk_write_dor (0);
}

void flpydsk_enable_controller () {
 
	flpydsk_write_dor ( FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
}

void flpydsk_set_perpendicular_mode(bool state)
{

	flpydsk_send_command(FLPYDSK_CMD_PERPENDICULAR);
	if (state)
		flpydsk_send_command(1 << (2+_CurrentDrive));
	else
		flpydsk_send_command(0);

}

void flpydsk_reset () {
 
	uint32_t st0, cyl;
 
	//! reset the controller
	flpydsk_disable_controller ();
	flpydsk_enable_controller ();
	flpydsk_wait_irq ();
 
	//! send CHECK_INT/SENSE INTERRUPT command to all drives
	for (int i=0; i<4; i++)
		flpydsk_check_int (&st0,&cyl);
 
	// //! transfer speed 500kb/s for 1.44M floppy
	// flpydsk_write_ccr (0);

	//! transfer speed 1mb/s for 2.88M floppy
	flpydsk_write_ccr (3);

	flpydsk_set_perpendicular_mode(true);

	//! pass mechanical drive info. steprate=3ms, unload time=240ms, load time=16ms
	flpydsk_drive_data (13,1,240,true);

	//! calibrate the disk
	flpydsk_calibrate ( _CurrentDrive );
}

//! turns the current floppy drives motor on/off
void flpydsk_control_motor (bool b) {

	//! sanity check: invalid drive
	if (_CurrentDrive > 3)
		return;

	uint8_t motor = 0;

	//! select the correct mask based on current drive
	switch (_CurrentDrive) {

		case 0:
			motor = FLPYDSK_DOR_MASK_DRIVE0_MOTOR;
			break;
		case 1:
			motor = FLPYDSK_DOR_MASK_DRIVE1_MOTOR;
			break;
		case 2:
			motor = FLPYDSK_DOR_MASK_DRIVE2_MOTOR;
			break;
		case 3:
			motor = FLPYDSK_DOR_MASK_DRIVE3_MOTOR;
			break;
	}

	//! turn on or off the motor of that drive
	if (b)
		flpydsk_write_dor (_CurrentDrive | motor | FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
	else
		flpydsk_write_dor (FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);

	//! in all cases; wait a little bit for the motor to spin up/turn off
	wait_milliseconds (20);
}

void flpydsk_lba_to_chs (int lba,int *head,int *track,int *sector) {
 
   *head = ( lba % ( FLPY_SECTORS_PER_TRACK * 2 ) ) / ( FLPY_SECTORS_PER_TRACK );
   *track = lba / ( FLPY_SECTORS_PER_TRACK * 2 );
   *sector = lba % FLPY_SECTORS_PER_TRACK + 1;
}

static const char * drive_types[8] = {
    "none",
    "360kB 5.25\"",
    "1.2MB 5.25\"",
    "720kB 3.5\"",

    "1.44MB 3.5\"",
    "2.88MB 3.5\"",
    "unknown type",
    "unknown type"
};

void floppy_detect_drives() {

   outb(0x70, 0x10);
   unsigned drives = inb(0x71);

   printf(" - Floppy drive 0: %s\n", drive_types[drives >> 4]);
   printf(" - Floppy drive 1: %s\n", drive_types[drives & 0xf]);

}

void flpydsk_install()
{

	irq_install_handler(6, flpydsk_handler);

	//! reset the fdc
	flpydsk_reset ();

}

void flpydsk_set_working_drive (uint8_t d)
{

	_CurrentDrive = d;

}

static void flpydsk_dma_init(floppy_dir dir) {

    union {
        unsigned char b[4]; // 4 bytes
        unsigned long l;    // 1 long = 32-bit
    } a, c; // address and count

    a.l = (unsigned) &floppy_dmabuf;
    c.l = (unsigned) floppy_dmalen - 1; // -1 because of DMA counting

    // check that address is at most 24-bits (under 16MB)
    // check that count is at most 16-bits (DMA limit)
    // check that if we add count and address we don't get a carry
    // (DMA can't deal with such a carry, this is the 64k boundary limit)
    if((a.l >> 24) || (c.l >> 16) || (((a.l&0xffff)+c.l)>>16)) {
        print("floppy_dma_init: static buffer problem\n");
    }

    unsigned char mode;
    switch(dir) {
        // 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
        case floppy_dir_read:  mode = 0x46; break;
        // 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2
        case floppy_dir_write: mode = 0x4a; break;
        default: print("floppy_dma_init: invalid direction");
                 return; // not reached, please "mode user uninitialized"
    }

    outb(0x0a, 0x06);   // mask chan 2

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x04, a.b[0]); //  - address low byte
    outb(0x04, a.b[1]); //  - address high byte

    outb(0x81, a.b[2]); // external page register

    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x05, c.b[0]); //  - count low byte
    outb(0x05, c.b[1]); //  - count high byte

    outb(0x0b, mode);   // set mode (see above)

    outb(0x0a, 0x02);   // unmask chan 2
}

void* flpydsk_read_sector (int sectorLBA) {
 
	if (_CurrentDrive >= 4)
		return 0;
 
	//! convert LBA sector to CHS
	int head=0, track=0, sector=1;
	flpydsk_lba_to_chs (sectorLBA, &head, &track, &sector);

	//! turn motor on and seek to track
	flpydsk_control_motor (true);
	if (flpydsk_seek (track, head) != 0)
		return 0;
 
	//! read sector and turn motor off
	flpydsk_rw_sector (head, track, sector, floppy_dir_read);
	flpydsk_control_motor (false);
 
	//! warning: this is a bit hackish
	return (void*) floppy_dmabuf;
}