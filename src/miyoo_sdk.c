
#define DEBUG

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <linux/fb.h>
#include <pthread.h>
#include <SDL/SDL.h>
#include "menu.h"
#include "miyoo_sdk.h"
#include <alsa/asoundlib.h>
#include <time.h>

static snd_pcm_t *handle;

volatile unsigned short  gp2x_palette[512][2];

static int fb_size=(320*240*2)+(16*2);
//static int mmuHackStatus=0;

//unsigned long gp2x_ticks_per_second=7372800/1000;
unsigned long   gp2x_dev[5]={0,0,0,0,0};
//unsigned long gp2x_physvram[4]={0,0,0,0};

unsigned short *framebuffer16[4]={0,0,0,0};
static unsigned short *framebuffer_mmap[4]={0,0,0,0};
unsigned short *gp2x_logvram15[2], gp2x_sound_buffer[4+((44100*2)*8)]; //*2=stereo, *4=max buffers
volatile unsigned short *gp2x_memregs;
volatile unsigned long *gp2x_memregl;
volatile unsigned long *gp2x_blitter = NULL;
unsigned int *gp2x_intVectors;
unsigned char  *framebuffer8[4], *gp2x_screen8prev, *gp2x_logvram8[2];

volatile short *pOutput[8];
int InitFramebuffer=0;
int Timer=0;
volatile int SoundThreadFlag=0;
volatile int CurrentSoundBank=0;
int CurrentFrameBuffer=0;
int CurrentFrag=0;
int CurrentFrameLimit=0;
unsigned int ExistingIntHandler;
snd_pcm_uframes_t samples=0;

extern unsigned short * pOutputScreen;

// 1024x8   8x8 font, i love it :)
const unsigned int font8x8[]= {0x0,0x0,0xc3663c18,0x3c2424e7,0xe724243c,0x183c66c3,0xc16f3818,0x18386fc1,0x83f61c18,0x181cf683,0xe7c3993c,0x3c99c3,0x3f7fffff,0xe7cf9f,0x3c99c3e7,0xe7c399,0x3160c080,0x40e1b,0xcbcbc37e,0x7ec3c3db,0x3c3c3c18,0x81c087e,0x8683818,0x60f0e08,0x81422418,0x18244281,0xbd5a2418,0x18245abd,0x818181ff,0xff8181,0xa1c181ff,0xff8995,0x63633e,0x3e6363,0x606060,0x606060,0x3e60603e,0x3e0303,0x3e60603e,0x3e6060,0x3e636363,0x606060,0x3e03033e,0x3e6060,0x3e03033e,0x3e6363,0x60603e,0x606060,0x3e63633e,0x3e6363,0x3e63633e,0x3e6060,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x18181818,0x180018,0x666666,0x0,0x367f3600,0x367f36,0x3c067c18,0x183e60,0x18366600,0x62660c,0xe1c361c,0x6e337b,0x181818,0x0,0x18183870,0x703818,0x18181c0e,0xe1c18,0xff3c6600,0x663c,0x7e181800,0x1818,0x0,0x60c0c00,0x7e000000,0x0,0x0,0x181800,0x18306040,0x2060c,0x6e76663c,0x3c6666,0x18181c18,0x7e1818,0x3060663c,0x7e0c18,0x3018307e,0x3c6660,0x363c3830,0x30307e,0x603e067e,0x3c6660,0x3e06063c,0x3c6666,0x1830607e,0xc0c0c,0x3c66663c,0x3c6666,0x7c66663c,0x1c3060,0x181800,0x1818,0x181800,0xc1818,0xc183060,0x603018,0x7e0000,0x7e00,0x30180c06,0x60c18,0x3060663c,0x180018,0x5676663c,0x7c0676,0x66663c18,0x66667e,0x3e66663e,0x3e6666,0x606663c,0x3c6606,0x6666361e,0x1e3666,0x3e06067e,0x7e0606,0x3e06067e,0x60606,0x7606067c,0x7c6666,0x7e666666,0x666666,0x1818183c,0x3c1818,0x60606060,0x3c6660,0xe1e3666,0x66361e,0x6060606,0x7e0606,0x6b7f7763,0x636363,0x7e7e6e66,0x666676,0x6666663c,0x3c6666,0x3e66663e,0x60606,0x6666663c,0x6c366e,0x3e66663e,0x666636,0x3c06663c,0x3c6660,0x1818187e,0x181818,0x66666666,0x7c6666,0x66666666,0x183c66,0x6b636363,0x63777f,0x183c6666,0x66663c,0x3c666666,0x181818,0x1830607e,0x7e060c,0x18181878,0x781818,0x180c0602,0x406030,0x1818181e,0x1e1818,0x63361c08,0x0,0x0,0x7f0000,0xc060300,0x0,0x603c0000,0x7c667c,0x663e0606,0x3e6666,0x63c0000,0x3c0606,0x667c6060,0x7c6666,0x663c0000,0x3c067e,0xc3e0c38,0xc0c0c,0x667c0000,0x3e607c66,0x663e0606,0x666666,0x181c0018,0x3c1818,0x18180018,0xe181818,0x36660606,0x66361e,0x1818181c,0x3c1818,0x7f370000,0x63636b,0x663e0000,0x666666,0x663c0000,0x3c6666,0x663e0000,0x63e6666,0x667c0000,0x607c6666,0x663e0000,0x60606,0x67c0000,0x3e603c,0x187e1800,0x701818,0x66660000,0x7c6666,0x66660000,0x183c66,0x63630000,0x363e6b,0x3c660000,0x663c18,0x66660000,0x3e607c66,0x307e0000,0x7e0c18,0xc181870,0x701818,0x18181818,0x18181818,0x3018180e,0xe1818,0x794f0600,0x30};

pthread_t       gp2x_sound_thread=0, gp2x_sound_thread_exit=0;
struct fb_fix_screeninfo fb0_fixed_info;
struct fb_fix_screeninfo fb1_fixed_info;

int altVolumeCtrl = 0;

SDL_Surface *screen, *sgame = NULL;
SDL_Surface *game[4] = {0,0,0,0};
SDL_Surface *optimizedSgame = NULL;

/* 
########################
Graphics functions
########################
 */
unsigned short gpi_getbppmode()
{
	return 16;

	//SDL_DisplayMode d;

	//SDL_GetCurrentDisplayMode(&d);

	//return d.?
}



static void debug(char *text, int pause)
{
	unsigned short bppmode = gpi_getbppmode();
	//bppmode=gp2x_memregs[0x28DA>>1];
	//bppmode>>=9;
	//bppmode<<=3;
	
	if(bppmode==8)
	{
		gp_clearFramebuffer8(framebuffer8[currFB],0);
		gp_drawString(0,0,strlen(text),text,0x51,framebuffer16[currFB]);
	}
	else
	{
		gp_clearFramebuffer16(framebuffer16[currFB],0);
		gp_drawString(0,0,strlen(text),text,(unsigned short)MENU_RGB(31,31,31),framebuffer16[currFB]);
	}
	MenuFlip();
	if(pause)	MenuPause();

}
static __inline__
void gp_drawPixel8 ( int x, int y, unsigned char c, unsigned char *framebuffer ) 
{
	*(framebuffer +(320*y)+x ) = c;
}

static int clipping_x1 = 0; 
static int clipping_x2 = 319;
static int clipping_y1 = 0;
static int clipping_y2 = 239;

void gp_setClipping(int x1, int y1, int x2, int y2) {
	if (x1 < 0) x1 = 0;
	if (x1 > 319) x1 = 319;
	if (x2 < 0) x2 = 0;
	if (x2 > 319) x2 = 319;
	if (y1 < 0) y1 = 0;
	if (y1 > 239) y1 = 239;
	if (y2 < 0) y2 = 0;
	if (y2 > 239) y2 = 239;

	if (x1 < x2) {
		clipping_x1 = x1;
		clipping_x2 = x2;
	} else {
		clipping_x2 = x1;
		clipping_x1 = x2;
	}

	if (y1 < y2) {
		clipping_y1 = y1;
		clipping_y2 = y2;
	} else {
		clipping_y2 = y1;
		clipping_y1 = y2;
	}
}

static __inline__
void gp_drawPixel16 ( int x, int y, unsigned short c, unsigned short *framebuffer ) 
{
	if ((x < clipping_x1) || (x > clipping_x2) || (y < clipping_y1) || (y > clipping_y2)) return;
	*(framebuffer +(320*y)+x ) = c;
}

static
void set_char8x8_16bpp (int xx,int yy,int offset,unsigned short mode,unsigned short *framebuffer) 
{
	unsigned int y, pixel;
	offset *= 2;
	pixel = font8x8[0 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel16(xx+0, yy+y, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel16(xx+1, yy+y, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel16(xx+2, yy+y, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel16(xx+3, yy+y, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel16(xx+4, yy+y, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel16(xx+5, yy+y, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel16(xx+6, yy+y, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel16(xx+7, yy+y, mode, framebuffer);
	}
	pixel = font8x8[1 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel16(xx+0, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel16(xx+1, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel16(xx+2, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel16(xx+3, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel16(xx+4, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel16(xx+5, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel16(xx+6, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel16(xx+7, yy+y+4, mode, framebuffer);

	}
}

static
void set_char8x8_8bpp (int xx,int yy,int offset,unsigned char mode,unsigned char *framebuffer) 
{
	unsigned int y, pixel;
	offset *= 2;
	pixel = font8x8[0 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel8(xx+0, yy+y, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel8(xx+1, yy+y, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel8(xx+2, yy+y, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel8(xx+3, yy+y, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel8(xx+4, yy+y, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel8(xx+5, yy+y, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel8(xx+6, yy+y, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel8(xx+7, yy+y, mode, framebuffer);
	}
	pixel = font8x8[1 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel8(xx+0, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel8(xx+1, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel8(xx+2, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel8(xx+3, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel8(xx+4, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel8(xx+5, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel8(xx+6, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel8(xx+7, yy+y+4, mode, framebuffer);

	}
}

void gp_drawString (int x,int y,int len,char *buffer,unsigned short color,void *framebuffer) 
{
	int l,base=0;
	unsigned short bppmode = gpi_getbppmode();
	//bppmode=gp2x_memregs[0x28DA>>1];


	//bppmode>>=9;
	//bppmode<<=3;

		for (l=0;l<len;l++) 
		{
			if (bppmode==8)
			{
				set_char8x8_8bpp (x+base,y,buffer[l],color,(unsigned char*)framebuffer);
			}
			else 
			{
				set_char8x8_16bpp (x+base,y,buffer[l],color,(short unsigned int*)framebuffer);
			}
			base+=8;
		}
}

void gp_clearFramebuffer16(unsigned short *framebuffer, unsigned short pal)
{
	int x,y;
	for (y=0;y<240;y++)
	{
		for (x=0;x<320;x++)
		{
			*framebuffer++ = pal;
		}
	}
}

void gp_clearFramebuffer8(unsigned char *framebuffer, unsigned char pal)
{
	int x,y;
	for (y=0;y<240;y++)
	{
		for (x=0;x<320;x++)
		{
			*framebuffer++ = pal;
		}
	}
}

void gp_clearFramebuffer(void *framebuffer, unsigned int pal)
{
	unsigned short bppmode;
	bppmode=gp2x_memregs[0x28DA>>1];
	bppmode>>=9;
	bppmode<<=3;
	if(bppmode==8) 	gp_clearFramebuffer8((unsigned char*)framebuffer,(unsigned char)pal);
	else 					gp_clearFramebuffer16((unsigned short*)framebuffer,(unsigned short)pal);
}




unsigned int gp_getButton(unsigned char enable_diagnals)
{
	SDL_PumpEvents();
	Uint8 *keystate = SDL_GetKeyState(NULL);

	unsigned int ret = 0;

	if (keystate[SDLK_UP]) ret |= 1<<INP_BUTTON_UP;
	if (keystate[SDLK_DOWN]) ret |= 1<<INP_BUTTON_DOWN;
	if (keystate[SDLK_LEFT]) ret |= 1<<INP_BUTTON_LEFT;
	if (keystate[SDLK_RIGHT]) ret |= 1<<INP_BUTTON_RIGHT;
	if (keystate[SDLK_RETURN]) ret |= 1<<INP_BUTTON_START;
	if (keystate[SDLK_ESCAPE]) ret |= 1<<INP_BUTTON_SELECT;
	if (keystate[SDLK_TAB]) ret |= 1<<INP_BUTTON_L;
	if (keystate[SDLK_BACKSPACE]) ret |= 1<<INP_BUTTON_R;
	if (keystate[SDLK_LALT]) ret |= 1<<INP_BUTTON_A;
	if (keystate[SDLK_LCTRL]) ret |= 1<<INP_BUTTON_B;
	if (keystate[SDLK_LSHIFT]) ret |= 1<<INP_BUTTON_X;
	if (keystate[SDLK_SPACE]) ret |= 1<<INP_BUTTON_Y;
	if (keystate[SDLK_RCTRL]) ret |= 1<<INP_BUTTON_HOME;

	return ret;
}

void gp_initGraphics(unsigned short bpp, int flip, int applyMmuHack)
{
	
	int x = 0;
	unsigned int key = 0;
	unsigned int offset = 0;
	char buf[256];
	
	gp_setClipping(0, 0, 319, 239);

#ifdef DEBUG
	printf("Entering gp_initGraphics....\r\n");
#endif
	/*
	First check that frame buffer memory has not already been setup
	*/
	if (!InitFramebuffer)
	{
#ifdef DEBUG
		sprintf(buf, "Initing buffer\r\n");
		printf(buf);
#endif 
		if (!framebuffer_mmap[0]) framebuffer_mmap[0]=(void *)malloc(fb_size);
		if (!framebuffer_mmap[1]) framebuffer_mmap[1]=(void *)malloc(fb_size);
		if (!framebuffer_mmap[2]) framebuffer_mmap[2]=(void *)malloc(fb_size);
		if (!framebuffer_mmap[3]) framebuffer_mmap[3]=(void *)malloc(fb_size);

		// offset externally visible buffers by 8
		// this allows DrMD to not worry about clipping
		framebuffer16[0]=framebuffer_mmap[0]+8;
		framebuffer16[1]=framebuffer_mmap[1]+8;
		framebuffer16[2]=framebuffer_mmap[2]+8;
		framebuffer16[3]=framebuffer_mmap[3]+8;
		//ofset physical buffer as well
		//gp2x_physvram[0]=framebuffer16[0];
		//gp2x_physvram[1]=framebuffer16[1];
		//gp2x_physvram[2]=framebuffer16[2];
		//gp2x_physvram[3]=framebuffer16[3];
		// clear all the framebuffers to black
		// otherwise you get crap displayed on the screen the first time
		// you start 
		framebuffer8[0]=(unsigned char*)framebuffer16[0];
		framebuffer8[1]=(unsigned char*)framebuffer16[1];
		framebuffer8[2]=(unsigned char*)framebuffer16[2];
		framebuffer8[3]=(unsigned char*)framebuffer16[3];

		// Clear the frame buffers
		memset(framebuffer16[0],0,320*240*2);
		memset(framebuffer16[1],0,320*240*2);
		memset(framebuffer16[2],0,320*240*2);
		memset(framebuffer16[3],0,320*240*2);
	
		InitFramebuffer=1;
	}
	
	
    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
	fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
	exit(1);
    }
    atexit(SDL_Quit);

	if (screen == NULL)
{
	screen = SDL_SetVideoMode(320, 240, bpp, SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF);
	if ( screen == NULL ) {
		fprintf(stderr, "Unable to set 320x240 video: %s\n", SDL_GetError());
		exit(1);
	}

	sgame = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 240, screen->format->BitsPerPixel,
						screen->format->Rmask,
						screen->format->Gmask,
						screen->format->Bmask, 0);
	if ( sgame == NULL ) {
		fprintf(stderr, "Unable to create 320x240 surface: %s\n", SDL_GetError());
		exit(1);
	}
	optimizedSgame = SDL_DisplayFormat(sgame);
	if ( sgame == NULL ) {
		fprintf(stderr, "Unable to optimize surface: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_FreeSurface(sgame);
	sgame = optimizedSgame;

	for (x=0; x<4; x++)
	{
		game[x] = SDL_CreateRGBSurfaceFrom(framebuffer16[x], 320, 240, screen->format->BitsPerPixel,
				    //320 * (screen->format->BitsPerPixel / 8),
				    sgame->pitch,
		                    screen->format->Rmask,
		                    screen->format->Gmask,
		                    screen->format->Bmask, 0);
		if ( game[x] == NULL ) {
		fprintf(stderr, "Unable to create 320x240 surface: %s\n", SDL_GetError());
		exit(1);
		}
	}
	SDL_ShowCursor(SDL_DISABLE);
}
	
	gp_setFramebuffer(flip,1);
	
#ifdef DEBUG
	printf("Leaving gp_initGraphics....\r\n");
#endif 
}

void gp_setFramebuffer(int flip, int sync)
{

	//printf("%d\n", flip);

	//SDL_LockSurface( sgame );
	//memcpy(sgame->pixels, framebuffer16[flip], fb_size);
	//SDL_UnlockSurface( sgame );
	//SDL_BlitSurface(sgame, NULL, screen, NULL);
	//SDL_SoftStretch( sgame, NULL, screen, NULL );

	//SDL_SoftStretch( game[CurrentFrameBuffer], NULL, screen, NULL );
	SDL_BlitSurface(game[flip], NULL, screen, NULL);

	SDL_Flip(screen);
}

void gp2x_video_setpalette(void) 
{ 
	//unsigned short *g=(unsigned short *)gp2x_palette; int i=512; 
	//gp2x_memregs[0x2958>>1]=0;
	//while(i--) gp2x_memregs[0x295A>>1]=*g++; 
} 

/* 
########################
Sound functions
########################
 */
static
void *gp2x_sound_play(void)
{

	while(! gp2x_sound_thread_exit)
	{
		Timer++;
		CurrentSoundBank++;
		if (CurrentSoundBank >= 8) CurrentSoundBank = 0;
		snd_pcm_writei(handle, (void *)pOutput[CurrentSoundBank], samples);
	}
}

void gp2x_sound_play_bank(int bank)
{
}

void gp2x_sound_sync(void)
{
}

void gp2x_sound_volume(int l, int r) 
{ 
} 

unsigned long gp_timer_read(void)
{
	// Once again another peice of direct hardware access bites the dust
	// the code below is broken in firmware 2.1.1 so I've replaced it with a
	// to a linux function which seems to work
	//return gp2x_memregl[0x0A00>>2]/gp2x_ticks_per_second;
	struct timeval tval; // timing

	gettimeofday(&tval, 0);
	//tval.tv_usec
	//tval.tv_sec
	return (tval.tv_sec*1000000)+tval.tv_usec;
}

int gp_initSound(int rate, int bits, int stereo, int Hz, int frag, int frame_limit)
{
	int status;
	int i=0;
	int nonblocking=1;
	unsigned int bufferStart=0, period_time;
	int result;
	char text[256];

	snd_pcm_hw_params_t *params;
	uint32_t val;
	int32_t dir = 0;

	if (handle && CurrentFrameLimit != frame_limit) {
		snd_pcm_drain(handle);
		snd_pcm_close(handle);
	}
	/* Open PCM device for playback. */
	int32_t rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
		
	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
	{
		fprintf(stderr, "unable to open PCM device: %s\n", snd_strerror(rc));
		return 1;
	}
	
	snd_pcm_nonblock(handle, 0);

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	rc = snd_pcm_hw_params_any(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_any %s\n", snd_strerror(rc));
		return 1;
	}

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_access %s\n", snd_strerror(rc));
		return 1;
	}

	/* Signed 16-bit little-endian format */
	rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_format %s\n", snd_strerror(rc));
		return 1;
	}

	/* Two channels (stereo) */
	rc = snd_pcm_hw_params_set_channels(handle, params, 2);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_channels %s\n", snd_strerror(rc));
		return 1;
	}
	
	val = rate;
	rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_rate_near %s\n", snd_strerror(rc));
		return 1;
	}

	samples = val / frame_limit;
	period_time = samples / val * 1000000;
	CurrentFrameLimit = frame_limit;
	rc = snd_pcm_hw_params_set_period_size_near(handle, params, &samples, NULL);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return 1;
	}
	snd_pcm_uframes_t buf_size = samples*4;
	rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buf_size);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return 1;
	}
	rc = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_period_time_near %s\n", snd_strerror(rc));
		return 1;
	}
	printf("Actual Audio Freq %d Period %d bufsize %d period_time %d frame_limit %d\n",rate, samples, buf_size, period_time, frame_limit);
	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Unable to set hw parameters: %s\n", snd_strerror(rc));
		return 1;
	}
	
	gp2x_sound_buffer[1]=(gp2x_sound_buffer[0]=(rate/Hz)) << (stereo + (bits==16));
	gp2x_sound_buffer[2]=(1000000000/Hz)&0xFFFF;
	gp2x_sound_buffer[3]=(1000000000/Hz)>>16;
 
	bufferStart= &gp2x_sound_buffer[4];
	pOutput[0] = (short*)bufferStart+(0*gp2x_sound_buffer[1]);
	pOutput[1] = (short*)bufferStart+(1*gp2x_sound_buffer[1]);
	pOutput[2] = (short*)bufferStart+(2*gp2x_sound_buffer[1]);
	pOutput[3] = (short*)bufferStart+(3*gp2x_sound_buffer[1]);
	pOutput[4] = (short*)bufferStart+(4*gp2x_sound_buffer[1]);
	pOutput[5] = (short*)bufferStart+(5*gp2x_sound_buffer[1]);
	pOutput[6] = (short*)bufferStart+(6*gp2x_sound_buffer[1]);
	pOutput[7] = (short*)bufferStart+(7*gp2x_sound_buffer[1]);

	if(!gp2x_sound_thread)
	{
		pthread_create( &gp2x_sound_thread, NULL, gp2x_sound_play, NULL);
	}

	for(i=0;i<(gp2x_sound_buffer[1]*8);i++)
	{
		gp2x_sound_buffer[4+i] = 0;
	}
	
	return(0);
}

void gp_stopSound(void)
{
	unsigned int i=0;
	gp2x_sound_thread_exit=1;

	for(i=0;i<(gp2x_sound_buffer[1]*8);i++)
	{
		gp2x_sound_buffer[4+i] = 0;
	}
	gp2x_sound_thread=0;
	gp2x_sound_thread_exit=0;
	CurrentSoundBank=0;
}


/* 
########################
System functions
########################
 */
void gp_Reset(void)
{

}

// SNES9X2002 scaler macros:
#define AVERAGE(z, x) ((((z) & 0xF7DEF7DE) >> 1) + (((x) & 0xF7DEF7DE) >> 1))
#define AVERAGEHI(AB) ((((AB) & 0xF7DE0000) >> 1) + (((AB) & 0xF7DE) << 15))
#define AVERAGELO(CD) ((((CD) & 0xF7DE) >> 1) + (((CD) & 0xF7DE0000) >> 17))
#define Half(A) (((A) >> 1) & 0x7BEF)
#define Quarter(A) (((A) >> 2) & 0x39E7)
#define RestHalf(A) ((A) & 0x0821)
#define RestQuarter(A) ((A) & 0x1863)
#define Corr1_3(A, B)     Quarter(RestQuarter(A) + (RestHalf(B) << 1) + RestQuarter(B))
#define Corr3_1(A, B)     Quarter((RestHalf(A) << 1) + RestQuarter(A) + RestQuarter(B))
#define Corr1_1(A, B)     ((A) & (B) & 0x0821)
#define Weight1_3(A, B)   (Quarter(A) + Half(B) + Quarter(B) + Corr1_3(A, B))
#define Weight3_1(A, B)   (Half(A) + Quarter(A) + Quarter(B) + Corr3_1(A, B))
#define Weight1_1(A, B)   (Half(A) + Half(B) + Corr1_1(A, B))

// Old scaling macro
#define COLORMIX(a, b) ( ((((a & 0xF81F) + (b & 0xF81F)) >> 1) & 0xF81F) | ((((a & 0x07E0) + (b & 0x07E0)) >> 1) & 0x07E0) )

// Line scaling macros
#define ScaleLineFast(Target, Source, x) \
	do { \
		uint16_t* target = Target; \
		uint16_t* source = Source; \
		unsigned short w = x; \
		while (w != 0) { \
			target[0] = source[0]; \
			target[1] = source[1]; \
			target[2] = source[2]; \
			target[3] = Weight1_1(source[2], source[3]); \
			target[4] = source[3]; \
			target += 5; \
			source += 4; \
			w--; \
		} \
	} while (0)

#define ScaleLineSlow(Target, Source, x) \
	do { \
		uint16_t* target = Target; \
		uint16_t* source = Source; \
		unsigned short w = x; \
		while (w != 0) { \
			uint16_t _1 = source[0]; \
			target[0] = _1; \
			uint16_t _2 = source[1]; \
			target[1] = Weight1_3(_1, _2); \
			uint16_t _3 = source[2]; \
			target[2] = Weight1_1(_2, _3); \
			uint16_t _4 = source[3]; \
			target[3] = Weight3_1(_3, _4); \
			target[4] = _4; \
			target += 5; \
			source += 4; \
			w--; \
		} \
	} while (0)

// Line interpolation macros
#define InterpolateLines_1_1(Target, SourceA, SourceB, x) \
	do { \
		uint16_t* target = Target; \
		uint16_t* sourceA = SourceA; \
		uint16_t* sourceB = SourceB; \
		unsigned short w = x; \
		while (w != 0) { \
			(target)[0] = Weight1_1((sourceA)[0], (sourceB)[0]); \
			(target)[1] = Weight1_1((sourceA)[1], (sourceB)[1]); \
			(target)[2] = Weight1_1((sourceA)[2], (sourceB)[2]); \
			(target)[3] = Weight1_1((sourceA)[3], (sourceB)[3]); \
			(target)[4] = Weight1_1((sourceA)[4], (sourceB)[4]); \
			(target) += 5; \
			(sourceA) += 5; \
			(sourceB) += 5; \
			w--; \
		} \
	} while (0)

#define InterpolateLines_1_3(Target, SourceA, SourceB, x) \
	do { \
		uint16_t* target = Target; \
		uint16_t* sourceA = SourceA; \
		uint16_t* sourceB = SourceB; \
		unsigned short w = x; \
		while (w != 0) { \
			(target)[0] = Weight1_3((sourceA)[0], (sourceB)[0]); \
			(target)[1] = Weight1_3((sourceA)[1], (sourceB)[1]); \
			(target)[2] = Weight1_3((sourceA)[2], (sourceB)[2]); \
			(target)[3] = Weight1_3((sourceA)[3], (sourceB)[3]); \
			(target)[4] = Weight1_3((sourceA)[4], (sourceB)[4]); \
			(target) += 5; \
			(sourceA) += 5; \
			(sourceB) += 5; \
			w--; \
		} \
	} while (0)

#define InterpolateLines_3_1(Target, SourceA, SourceB, x) \
	do { \
		uint16_t* target = Target; \
		uint16_t* sourceA = SourceA; \
		uint16_t* sourceB = SourceB; \
		unsigned short w = x; \
		while (w != 0) { \
			(target)[0] = Weight3_1((sourceA)[0], (sourceB)[0]); \
			(target)[1] = Weight3_1((sourceA)[1], (sourceB)[1]); \
			(target)[2] = Weight3_1((sourceA)[2], (sourceB)[2]); \
			(target)[3] = Weight3_1((sourceA)[3], (sourceB)[3]); \
			(target)[4] = Weight3_1((sourceA)[4], (sourceB)[4]); \
			(target) += 5; \
			(sourceA) += 5; \
			(sourceB) += 5; \
			w--; \
		} \
	} while (0)

void gp_video_RGB_setscaling(int W, int H)
{
	uint16_t * pSource = (uint16_t *)pOutputScreen;
	uint16_t * pTarget = (uint16_t *)framebuffer16[currFB];
	unsigned short y, x;
	if (H == 224)
	{
		pSource += 2560;
		uint16_t l1[320];
		uint16_t l2[320];
		uint16_t* l3;

		for (y = 0; y < 16; y++)
		{
			x = 64;
			// Direct copy
			pSource += 32;
			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			l3 = pTarget;
			pSource += 256 + 64;
			pTarget += 320;

			// Interpolate 1/3
			ScaleLineSlow(l2, pSource, x);
			InterpolateLines_1_3(pTarget, l3, l2, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(l1, pSource, x);
			InterpolateLines_1_3(pTarget, l2, l1, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(l2, pSource, x);
			InterpolateLines_1_3(pTarget, l1, l2, x);
			pSource += 256 + 64;
			pTarget += 320;

			// Interpolate 1/1
			ScaleLineSlow(l1, pSource, x);
			InterpolateLines_1_1(pTarget, l2, l1, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(l2, pSource, x);
			InterpolateLines_1_1(pTarget, l1, l2, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(l1, pSource, x);
			InterpolateLines_1_1(pTarget, l2, l1, x);
			pSource += 256 + 64;
			pTarget += 320;

			// Interpolate 3/1
			ScaleLineSlow(l2, pSource, x);
			InterpolateLines_3_1(pTarget, l1, l2, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(l1, pSource, x);
			InterpolateLines_3_1(pTarget, l2, l1, x);
			pSource += 256 + 64;
			pTarget += 320;

			l3 = pTarget + 320;
			ScaleLineSlow(l3, pSource, x);
			InterpolateLines_3_1(pTarget, l1, l3, x);
			pSource += 256 + 64;
			pTarget += 320;

			// Direct copy
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 32;
			pTarget += 320;
		}
	}
	else // H == 239
	{
		for (y = H; y != 0; y--)
		{
			x = 64;
			pSource += 32;
			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 32;
			pTarget += 320;
		}
	}
}

void gp_video_RGB_setscaling_fast(int W, int H)
{
	uint16_t * pSource = (uint16_t *)pOutputScreen;
	uint16_t * pTarget = (uint16_t *)framebuffer16[currFB];
	unsigned short y, x;
	if (H == 224)
	{
		pSource += 2560;

		for (y = 0; y < 16; y++)
		{
			x = 64;
			pSource += 32;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 64;
			pTarget += 320;

			// Skip a line
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			
			// Interpolate the skipped line
			InterpolateLines_1_1(pTarget - 320, pTarget - 640, pTarget, x);
			pSource += 256 + 64;
			pTarget += 320;

			ScaleLineSlow(pTarget, pSource, x);
			pSource += 256 + 32;
			pTarget += 320;
		}
	}
	else // H == 239
	{
		for (y = H; y != 0; y--)
		{
			x = 64;
			pSource += 32;
			ScaleLineFast(pTarget, pSource, x);
			pSource += 256 + 32;
			pTarget += 320;
		}
	}
}

void gp_video_RGB_setHZscaling(int W, int H)
{
	uint16_t * pSource = (uint16_t *)pOutputScreen;
	uint16_t * pTarget = (uint16_t *)framebuffer16[currFB];
	unsigned short y, x;

	if (H == 224)
	{
		pSource += 2560;
		pTarget += 2560;
	}
	for (y = H; y != 0; y--)
	{ // 239 || 224
		x = 64;
		pSource += 32;
		ScaleLineSlow(pTarget,pSource, x);
		pSource += 256 + 32;
		pTarget += 320;
	}
}

void gp_video_RGB_setHZscaling_fast(int W, int H)
{
	uint16_t * pSource = (uint16_t *)pOutputScreen;
	uint16_t * pTarget = (uint16_t *)framebuffer16[currFB];
	unsigned short y, x;

	if (H == 224)
	{
		pSource += 2560;
		pTarget += 2560;
	}
	for (y = H; y != 0; y--)
	{ // 239 || 224
		x = 64;
		pSource += 32;
		ScaleLineFast(pTarget,pSource, x);
		pSource += 256 + 32;
		pTarget += 320;
	}
}

void gp_setCpuspeed(unsigned int MHZ)
{

}

// craigix: --trc 6 --tras 4 --twr 1 --tmrd 1 --trfc 1 --trp 2 --trcd 2
// set_RAM_Timings(6, 4, 1, 1, 1, 2, 2);
void set_RAM_Timings(int tRC, int tRAS, int tWR, int tMRD, int tRFC, int tRP, int tRCD)
{
}

void set_gamma(int g100)
{
}

void gp_sound_volume(int l, int r)
{ 
} 
