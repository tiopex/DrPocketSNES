#ifndef _MIYOO_SDK_H_
#define _MIYOO_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SOUND_THREAD_SOUND_ON			1
#define SOUND_THREAD_SOUND_OFF		2
#define SOUND_THREAD_PAUSE				3

#define INP_BUTTON_UP				(0)
#define INP_BUTTON_LEFT				(1)
#define INP_BUTTON_DOWN				(2)
#define INP_BUTTON_RIGHT			(3)
#define INP_BUTTON_START			(4)
#define INP_BUTTON_SELECT			(5)
#define INP_BUTTON_L				(6)
#define INP_BUTTON_R				(7)
#define INP_BUTTON_A				(8)
#define INP_BUTTON_B				(9)
#define INP_BUTTON_X				(10)
#define INP_BUTTON_Y				(11)
#define INP_BUTTON_HOME				(12)

void gp_setClipping(int x1, int y1, int x2, int y2);
void gp_drawString (int x,int y,int len,char *buffer,unsigned short color,void *framebuffer);
void gp_clearFramebuffer16(unsigned short *framebuffer, unsigned short pal);
void gp_clearFramebuffer8(unsigned char *framebuffer, unsigned char pal);
void gp_clearFramebuffer(void *framebuffer, unsigned int pal);
void gp_setCpuspeed(unsigned int cpuspeed);
void gp_initGraphics(unsigned short bpp, int flip, int applyMmuHack);
void gp_setFramebuffer(int flip, int sync);
void gp2x_video_setpalette(void);
int gp_initSound(int rate, int bits, int stereo, int Hz, int frag, int frame_limit);
void gp_stopSound(void);
void gp_Reset(void);
void gp2x_enableIRQ(void);
void gp2x_disableIRQ(void);
void gp2x_sound_volume(int l, int r);
void gp_sound_volume(int l, int r);
unsigned long gp_timer_read(void);
unsigned int gp_getButton(unsigned char enable_diagnals);
void gp_video_RGB_setscaling(int W, int H);
void gp_video_RGB_setHZscaling(int W, int H);
void gp_video_RGB_setscaling_fast(int W, int H);
void gp_video_RGB_setHZscaling_fast(int W, int H);
void gp2x_sound_play_bank(int bank);
void gp2x_sound_sync(void);
void set_gamma(int g100);
void set_RAM_Timings(int tRC, int tRAS, int tWR, int tMRD, int tRFC, int tRP, int tRCD);

extern volatile int SoundThreadFlag;
extern volatile int CurrentSoundBank;
extern int CurrentFrameBuffer;
extern volatile short *pOutput[];
extern unsigned short *framebuffer16[];
extern unsigned long gp2x_physvram[];
extern unsigned char *framebuffer8[];
extern volatile unsigned short  gp2x_palette[512][2];
extern volatile unsigned short *gp2x_memregs;
extern volatile unsigned long *gp2x_memregl;
extern volatile unsigned long *gp2x_blitter;
 
#ifdef __cplusplus
}
#endif

#endif

