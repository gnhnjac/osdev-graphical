#include <stdint.h>

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

#pragma pack()

#define MEMORY_PLANE_WRITE_ENABLE 0x02
#define READ_MAP_SELECT 0x04

//refs
static uint8_t reverse(uint8_t b);
void init_psf1_8x16();
int load_psf1_8x16(char *path);
void display_psf1_8x16_char_linear(char c, int x, int y, int fgcolor);
void display_psf1_8x16_char_bg_linear(char c, int x, int y, int bgcolor, int fgcolor);
void display_psf1_8x16_char(char c, int x, int y, uint8_t fgcolor);
void display_psf1_8x16_char_bg(char c, int x, int y, int bgcolor, int fgcolor);
void fill_rect_linear(int x, int y, int width, int height, uint8_t color);
void outline_rect(int x, int y, int width, int height, int size, uint8_t color);
void fill_rect(int x, int y, int width, int height, uint8_t color);
void outline_circle(int mx, int my, int rad, uint8_t color);
void set_pixel (int x, int y, uint8_t color);
uint8_t is_planar_bit_activated(int x, int y, uint8_t plane);
uint8_t get_pixel (int x, int y);