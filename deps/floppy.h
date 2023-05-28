
#include <stdbool.h>
#include <stdint.h>


enum FLPYDSK_IO {
 
	FLPYDSK_DOR		=	0x3f2,
	FLPYDSK_MSR		=	0x3f4, // main status register
	FLPYDSK_FIFO		=	0x3f5,	//data register
	FLPYDSK_CTRL		=	0x3f7
};

enum FLPYDSK_DOR_MASK {
 
	FLPYDSK_DOR_MASK_DRIVE0			=	0,	//00000000
	FLPYDSK_DOR_MASK_DRIVE1			=	1,	//00000001
	FLPYDSK_DOR_MASK_DRIVE2			=	2,	//00000010
	FLPYDSK_DOR_MASK_DRIVE3			=	3,	//00000011
	FLPYDSK_DOR_MASK_RESET			=	4,	//00000100
	FLPYDSK_DOR_MASK_DMA			=	8,	//00001000
	FLPYDSK_DOR_MASK_DRIVE0_MOTOR		=	16,	//00010000
	FLPYDSK_DOR_MASK_DRIVE1_MOTOR		=	32,	//00100000
	FLPYDSK_DOR_MASK_DRIVE2_MOTOR		=	64,	//01000000
	FLPYDSK_DOR_MASK_DRIVE3_MOTOR		=	128	//10000000
};

enum FLPYDSK_MSR_MASK {
 
	FLPYDSK_MSR_MASK_DRIVE1_POS_MODE	=	1,	//00000001
	FLPYDSK_MSR_MASK_DRIVE2_POS_MODE	=	2,	//00000010
	FLPYDSK_MSR_MASK_DRIVE3_POS_MODE	=	4,	//00000100
	FLPYDSK_MSR_MASK_DRIVE4_POS_MODE	=	8,	//00001000
	FLPYDSK_MSR_MASK_BUSY			=	16,	//00010000
	FLPYDSK_MSR_MASK_DMA			=	32,	//00100000
	FLPYDSK_MSR_MASK_DATAIO			=	64, 	//01000000
	FLPYDSK_MSR_MASK_DATAREG		=	128	//10000000
};

enum FLPYDSK_CMD {
	
	FDC_CMD_READ_TRACK	=	2,
	FDC_CMD_SPECIFY		=	3,
	FDC_CMD_CHECK_STAT	=	4,
	FDC_CMD_WRITE_SECT	=	5,
	FDC_CMD_READ_SECT	=	6,
	FDC_CMD_CALIBRATE	=	7,
	FDC_CMD_CHECK_INT	=	8,
	FDC_CMD_WRITE_DEL_S	=	9,
	FDC_CMD_READ_ID_S	=	0xa,
	FDC_CMD_READ_DEL_S	=	0xc,
	FDC_CMD_FORMAT_TRACK	=	0xd,
	FDC_CMD_SEEK		=	0xf
};

enum FLPYDSK_CMD_EXT {
 
	FDC_CMD_EXT_SKIP	=	0x20,	//00100000
	FDC_CMD_EXT_DENSITY	=	0x40,	//01000000
	FDC_CMD_EXT_MULTITRACK	=	0x80	//10000000
};

enum FLPYDSK_GAP3_LENGTH {
 
	FLPYDSK_GAP3_LENGTH_STD = 42,
	FLPYDSK_GAP3_LENGTH_5_14= 32,
	FLPYDSK_GAP3_LENGTH_3_5= 27
};

// 2^n * 128, where ^ denotes "to the power of"
enum FLPYDSK_SECTOR_DTL {
 
	FLPYDSK_SECTOR_DTL_128	=	0,
	FLPYDSK_SECTOR_DTL_256	=	1,
	FLPYDSK_SECTOR_DTL_512	=	2,
	FLPYDSK_SECTOR_DTL_1024	=	4
};

#define FLPY_SECTORS_PER_TRACK 18
#define FLPYDSK_CMD_PERPENDICULAR 0x12

// specify operation on floppy disk and dma
typedef enum {
    floppy_dir_read = 1,
    floppy_dir_write = 2
} floppy_dir;

//refs
void flpydsk_handler();
void flpydsk_wait_irq();
void flpydsk_wait_for_fdc_data();
void flpydsk_write_dor (uint8_t val );
void flpydsk_write_ccr (uint8_t val);
void flpydsk_send_command (uint8_t cmd);
uint8_t flpydsk_read_data ();
int flpydsk_rw_sector (uint8_t head, uint8_t track, uint8_t sector, floppy_dir dir);
void flpydsk_drive_data (uint32_t stepr, uint32_t loadt, uint32_t unloadt, bool dma );
int flpydsk_calibrate (uint32_t drive);
void flpydsk_check_int (uint32_t* st0, uint32_t* cyl);
int flpydsk_seek ( uint32_t cyl, uint32_t head );
void flpydsk_disable_controller ();
void flpydsk_enable_controller ();
void flpydsk_set_perpendicular_mode(bool state);
void flpydsk_reset ();
void flpydsk_control_motor (bool b);
void flpydsk_lba_to_chs (int lba,int *head,int *track,int *sector);
void floppy_detect_drives();
void flpydsk_install();
void flpydsk_set_working_drive (uint8_t d);
static void flpydsk_dma_init(floppy_dir dir);
void* flpydsk_read_sector (int sectorLBA);