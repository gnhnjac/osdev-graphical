#pragma once
#include <stdint.h>

#define PIXEL_WIDTH 1920
#define PIXEL_HEIGHT 1080

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

#define PSF1_FONT_MAGIC 0x0436

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#pragma pack(1)
 
typedef struct {
    uint16_t magic; // Magic bytes for idnetiifcation.
    uint8_t fontMode; // PSF font mode
    uint8_t characterSize; // PSF character size.
} PSF1_Header;
 
 
#define PSF_FONT_MAGIC 0x864ab572
 
typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} PSF_font;

struct VbeInfoBlock {
   char     VbeSignature[4];         // == "VESA"
   uint16_t VbeVersion;              // == 0x0300 for VBE 3.0
   uint16_t OemStringPtr[2];         // isa vbeFarPtr
   uint8_t  Capabilities[4];
   uint16_t VideoModePtr[2];         // isa vbeFarPtr
   uint16_t TotalMemory;             // as # of 64KB blocks
   uint8_t  Reserved[492];
};

struct vbe_mode_info_structure {
    uint16_t attributes;        // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;           // deprecated
    uint8_t window_b;           // deprecated
    uint16_t granularity;       // deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;      // deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;         // number of bytes per horizontal line
    uint16_t width;         // width in pixels
    uint16_t height;            // height in pixels
    uint8_t w_char;         // unused...
    uint8_t y_char;         // ...
    uint8_t planes;
    uint8_t bpp;            // bits per pixel in this mode
    uint8_t banks;          // deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size;      // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;       // physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;   // size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
};

#pragma pack()

//refs
static uint8_t reverse(uint8_t b);
void init_psf1_8x16();
void graphics_init();
uint8_t *get_font_buffer();
void load_font_to_buffer(void *buff);
int load_psf1_8x16(char *path);
void display_psf1_8x16_char_linear(char c, int x, int y, int fgcolor);
void display_psf1_8x16_char_bg_linear(char c, int x, int y, int bgcolor, int fgcolor);
void display_psf1_8x16_char(char c, int x, int y, uint8_t fgcolor);
void display_psf1_8x16_char_bg(char c, int x, int y, int bgcolor, int fgcolor);
void fill_rect_linear(int x, int y, int width, int height, uint32_t color);
void outline_rect(int x, int y, int width, int height, int size, uint8_t color);
void fill_rect(int x, int y, int width, int height, uint32_t color);
void outline_circle(int mx, int my, int rad, uint8_t color);
void set_pixel (int x, int y, uint32_t color);
uint32_t get_pixel (int x, int y);