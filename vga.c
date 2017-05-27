// #include "vga.h"

// #include "io_func.h"

// unsigned char g_320x200x256[] =
// {
// /* MISC */
// 	0x63,
// /* SEQ */
// 	0x03, 0x01, 0x0F, 0x00, 0x0E,
// /* CRTC */
// 	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
// 	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 	0x9C, 0x0E, 0x8F, 0x28,	0x40, 0x96, 0xB9, 0xA3,
// 	0xFF,
// /* GC */
// 	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
// 	0xFF,1,
// /* AC */
// 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
// 	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
// 	0x41, 0x00, 0x0F, 0x00,	0x00
// };

// typedef struct {
//     unsigned char r;
//     unsigned char g;
//     unsigned char b;
// } color3;

// void write_regs(unsigned char *regs)
// {
// 	unsigned i;

// /* write MISCELLANEOUS reg */
// 	outb(VGA_MISC_WRITE, *regs);
// 	regs++;
// /* write SEQUENCER regs */
// 	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
// 	{
// 		outb(VGA_SEQ_INDEX, i);
// 		outb(VGA_SEQ_DATA, *regs);
// 		regs++;
// 	}
// /* unlock CRTC registers */
// 	outb(VGA_CRTC_INDEX, 0x03);
// 	outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
// 	outb(VGA_CRTC_INDEX, 0x11);
// 	outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
// /* make sure they remain unlocked */
// 	regs[0x03] |= 0x80;
// 	regs[0x11] &= ~0x80;
// /* write CRTC regs */
// 	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
// 	{
// 		outb(VGA_CRTC_INDEX, i);
// 		outb(VGA_CRTC_DATA, *regs);
// 		regs++;
// 	}
// /* write GRAPHICS CONTROLLER regs */
// 	for(i = 0; i < VGA_NUM_GC_REGS; i++)
// 	{
// 		outb(VGA_GC_INDEX, i);
// 		outb(VGA_GC_DATA, *regs);
// 		regs++;
// 	}
// /* write ATTRIBUTE CONTROLLER regs */
// 	for(i = 0; i < VGA_NUM_AC_REGS; i++)
// 	{
// 		(void)inb(VGA_INSTAT_READ);
// 		outb(VGA_AC_INDEX, i);
// 		outb(VGA_AC_WRITE, *regs);
// 		regs++;
// 	}
// /* lock 16-color palette and unblank display */
// 	(void)inb(VGA_INSTAT_READ);
// 	outb(VGA_AC_INDEX, 0x20);
// }

// static void set_plane(unsigned p)
// {
// 	unsigned char pmask;

// 	p &= 3;
// 	pmask = 1 << p;
// /* set read plane */
// 	outb(VGA_GC_INDEX, 4);
// 	outb(VGA_GC_DATA, p);
// /* set write plane */
// 	outb(VGA_SEQ_INDEX, 2);
// 	outb(VGA_SEQ_DATA, pmask);
// }

// static void write_pixel8x(unsigned x, unsigned y, unsigned c)
// {
//     int g_wd = 320;
//     int g_ht = 200;

//     uint8_t* fb = (uint8_t*)0xA0000;
//     unsigned wd_in_bytes;
//     unsigned off;

//     wd_in_bytes = g_wd / 4;
//     off = wd_in_bytes * y + x / 4;
//     set_plane(x & 3);
//     fb[off] = c;
// }

// void do_stuff()
// {
//     // Read misc register
//     unsigned char misc = inb(0x3cc);
    
//     // Read the feature control register
//     unsigned char fc = inb(0x3ca);
    
//     // Read the input status #1 register
//     unsigned char input1 = inb(0x3c2);
    
//     // Read the input status #2 register
//     unsigned char input2 = inb(0x3ba);
    
//     // Set the current Misc Graphic register
    
//     // Special register access
    
//     // Put the sub-register address in the Address port
//     outb(0x3ce, 0x06);
//     outb(0x3cf, 0xF);
//     // Check if data was written ok
//     unsigned char resData = inb(0x3cf);
    
// 	// Set the VGA chip in 320x200 mode
//     write_regs(g_320x200x256);
    
//     int g_wd = 320;
//     int g_ht = 200;
//     int x, y;

//     /* clear screen */
//     for(y = 0; y < g_ht; y++)
//     {
//         for(x = 0; x < g_wd; x++)
//         {
//             write_pixel8x(x, y, 0);
//         }
//     }
    
//     /* draw 2-color X */
//     for(y = 0; y < g_ht; y++)
//     {
//         write_pixel8x((g_wd - g_ht) / 2 + y, y, 1);
//         write_pixel8x((g_ht + g_wd) / 2 - y, y, 2);
//     }
// }
