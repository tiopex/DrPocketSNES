#include "menu.h"
#include "theme.h"
#include "config.h"
#include "screenshot.h"
#include "graphics.h"
#include "png.h"

#define MENU_AREA_Y_START	50
#define MENU_AREA_Y_END		227

#define PPU_IGNORE_FIXEDCOLCHANGES 			(1<<0)
#define PPU_IGNORE_WINDOW					(1<<1)
#define PPU_IGNORE_ADDSUB					(1<<2)
#define PPU_IGNORE_PALWRITE				 	(1<<3)
#define GFX_IGNORE_OBJ				 		(1<<4)
#define GFX_IGNORE_BG0				 		(1<<5)
#define GFX_IGNORE_BG1				 		(1<<6)
#define GFX_IGNORE_BG2				 		(1<<7)
#define GFX_IGNORE_BG3				 		(1<<8)

char romDir[MAX_PATH+1];
char snesRomDir[MAX_PATH+1];

#define ROM_SELECTOR_DEFAULT_FOCUS		3

DIRDATA dir;

#if defined (__GP2X__) || defined(__PANDORA__)
unsigned short cpuSpeedLookup[46]={ 
					10,20, 30, 40, 50,
					60,70, 80, 90,100,
					110,120,130,144,150,
					160,170,180,190,200,
					210,220,230,240,250,
					255,260,265,270,275,
					280,285,290,295,300,
					305,310,315,320,325,
					330,335,340,345,350,
					355};
#endif

#if defined (__WIZ__)	
unsigned short cpuSpeedLookup[46]={ 
					200,250, 300, 350, 400,
					450,500, 520, 540,560,
					580,590,600,610,620,
					630,640,650,660,670,
					680,690,700,710,720,
					730,740,750,760,770,
					780,790,800,810,820,
					830,840,850,860,870,
					875,880,885,890,895,
					900};
#endif

extern volatile int timer;
static int menutileXscroll=0;
static int menutileYscroll=0;
static int headerDone[4]; // variable that records if header graphics have been rendered or not
int quickSavePresent=0;
struct ROM_LIST_RECORD
{
	char filename[MAX_PATH+1];
	char type;
};

static struct ROM_LIST_RECORD romList[MAX_ROMS];
struct SNES_MENU_OPTIONS snesMenuOptions;

static int romCount;
char currentRomFilename[MAX_PATH+1]="";
int currFB=0;
int prevFB=0;
int currentEmuMode=EMU_MODE_SNES;

char currentWorkingDir[MAX_PATH+1];
char snesOptionsDir[MAX_PATH+1];
char snesSramDir[MAX_PATH+1];
char snesSaveStateDir[MAX_PATH+1];
#if defined(__WIZ__)
float soundRates[3]={22050.0,32000.0,44100.0};
#else
float soundRates[5]={8000.0,11025.0,16000.0,22050.0,44100.0};
#endif
char menutext[256][50];

struct SAVE_STATE saveState[10];  // holds the filenames for the savestate and "inuse" flags
char saveStateName[MAX_PATH+MAX_PATH+2];       // holds the last filename to be scanned for save states

// a few vars for persistent file selector
int lastSelected = -1; 
int defaultDir;

unsigned char gammaConv[32*29]={   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                                    0, 2, 3, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 24, 25, 26, 27, 28, 29, 29, 30, 31,
                                    0, 3, 5, 7, 8, 9, 10, 11, 13, 14, 15, 16, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 28, 28, 29, 30, 30, 31,
                                    0, 4, 6, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 30, 31,
                                    0, 6, 8, 10, 11, 12, 14, 15, 16, 17, 18, 18, 19, 20, 21, 22, 22, 23, 24, 24, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31,
                                    0, 7, 9, 11, 12, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 24, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31,
                                    0, 8, 10, 12, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 27, 27, 28, 28, 28, 29, 29, 30, 30, 31, 31,
                                    0, 9, 11, 13, 15, 16, 17, 18, 19, 20, 21, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 29, 30, 30, 31, 31,
                                    0, 10, 12, 14, 16, 17, 18, 19, 20, 21, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 28, 29, 29, 30, 30, 30, 31, 31,
                                    0, 11, 13, 15, 17, 18, 19, 20, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31,
                                    0, 12, 14, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31,
                                    0, 12, 15, 17, 18, 19, 20, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 30, 31, 31,
                                    0, 13, 16, 17, 19, 20, 21, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 30, 31, 31,
                                    0, 14, 16, 18, 19, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 31, 31, 31,
                                    0, 14, 17, 18, 20, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31,
                                    0, 15, 17, 19, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31,
                                    0, 16, 18, 19, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31,
                                    0, 16, 18, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
                                    0, 17, 19, 20, 21, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
                                    0, 17, 19, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
                                    0, 17, 20, 21, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
                                    0, 18, 20, 21, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31,
                                    0, 18, 20, 22, 23, 23, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31,
                                    0, 19, 21, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31,
                                    0, 19, 21, 22, 23, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
                                    0, 19, 21, 22, 23, 24, 25, 25, 26, 26, 27, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
                                    0, 20, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
                                    0, 20, 22, 23, 24, 24, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
                                    0, 20, 22, 23, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31};


void InitMenu(void) {
	
}
									
void UpdateMenuGraphicsGamma(void)
{
   unsigned int currPix=0;
   unsigned short pixel=0;
   unsigned char R,G,B;
   for(currPix=0;currPix<15360;currPix++)
   {
      // md  0000 bbb0 ggg0 rrr0
      // gp  rrrr rggg ggbb bbbi
      pixel=menuHeaderOrig[currPix];
      R=(pixel>>11)&0x1F; // 0000 0RRR - 3 bits Red
      G=(pixel>>6)&0x1F; 
      B=(pixel>>1)&0x1F;
      
      // Do gamma correction  
      R=gammaConv[R+(0<<5)];
      G=gammaConv[G+(0<<5)];
      B=gammaConv[B+(0<<5)];

      pixel=MENU_RGB(R,G,B);
      menuHeader[currPix]=pixel;
   }
   for(currPix=0;currPix<5120;currPix++)
   {
      // md  0000 bbb0 ggg0 rrr0
      // gp  rrrr rggg ggbb bbbi
      pixel=highLightBarOrig[currPix];
      R=(pixel>>11)&0x1F; // 0000 0RRR - 3 bits Red
      G=(pixel>>6)&0x1F; 
      B=(pixel>>1)&0x1F;
      
      // Do gamma correction  
      R=gammaConv[R+(0<<5)];
      G=gammaConv[G+(0<<5)];
      B=gammaConv[B+(0<<5)];

      pixel=MENU_RGB(R,G,B);
      highLightBar[currPix]=pixel;
   
   }
   
   for(currPix=0;currPix<(MENU_TILE_WIDTH*MENU_TILE_HEIGHT);currPix++)
   {
      // md  0000 bbb0 ggg0 rrr0
      // gp  rrrr rggg ggbb bbbi
      pixel=menuTileOrig[currPix];
      R=(pixel>>11)&0x1F; // 0000 0RRR - 3 bits Red
      G=(pixel>>6)&0x1F; 
      B=(pixel>>1)&0x1F;
      
      // Do gamma correction  
      R=gammaConv[R+(0<<5)];
      G=gammaConv[G+(0<<5)];
      B=gammaConv[B+(0<<5)];

      pixel=MENU_RGB(R,G,B);
      menuTile[currPix]=pixel;
   
   }
}

void SnesDefaultMenuOptions(void)
{
	// no options file loaded, so set to defaults
	snesMenuOptions.menuVer=SNES_OPTIONS_VER;
	snesMenuOptions.frameSkip=0;
	snesMenuOptions.soundOn = 1;
	snesMenuOptions.stereo = 1;
	snesMenuOptions.volume=100; 
	memset(snesMenuOptions.padConfig,0xFF,sizeof(snesMenuOptions.padConfig));
	snesMenuOptions.showFps=0;
	snesMenuOptions.transparency=1;
	snesMenuOptions.gamma=0;
	snesMenuOptions.asmspc700=0;
	snesMenuOptions.SpeedHacks=0;
	snesMenuOptions.soundHack=1;
#if defined(__WIZ__)
	snesMenuOptions.soundRate=0;
	snesMenuOptions.cpuSpeed=8;
#else
	snesMenuOptions.soundRate=3;
	snesMenuOptions.cpuSpeed=19;
#endif
	snesMenuOptions.loadOnInit = 0;
	snesMenuOptions.delayedRasterFX = 1;
}

static void ShowMessage(char *text, int showMessage) {
	if (showMessage) {
		gp_setClipping(0, 0, 310, 239);
		PrintBar(prevFB,240-16);
		gp_drawString(40,228,strlen(text),text,tTextColorFocus,framebuffer16[prevFB]);
		}	
}

int LoadMenuOptions(char *path, char *filename, char *ext, char *optionsmem, int maxsize, int showMessage)
{
	char fullFilename[MAX_PATH+MAX_PATH+1];
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
	FILE *stream;
	int size=0;
	char text[50];
	
	ShowMessage("Loading...", showMessage);
	
    SplitFilename(filename, _filename, _ext);
	sprintf(fullFilename,"%s%s%s.%s",path,DIR_SEP,_filename,ext);
	stream=fopen(fullFilename,"rb");
	if(stream)
	{
		// File exists do try to load it
		fseek(stream,0,SEEK_END);
		size=ftell(stream);
		if (size>maxsize) size=maxsize;
		fseek(stream,0,SEEK_SET);
		fread(optionsmem, 1, size, stream);
		fclose(stream);
		return(0);
	}
	else
	{
		return(1);
	}
}

int SaveMenuOptions(char *path, char *filename, char *ext, char *optionsmem, int maxsize, int showMessage)
{
	char fullFilename[MAX_PATH+MAX_PATH+1];
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
	FILE *stream;
	char text[50];
	
	ShowMessage("Saving...", showMessage);
	
	SplitFilename(filename, _filename, _ext);
	sprintf(fullFilename,"%s%s%s.%s",path,DIR_SEP,_filename,ext);
	stream=fopen(fullFilename,"wb");
	if(stream)
	{
		fwrite(optionsmem, 1, maxsize, stream);
		fclose(stream);
		sync();
		return(0);
	}
	else
	{
		return(1);
	}
}

int DeleteMenuOptions(char *path, char *filename, char *ext, int showMessage)
{
	char fullFilename[MAX_PATH+MAX_PATH+1];
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
	char text[50];
	
	ShowMessage("Deleting...", showMessage);
	
	SplitFilename(filename, _filename, _ext);
	sprintf(fullFilename,"%s%s%s.%s",path,DIR_SEP,_filename,ext);
	remove(fullFilename);
	sync();
	return(0);
}

#ifdef __GIZ__
void sync(void)
{
}
#endif

static void WaitForButtonsUp(void)
{
	int i=0,j=0,z=0;
	
	for(i=0;i<100;i++)
	{
		while(1)
		{     
			InputUpdate(0);
			z=0;
			for (j=0;j<32;j++)
			{
				if (Inp.held[j]) z=1;
			}
			if (z==0) break;
		}
	}
}

void MenuPause()
{
	int i=0,j=0,z=0;
	// wait for keys to be released
	for(i=0;i<100;i++)  // deal with keybounce by checking a few times
	{
		while(1)
		{     
			InputUpdate(0);
			z=0;
			for (j=0;j<32;j++)
			{
				if (Inp.held[j]) z=1;
			}
			if (z==0) break;
		}
	}
	
	for(i=0;i<100;i++)  // deal with keybounce by checking a few times
	{
		while(1)
		{     
			InputUpdate(0);
			z=0;
			for (j=0;j<32;j++)
			{
				if (Inp.held[j]) z=1;
			}
			if (z==1) break;
		}
	}
}
#if defined (__GP2X__) || defined(__WIZ__) || defined(__PANDORA__) || defined(__MIYOO__)
void MenuFlip()
{
	prevFB=currFB;
	gp_setFramebuffer(currFB,1);
    currFB++;
	currFB&=3;
}
#endif
#if defined (__GIZ__)	
void MenuFlip()
{
	prevFB=currFB=0;
	gp_setFramebuffer(currFB,0);
}
#endif
void SplitFilename(char *wholeFilename, char *filename, char *ext)
{
	int len=strlen(wholeFilename);
	int i=0,y=-1;

	ext[0]=0;
	filename[0]=0;
	//Check given string is not null
	if (len<=0)
	{
		return;
	}
	y=-1;
	for(i=len-2;i>0;i--)
	{
		if (wholeFilename[i]=='.')
		{
			y=i;
			break;
		}
	}
	
	if (y>=0)
	{
		memcpy(filename,wholeFilename,y);
		filename[y]=0; // change "." to zero to end string
		memcpy(ext,wholeFilename+y+1,len-(y+1));
		//ext[len-(y+1)+1]=0;
		ext[len-(y+1)]=0;
	}
	else
	{
		strcpy(filename,wholeFilename);
	}
}

//Ensures all directory seperators are valid for system
void CheckDirSep(char *path)
{
	int i=0;
	char dirSepBad[2]={DIR_SEP_BAD};
	char dirSep[2]={DIR_SEP};
	for(i=0;i<strlen(path);i++)
	{
		if(path[i] == dirSepBad[0]) path[i]=dirSep[0];
	}
}


int MenuMessageBox(char *message1,char *message2,char *message3,int mode)
{
  int select=0;
  int subaction=-1;
  int len=0;
  while(subaction==-1)
  {
     InputUpdate(0);
     if (Inp.repeat[INP_BUTTON_UP]) 
     {
       select^=1; // Up
     }
     if (Inp.repeat[INP_BUTTON_DOWN]) 
     {
       select^=1; // Down
     }
     if ((Inp.held[INP_BUTTON_MENU_SELECT]==1) || (Inp.held[INP_BUTTON_MENU_CANCEL]==1))
     {
        subaction=select;
     }
     PrintTile(currFB);
     PrintTitle(currFB);
	 len=strlen(message1);
	 if(len>39)len=39;
     gp_drawString(8,50,len,message1,tTextColorItem,framebuffer16[currFB]);
     len=strlen(message2);
	 if(len>39)len=39;
	 gp_drawString(8,60,len,message2,tTextColorItem,framebuffer16[currFB]);
     len=strlen(message3);
	 if(len>39)len=39;
	 gp_drawString(8,70,len,message3,tTextColorItem,framebuffer16[currFB]);
     switch(mode)
     {
        case 0: // yes no input
	       if(select==0)
	       {
			  PrintBar(currFB, 120-4);
	          gp_drawString(8,120,3,"YES",tTextColorFocus,framebuffer16[currFB]);
	          gp_drawString(8,140,2,"NO",tTextColorItem,framebuffer16[currFB]);
	       }
	       else
	       {
			  PrintBar(currFB, 140-4);
	          gp_drawString(8,120,3,"YES",tTextColorItem,framebuffer16[currFB]);
	          gp_drawString(8,140,2,"NO",tTextColorFocus,framebuffer16[currFB]);
	       
	       }
	       break;
     }
     MenuFlip();
  }
  return(subaction);
}

static
int deleterom(int romindex)
{
	char text[MAX_PATH+1];
	char fullfilename[MAX_PATH+MAX_PATH+1];
	int x;
	FILE *stream=NULL;
	
    PrintTile(currFB);
    PrintTitle(currFB);
    MenuFlip();
	
    sprintf(text,"Deleting Rom..");
    gp_drawString(8,50,strlen(text),text,tTextColorItem,framebuffer16[prevFB]);
	
    sprintf(text,"%s",romList[romindex].filename);
	x=strlen(text);
	if(x>40) x=40;
	gp_drawString(0,60,x,text,tTextColorItem,framebuffer16[prevFB]);
	
	sprintf(fullfilename,"%s%s%s",romDir,DIR_SEP,romList[romindex].filename);
    remove(fullfilename);
	sync();
	
    sprintf(text,"Updating Rom List..");
    gp_drawString(8,70,strlen(text),text,tTextColorItem,framebuffer16[prevFB]);
    for(x=romindex;x<romCount;x++)
    {
		strcpy(romList[x].filename, romList[x+1].filename);
		romList[x].type = romList[x+1].type;
    }
    romCount--;
	
	return(1);
}

static int tileCounter=0;
void PrintTile(int flip)
{
	short x=0,x2=0;
	short y=0,y2=0;
	unsigned short *framebuffer1 = framebuffer16[flip]+(48*320);
	unsigned short *graphics1 = NULL;

	if (isThemeActive() != 0) {
		gDrawBitmap16((unsigned short *)framebuffer16[flip], 0, MENU_AREA_Y_START, 
			tBmpBackground, 0, MENU_AREA_Y_START, tBmpBackground->w,  tBmpBackground->h - MENU_AREA_Y_START + 1);
	} else {

		x2=menutileXscroll;
		y2=(menutileYscroll*MENU_TILE_WIDTH);
		graphics1 = menuTile+y2;
		for (y=0; y<(240-48); y++)
		{
			for (x=0; x<320; x++)
			{
				*framebuffer1++ = graphics1[x2];
				x2++;
				x2&=(MENU_TILE_WIDTH-1);
			}
			y2+=MENU_TILE_WIDTH;
			y2&=((MENU_TILE_HEIGHT*MENU_TILE_WIDTH)-1);
			graphics1=menuTile+y2;
		}

		tileCounter++;
		if (tileCounter > 5)
		{
			tileCounter=0;
			menutileXscroll++;
			if(menutileXscroll>=MENU_TILE_WIDTH) menutileXscroll=0;
		  
			menutileYscroll++;
			if(menutileYscroll>=MENU_TILE_HEIGHT) menutileYscroll=0;
		}
	}  
	return; 
}

void PrintTitle(int flip)
{
	unsigned int x,y;
	char text[] = DRSNES_VERSION;
	//If header already drawn for this layer exit
	if (headerDone[flip]) return;
  
	if (isThemeActive() != 0) {
		gDrawBitmap16((unsigned short *)framebuffer16[flip], 0, 0, tBmpBackground, 0, 0, tBmpBackground->w, MENU_AREA_Y_START);
	} else {
		unsigned short *framebuffer = (unsigned short*)framebuffer16[flip];
		unsigned short *graphics = (unsigned short*)menuHeader;

		for (y=0; y<48; y++)
		{
			for (x=0; x<320; x++)
			{
				*framebuffer++ = *graphics++;
			}
		}
	}
  
	//sprintf(text,"%s",DRSNES_VERSION);
	gp_drawString(175,15,strlen(text),text,tTextColorVersion,framebuffer16[flip]);
	headerDone[currFB] = 1;
}

void PrintBar(int flip, unsigned int givenY)
{
  if (isThemeActive()) {
	gBlendBitmap16((unsigned short *)framebuffer16[flip], 0, givenY, tBmpBar, 0, 0, tBmpBar->w, tBmpBar->h);
  } else {
	unsigned int *framebuffer1 = NULL;
	unsigned int *graphics1 = (unsigned int *)highLightBar;
	unsigned int x,y;

	framebuffer1 = (unsigned int*)framebuffer16[flip]+(givenY*160);
	for (y=0; y<16; y++)
	{
		for (x=0; x<160; x++)
		{
			*framebuffer1++ = *graphics1++;
		}
	}
  }
}

static int StringCompare(char *string1, char *string2)
{
	int i=0;
	char c1=0,c2=0;
	while(1)
	{
		c1=string1[i];
		c2=string2[i];
		// check for string end
		
		if ((c1 == 0) && (c2 == 0)) return 0;
		if (c1 == 0) return 1;
		if (c2 == 0) return -1;
		
		if ((c1 >= 0x61)&&(c1<=0x7A)) c1-=0x20;
		if ((c2 >= 0x61)&&(c2<=0x7A)) c2-=0x20;
		if (c1>c2)
			return 1;
		else if (c1<c2)
			return -1;
		i++;
	}

}

#ifdef __GIZ__
static BOOL CharToWChar(wchar_t *wc, char *c)
{
	int len=strlen(c);
	int x=0;
	for (x=0;x<len;x++)
	{
		wc[x] = btowc(c[x]);
	}
	wc[len]=0;
	return TRUE;
}
#endif

int FileScan()
{
	int i=0,j=0;
	char text[256];
	DIR *d;
	struct dirent  *de;
	char dirCheck[MAX_PATH+1];
	int dirCount=0;
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
	char ll[1024];
	char *llf;

#ifdef __GIZ__	
	wchar_t  wc[MAX_PATH+1];
	HANDLE hTest;
    WIN32_FIND_DATAW fileInfo;
#endif
 
	PrintTile(currFB);
	PrintTitle(currFB);
	gp_setClipping(0, 0, 319, 239);
	gp_drawString(8,120,25,"Getting Directory Info...",tTextColorItem,framebuffer16[currFB]);
	MenuFlip();
	
	for (i=0;i<MAX_ROMS;i++)
	{
		romList[i].filename[0] = 0;
	}

	//Get rom directory details
	romCount=0;
	
	// Now sort the directory details
	sprintf(romList[0].filename,"Save As Default Directory");
	sprintf(romList[1].filename,"Back To Main Menu");
	sprintf(romList[2].filename,"Parent Directory");
	romList[3].filename[0] = 0;
	romCount=4;

	lastSelected = -1;
	defaultDir = (strcmp(snesRomDir, romDir) == 0);	

	d = opendir(romDir);

	if (d)
	{
		while ((de = readdir(d)))
		{
			if (de->d_name[0] != '.')
			{
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__) || defined(__MIYOO__)
				if (de->d_type == 4) // Directory
#endif
#ifdef __GIZ__
				sprintf(dirCheck,"%s%s%s",romDir,DIR_SEP,de->d_name);
				CharToWChar(wc, dirCheck);
				hTest=FindFirstFileW(wc, &fileInfo);
				if (fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
#endif	
				{
			
					for (i=ROM_SELECTOR_DEFAULT_FOCUS+1;i<=(romCount+1);i++)
					{
						if (romList[i].filename[0] == 0) // string is empty so shove new value in
						{
							strcpy(romList[i].filename,de->d_name);
							romList[i].type=FILE_TYPE_DIRECTORY;//de->d_type;
							break;
						}
						else
						{
							if ((StringCompare(romList[i].filename,de->d_name) > 0) ||
								  (romList[i].type != FILE_TYPE_DIRECTORY))
							{
								// new entry is lower than current string so move all entries up one and insert
								// new value in
								for (j=romCount;j>=i;j--)
								{
									strcpy(romList[j+1].filename,romList[j].filename);
									romList[j+1].type=romList[j].type;
								}
								strcpy(romList[i].filename,de->d_name);
								romList[i].type=FILE_TYPE_DIRECTORY;//de->d_type;
								break;
							}
						}
					}
					dirCount++;
					romCount++;
				}
				else // File
				{
					// only interested in Zip and SMC files
					SplitFilename(de->d_name,_filename,_ext);
					if ((StringCompare(_ext,"zip") == 0) ||
					    (StringCompare(_ext,"smc") == 0) ||
						(StringCompare(_ext,"sfc") == 0))
					{
						for (i=ROM_SELECTOR_DEFAULT_FOCUS+1+dirCount;i<=(romCount+1);i++)
						{
							if (romList[i].filename[0] == 0) // string is empty so shove new value in
							{
								strcpy(romList[i].filename,de->d_name);
								romList[i].type=FILE_TYPE_FILE;//de->d_type;
								break;
							}
							else
							{
								if (StringCompare(romList[i].filename,de->d_name) > 0)
								{
									// new entry is lower than current string so move all entries up one and insert
									// new value in
									for (j=romCount;j>=i;j--)
									{
										strcpy(romList[j+1].filename,romList[j].filename);
										romList[j+1].type=romList[j].type;
									}
									strcpy(romList[i].filename,de->d_name);
									romList[i].type=FILE_TYPE_FILE;//de->d_type;
									break;
								}
							}
						}
						romCount++;
					}
				}

				
				if (romCount > MAX_ROMS)
				{
					PrintTile(currFB);
					PrintTitle(currFB);
					sprintf(text,"Max rom limit exceeded! %d max",MAX_ROMS);
					gp_drawString(8,120,strlen(text),text,tTextColorItem,framebuffer16[currFB]);
					sprintf(text,"Please reduce number of roms");
					gp_drawString(8,130,strlen(text),text,tTextColorItem,framebuffer16[currFB]);
					MenuFlip();
					MenuPause();
					break;
				}
			}
		}
		closedir(d);

		getConfigValue(CONFIG_LASTLOADED, ll, sizeof(ll));
		if (strstr(ll, romDir)) { // where loaded in current dir		
			llf = strrchr(ll, '/');
			if (!llf) {
				ll[0] = '\0';
				llf = ll;
			} else llf++;

			for(i = 4; i < romCount; i++) {
				if ((romList[i].type == FILE_TYPE_FILE) && 
				    (strncmp(romList[i].filename, llf, strlen(romList[i].filename)) == 0)) {
					lastSelected = i;
				}
			}
		}
	}
	else
	{
		PrintTile(currFB);
		PrintTitle(currFB);
		sprintf(text,"Failed to open directory!");
		gp_drawString(8,120,strlen(text),text,tTextColorItem,framebuffer16[currFB]);
		MenuFlip();
		MenuPause();
	}



	return romCount;
}

int FileSelect(int mode)
{
	char text[256];
	int romname_length;
	int action=0;
	int smooth=0;
	unsigned short color=0;
	int i=0;
	int focus=ROM_SELECTOR_DEFAULT_FOCUS;
	int menuExit=0;
	int scanstart=0,scanend=0;
	char dirSep[2]={DIR_SEP};
	char dirSepBad[2]={DIR_SEP_BAD};
	
	//int x = lastSelected;
	int txt_dir; 
	int txt_offset;
	int frame;

#define TIME_TO_SCREENSHOT	(20 * 1)	
#define RESET_SCREENSHOT_COUNTER \
	timeToScreenshot = TIME_TO_SCREENSHOT;\
	if (bmpScreenshot) {\
	 	gDestroyBitmap(bmpScreenshot); \
		bmpScreenshot = NULL; \
		}\
	noScreenshot = 0; 

	int timeToScreenshot;
	int noScreenshot;
	int xScreenshot;
	gBITMAP *bmpScreenshot = NULL;

	FileScan();
#define RESET_NAME_SCROLL \
	txt_dir = 1; \
	txt_offset = -1;

	RESET_NAME_SCROLL
	RESET_SCREENSHOT_COUNTER

	if (lastSelected >= 0) focus = lastSelected;
	if (focus>romCount-1) focus = ROM_SELECTOR_DEFAULT_FOCUS;
	smooth=focus<<8;

	while (menuExit==0)
	{
		InputUpdate(0);

		// Change which rom is focused on:
		if (Inp.repeat[INP_BUTTON_UP])
		{
			focus--; // Up
			RESET_NAME_SCROLL
			RESET_SCREENSHOT_COUNTER
		}
		if (Inp.repeat[INP_BUTTON_DOWN])
		{
			focus++; // Down
			RESET_NAME_SCROLL
			RESET_SCREENSHOT_COUNTER
		}

		if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) {action=0; menuExit=1;}
   
		if (Inp.repeat[INP_BUTTON_LEFT] || Inp.repeat[INP_BUTTON_RIGHT]   )
		{
			if (Inp.repeat[INP_BUTTON_LEFT]) 
			{
				focus-=12;
				smooth=(focus<<8)-1;
			}      
			else if (Inp.repeat[INP_BUTTON_RIGHT])
			{
				focus+=12;
				smooth=(focus<<8)-1;
			}
			
			if (focus>romCount-1) 
			{
				focus=romCount-1;
				smooth=(focus<<8)-1;
			}
			else if (focus<0)
			{
				focus=0;
				smooth=(focus<<8)-1;
			}
			RESET_NAME_SCROLL
			RESET_SCREENSHOT_COUNTER
		}

		if (focus>romCount-1) 
		{
			focus=0;
			smooth=(focus<<8)-1;
			RESET_NAME_SCROLL
			RESET_SCREENSHOT_COUNTER
		}
		else if (focus<0)
		{
			focus=romCount-1;
			smooth=(focus<<8)-1;
			RESET_NAME_SCROLL
			RESET_SCREENSHOT_COUNTER
		}
		//if (defaultDir) lastSelected = focus;
		if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
	    {
			switch(focus)
			{
				case 0: //Save default directory
					SaveMenuOptions(snesOptionsDir, DEFAULT_ROM_DIR_FILENAME, DEFAULT_ROM_DIR_EXT, romDir, strlen(romDir),1);
					strcpy(snesRomDir,romDir);
					break;
					
				case 1: //Return to menu
					action=0;
					menuExit=1;
					break;
					
				case 2: //Goto Parent Directory
					// up a directory
					//Remove a directory from RomPath and rescan
					//Code below will never let you go further up than \SD Card\ on the Gizmondo
					//This is by design.
					for(i=strlen(romDir)-1;i>0;i--) // don't want to change first char in screen
					{
						if((romDir[i] == dirSep[0]) || (romDir[i] == dirSepBad[0]))
						{
							romDir[i] = 0;
							break;
						}
					}
					FileScan();
					RESET_NAME_SCROLL
					RESET_SCREENSHOT_COUNTER
					focus=ROM_SELECTOR_DEFAULT_FOCUS; // default menu to non menu item
													// just to stop directory scan being started 
					smooth=focus<<8;
					memset(&headerDone,0,sizeof(headerDone)); //clear header
					break;
					
				case ROM_SELECTOR_DEFAULT_FOCUS: //blank space - do nothing
					break;
					
				default:
					// normal file or dir selected
					if (romList[focus].type == FILE_TYPE_DIRECTORY)
					{
						//Just check we are not in root dir as this will always have
						//a trailing directory seperater which screws with things
						sprintf(romDir,"%s%s%s",romDir,DIR_SEP,romList[focus].filename);
						FileScan();
						RESET_NAME_SCROLL
						RESET_SCREENSHOT_COUNTER
						focus=ROM_SELECTOR_DEFAULT_FOCUS; // default menu to non menu item
														// just to stop directory scan being started 
						smooth=focus<<8;
					}
					else
					{
						// user has selected a rom, so load it
						sprintf(currentRomFilename,romList[focus].filename);
						quickSavePresent=0;  // reset any quick saves
						action=1;
						menuExit=1;
					}
					break;
			}
	    }

		if (Inp.held[INP_BUTTON_MENU_DELETE]==1)
		{
			if(focus>ROM_SELECTOR_DEFAULT_FOCUS)
			{
				//delete current rom
				if (romList[focus].type != FILE_TYPE_DIRECTORY)
				{
					sprintf(text,"%s",romList[focus].filename);

					if(MenuMessageBox("Are you sure you want to delete",text,"",0)==0)
					{
						deleterom(focus);
						RESET_NAME_SCROLL
						RESET_SCREENSHOT_COUNTER
					}
				}
			}
		}
		#define MAX_WIDTH 	(39 * 8)

		if (timeToScreenshot) timeToScreenshot--;
		else if ((!bmpScreenshot) && (focus > ROM_SELECTOR_DEFAULT_FOCUS) && (!noScreenshot)) {
			char png_fn[1024];
			char *ext;
			gBITMAP *b;
			// Compose focused filename + screenshots dir
			snprintf(png_fn, sizeof(png_fn), "%s/%s", getScreenShotsDir(), romList[focus].filename);
			// set file ext to .png
			ext = strrchr(png_fn, '.');
			if (!ext) ext = &png_fn[strlen(png_fn)];
			strcpy(ext, ".png");
			// load screenshot
			b = load_png(png_fn, NULL);
			if (!b) noScreenshot = 1;
			else {
				unsigned int sh, sw;
				sh = MENU_AREA_Y_END - MENU_AREA_Y_START - 2 + 1;
				sw = (sh * b->w)/ b->h;
				bmpScreenshot = gStretchBitmap(b, sw, sh);
				gDestroyBitmap(b);
				if (!bmpScreenshot) noScreenshot = 1;
				else xScreenshot = SCREEN_WIDTH - 2;
				}
		}

		// Draw screen:
		PrintTile(currFB);
		PrintTitle(currFB);
		sprintf(text,"%s:%s",mode?"Select Rom":"Delete Rom",romDir);
		gp_setClipping(0, 0, 319, 239);
		gp_drawString(6,35,strlen(text)>=40?39:strlen(text),text,tTextColorTitle,framebuffer16[currFB]); 
		//gp_setClipping(8, MENU_AREA_Y_START, 8 + MAX_WIDTH, MENU_AREA_Y_END);
		
		// Draw focused file's screenshot
		if (bmpScreenshot) {
			//unsigned int sh, sw;
			//sh = MENU_AREA_Y_END - MENU_AREA_Y_START - 2 + 1;
			//sw = (sh * bmpScreenshot->w)/ bmpScreenshot->h;
			//gDrawScaledBitmap16(framebuffer16[currFB], xScreenshot, MENU_AREA_Y_START + 1, bmpScreenshot, sw, sh);
			//if (xScreenshot > (SCREEN_WIDTH - sw - 4)) xScreenshot -= 2;
			gDrawBitmap16(framebuffer16[currFB], xScreenshot, MENU_AREA_Y_START + 1, bmpScreenshot, 0, 0, bmpScreenshot->w, bmpScreenshot->h);
			if (xScreenshot > (SCREEN_WIDTH - bmpScreenshot->w - 4)) xScreenshot -= 8;
			}

		smooth=smooth*7+(focus<<8); smooth>>=3;

		scanstart=focus-15;
		if (scanstart<0) scanstart=0;
		scanend = focus+15;
		if (scanend>romCount) scanend=romCount;
		
		for (i=scanstart;i<scanend;i++)
		{
			int x=0,y=0, offset=0;
      
			y=(i<<4)-(smooth>>4);
			x=8;
			y+=112;
			if (y<= (MENU_AREA_Y_START - 8) || y>=232) continue;

			gp_setClipping(8, MENU_AREA_Y_START, 8 + MAX_WIDTH, MENU_AREA_Y_END);
			romname_length=strlen(romList[i].filename);           
			if (i==focus)
			{
				color=tTextColorFocus;
				PrintBar(currFB,y-4);
				if ((romname_length * 8) > MAX_WIDTH) { // let's scroll romname if length is bigger than screen width
					if (frame == 0) txt_offset += txt_dir; // one of each 4 frames
					if (txt_offset == 1) {txt_offset = 0; txt_dir = -1;}
					if ((romname_length * 8 + txt_offset) < MAX_WIDTH) txt_dir = 1;
					offset = txt_offset;
					} 
			}
			else
			{
				color=tTextColorItem;
				if (bmpScreenshot) gp_setClipping(8, MENU_AREA_Y_START, xScreenshot - 1, MENU_AREA_Y_END);
			}

			// Draw Directory icon if current entry is a directory
			if(romList[i].type == FILE_TYPE_DIRECTORY)
			{
				gp_drawString(x-8,y,1,"+",color,framebuffer16[currFB]); 
			}
			
			gp_drawString(x + offset, y, romname_length,romList[i].filename,color,framebuffer16[currFB]); 
		}
		gp_setClipping(0, 0, 319, 239);
		frame = (frame + 1) & 3;
		MenuFlip();
	}
	
	gDestroyBitmap(bmpScreenshot);
	return action;
}

static void ScanSaveStates(char *romname)
{
	FILE *stream;
	int i=0;
	char savename[MAX_PATH+1];
	char filename[MAX_PATH+1];
	char ext[MAX_PATH+1];

	if(!strcmp(romname,saveStateName)) return; // is current save state rom so exit
	
	SplitFilename(romname,filename,ext);

	sprintf(savename,"%s.%s",filename,SAVESTATE_EXT);
  
	for(i=0;i<10;i++)
	{
		/*
		need to build a save state filename
		all saves are held in current working directory (snesSaveStateDir)
		save filename has following format
		shortname(minus file ext) + SV + saveno ( 0 to 9 )
		*/
		sprintf(saveState[i].filename,"%s%d",savename,i);
		sprintf(saveState[i].fullFilename,"%s%s%s",snesSaveStateDir,DIR_SEP,saveState[i].filename);
		stream=(FILE*)fopen(saveState[i].fullFilename,"rb");
		if(stream)
		{
			// we have a savestate
			saveState[i].inUse = 1;
			fclose(stream);	
		}
		else
		{
			// no save state
			saveState[i].inUse = 0;
		}
	}
	strcpy(saveStateName,romname);  // save the last scanned romname
}

void LoadStateFile(char *filename)
{
	S9xUnfreezeGame(filename);
}

static void SaveStateFile(char *filename)
{
	S9xFreezeGame(filename);
	sync();
}

static int SaveStateSelect(int mode)
{
	char text[128];
	int action=11;
	int saveno=0;
	
	if(currentRomFilename[0]==0)
	{
		// no rom loaded
		// display error message and exit
		return(0);
	}
	
	memset(&headerDone,0,sizeof(headerDone));
	ScanSaveStates(currentRomFilename);

	while (action!=0&&action!=100)
	{
		InputUpdate(0);
		if(Inp.held[INP_BUTTON_UP]==1) {saveno--; action=1;}
		if(Inp.held[INP_BUTTON_DOWN]==1) {saveno++; action=1;}
		if(saveno<-1) saveno=9;
		if(saveno>9) saveno=-1;
	      
		if(Inp.held[INP_BUTTON_MENU_CANCEL]==1) action=0; // exit
		else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(saveno==-1)) action=0; // exit
		else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==0)&&((action==2)||(action==5))) action=6;  // pre-save mode
		else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==1)&&(action==5)) action=8;  // pre-load mode
		else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==2)&&(action==5))
		{
			if(MenuMessageBox("Are you sure you want to delete","this save?","",0)==0) action=13;  //delete slot with no preview
		}
		//else if((Inp.held[INP_BUTTON_R]==1)&&(action==12)) action=3;  // preview slot mode
		else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==1)&&(action==12)) action=8;  //load slot with no preview
		else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==0)&&(action==12)) action=6;  //save slot with no preview
		else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==2)&&(action==12))
		{
			if(MenuMessageBox("Are you sure you want to delete","this save?","",0)==0) action=13;  //delete slot with no preview
		}

		PrintTile(currFB);
		PrintTitle(currFB);
		if(mode==SAVESTATE_MODE_SAVE) gp_drawString(6,35,10,"Save State",tTextColorTitle,framebuffer16[currFB]); 
		if(mode==SAVESTATE_MODE_LOAD) gp_drawString(6,35,10,"Load State",tTextColorTitle,framebuffer16[currFB]); 
		if(mode==SAVESTATE_MODE_DELETE) gp_drawString(6,35,12,"Delete State",tTextColorTitle,framebuffer16[currFB]); 
		sprintf(text,"Press UP and DOWN to change save slot");
		gp_drawString(12,230,strlen(text),text,(unsigned short)MENU_RGB(31,15,5),framebuffer16[currFB]);
      
		if(saveno==-1) 
		{
			if(action!=10&&action!=0) 
			{
				action=10;
			}
		}
		else
		{
			PrintBar(currFB,60-4);
			sprintf(text,"SLOT %d",saveno);
			gp_drawString(136,60,strlen(text),text,tTextColorItem,framebuffer16[currFB]);
		}
      
		switch(action)
		{
			case 1:
				//gp_drawString(112,145,14,"Checking....",tTextColorItem,framebuffer16[currFB]);
				break;
			case 2:
				gp_drawString(144,145,4,"FREE",tTextColorItem,framebuffer16[currFB]);
				break;
			case 3:
				gp_drawString(104,145,14,"Previewing....",tTextColorItem,framebuffer16[currFB]);
				break;
			case 4:
				gp_drawString(88,145,18,"Previewing....fail",tTextColorItem,framebuffer16[currFB]);
				break;
			case 5: 
				gp_drawString(112,145,17, "Not gonna happen!",tTextColorItem,framebuffer16[currFB]);
				if(mode==1) gp_drawString((320-(strlen(MENU_TEXT_LOAD_SAVESTATE)<<3))>>1,210,strlen(MENU_TEXT_LOAD_SAVESTATE), MENU_TEXT_LOAD_SAVESTATE,tTextColorItem,framebuffer16[currFB]);
				else if(mode==0) gp_drawString((320-(strlen(MENU_TEXT_OVERWRITE_SAVESTATE)<<3))>>1,210,strlen(MENU_TEXT_OVERWRITE_SAVESTATE), MENU_TEXT_OVERWRITE_SAVESTATE,tTextColorItem,framebuffer16[currFB]);
				else if(mode==2) gp_drawString((320-(strlen(MENU_TEXT_DELETE_SAVESTATE)<<3))>>1,210,strlen(MENU_TEXT_DELETE_SAVESTATE), MENU_TEXT_DELETE_SAVESTATE,tTextColorItem,framebuffer16[currFB]);
				break;
			case 6:
				gp_drawString(124,145,9,"Saving...",tTextColorItem,framebuffer16[currFB]);
				break;
			case 7:
				gp_drawString(124,145,14,"Saving...Fail!",tTextColorItem,framebuffer16[currFB]);
				break;
			case 8:
				gp_drawString(116,145,11,"loading....",tTextColorItem,framebuffer16[currFB]);
				break;
				case 9:
				gp_drawString(116,145,15,"loading....Fail",tTextColorItem,framebuffer16[currFB]);
				break;
			case 10:	

				PrintBar(currFB,145-4);
				gp_drawString(104,145,14,"Return To Menu",tTextColorItem,framebuffer16[currFB]);
				break;
			case 12:
				gp_drawString(124,145,9,"Slot used",tTextColorItem,framebuffer16[currFB]);
				//gp_drawString((320-(strlen(MENU_TEXT_PREVIEW_SAVESTATE)<<3))>>1,165,strlen(MENU_TEXT_PREVIEW_SAVESTATE),MENU_TEXT_PREVIEW_SAVESTATE,tTextColorItem,framebuffer16[currFB]);
				if(mode==1) gp_drawString((320-(strlen(MENU_TEXT_LOAD_SAVESTATE)<<3))>>1,175,strlen(MENU_TEXT_LOAD_SAVESTATE), MENU_TEXT_LOAD_SAVESTATE,tTextColorItem,framebuffer16[currFB]);
				else if(mode==0) gp_drawString((320-(strlen(MENU_TEXT_OVERWRITE_SAVESTATE)<<3))>>1,175,strlen(MENU_TEXT_OVERWRITE_SAVESTATE), MENU_TEXT_OVERWRITE_SAVESTATE,tTextColorItem,framebuffer16[currFB]);
				else if(mode==2) gp_drawString((320-(strlen(MENU_TEXT_DELETE_SAVESTATE)<<3))>>1,175,strlen(MENU_TEXT_DELETE_SAVESTATE), MENU_TEXT_DELETE_SAVESTATE,tTextColorItem,framebuffer16[currFB]);
				break;
			case 13:
				gp_drawString(116,145,11,"Deleting....",tTextColorItem,framebuffer16[currFB]);
				break;
		}
      
		MenuFlip();
      
		switch(action)
		{
			case 1:
				if(saveState[saveno].inUse) 
				{
					action=12;
				}
				else 
				{
					action=2;
				}
				break;
			case 3:
				gp_setCpuspeed(MENU_FAST_CPU_SPEED);
				LoadStateFile(saveState[saveno].fullFilename);
				gp_setCpuspeed(MENU_CPU_SPEED);
				action=5;
				break;
			case 6:
				gp_setCpuspeed(MENU_FAST_CPU_SPEED);
				SaveStateFile(saveState[saveno].fullFilename);
				gp_setCpuspeed(MENU_CPU_SPEED);
				saveState[saveno].inUse=1;
				action=1;
				break;
			case 7:
				action=1;
				break;
			case 8:
				LoadStateFile(saveState[saveno].fullFilename);
				action=100;  // loaded ok so exit
				break;
			case 9:
				action=1;
				break;
			case 11:
				action=1;
				break;
			case 13:
				remove(saveState[saveno].fullFilename);
				sync();
				saveState[saveno].inUse = 0;
				action=1;
				break;
		}
	}
	memset(&headerDone,0,sizeof(headerDone));
	return(action);
}

static
void RenderMenu(char *menuName, int menuCount, int menuSmooth, int menufocus)
{
	int i=0;
	char text[50];
	unsigned short color=0;
	PrintTile(currFB);
	PrintTitle(currFB);

	gp_setClipping(0, 0, 319, 239);
	gp_drawString(6,35,strlen(menuName),menuName,tTextColorTitle,framebuffer16[currFB]); 
	gp_setClipping(8, MENU_AREA_Y_START, SCREEN_WIDTH - 8, MENU_AREA_Y_END);
    
    // RRRRRGGGGGBBBBBI  gp32 color format
    for (i=0;i<menuCount;i++)
    {
      int x=0,y=0;

      y=(i<<4)-(menuSmooth>>4);
      x=8;
      y+=112;

      if (y<= (MENU_AREA_Y_START - 8) || y>=232) continue;
      
      if (i==menufocus)
      {
        color=tTextColorFocus;
	PrintBar(currFB,y-4);
      }
      else
      {
        color=tTextColorItem;
      }

      sprintf(text,"%s",menutext[i]);
      gp_drawString(x,y,strlen(text),text,color,framebuffer16[currFB]);
    }
    gp_setClipping(0, 0, 319, 239);
}

static
int LoadRomMenu(void)
{
	int menuExit=0,menuCount=LOAD_ROM_MENU_COUNT,menufocus=0,menuSmooth=0;
	int action=0;
	int subaction=0;

	memset(&headerDone,0,sizeof(headerDone));
	strcpy(romDir,snesRomDir);
	subaction=FileSelect(0);
	memset(&headerDone,0,sizeof(headerDone));
	if(subaction)
	{
		action=EVENT_LOAD_SNES_ROM;
		menuExit=1;
	}

	return action;
}

static
int SaveStateMenu(void)
{
	int menuExit=0,menuCount=SAVESTATE_MENU_COUNT,menufocus=0,menuSmooth=0;
	int action=0;
	int subaction=0;

	memset(&headerDone,0,sizeof(headerDone));
  
	//Update
	sprintf(menutext[SAVESTATE_MENU_LOAD],"Load State");
	sprintf(menutext[SAVESTATE_MENU_SAVE],"Save State");
	sprintf(menutext[SAVESTATE_MENU_DELETE],"Delete State");
	sprintf(menutext[SAVESTATE_MENU_RETURN],"Back");
	
	while (!menuExit)
	{
		InputUpdate(0);

		// Change which rom is focused on:
		if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
		if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
    
		if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) menuExit=1;
    
		if (menufocus>menuCount-1)
		{
			menufocus=0;
			menuSmooth=(menufocus<<8)-1;
		}   
		else if (menufocus<0) 
		{
			menufocus=menuCount-1;
			menuSmooth=(menufocus<<8)-1;
		}

		if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
		{
			switch(menufocus)
			{
				case SAVESTATE_MENU_LOAD:
					subaction=SaveStateSelect(SAVESTATE_MODE_LOAD);
					if(subaction==100)
					{
						menuExit=1;
						action=100;
					}
					break;
				case SAVESTATE_MENU_SAVE:
					SaveStateSelect(SAVESTATE_MODE_SAVE);
					break;
				case SAVESTATE_MENU_DELETE:
					SaveStateSelect(SAVESTATE_MODE_DELETE);
					break;
				case SAVESTATE_MENU_RETURN:
					menuExit=1;
					break;
			}	
		}
		// Draw screen:
		menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
		RenderMenu("Save States", menuCount,menuSmooth,menufocus);
		MenuFlip();
       
	}
  
  return action;
}

static
int SramMenu(void)
{
	int menuExit=0,menuCount=SRAM_MENU_COUNT,menufocus=0,menuSmooth=0;
	int action=0;
	int subaction=0;
	char *srammem=NULL;
	

	memset(&headerDone,0,sizeof(headerDone));
  
	//Update
	sprintf(menutext[SRAM_MENU_LOAD],"Load SRAM");
	sprintf(menutext[SRAM_MENU_SAVE],"Save SRAM");
	sprintf(menutext[SRAM_MENU_DELETE],"Delete SRAM");
	sprintf(menutext[SRAM_MENU_RETURN],"Back");
	
	while (!menuExit)
	{
		InputUpdate(0);

		// Change which rom is focused on:
		if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
		if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
    
		if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) menuExit=1;
    
		if (menufocus>menuCount-1)
		{
			menufocus=0;
			menuSmooth=(menufocus<<8)-1;
		}   
		else if (menufocus<0) 
		{
			menufocus=menuCount-1;
			menuSmooth=(menufocus<<8)-1;
		}

		if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
		{
			switch(menufocus)
			{
				case SRAM_MENU_LOAD:
					//LoadSram(snesSramDir,currentRomFilename,SRAM_FILE_EXT,(char*)&sram);
					break;
				case SRAM_MENU_SAVE:
					//SaveSram(snesSramDir,currentRomFilename,SRAM_FILE_EXT,(char*)&sram);
					break;
				case SRAM_MENU_DELETE:
					//DeleteSram(snesSramDir,currentRomFilename,SRAM_FILE_EXT);
					break;
				case SRAM_MENU_RETURN:
					menuExit=1;
					break;
			}	
		}
		// Draw screen:
		menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
		RenderMenu("SRAM", menuCount,menuSmooth,menufocus);
		MenuFlip();
       
	}
  
  return action;
}

static 
void SNESOptionsUpdateText(int menu_index)
{
	switch(menu_index)
	{
		case SNES_MENU_SOUND:
		switch(snesMenuOptions.soundOn)
		{
			case 0:
				sprintf(menutext[SNES_MENU_SOUND],"Sound: OFF");
				break;
			case 1:
				sprintf(menutext[SNES_MENU_SOUND],"Sound: ON");
				break;
		}
		break;
		
		case SNES_MENU_SOUND_RATE:
			if (snesMenuOptions.stereo)		
				sprintf(menutext[SNES_MENU_SOUND_RATE],"Sound Rate: %d stereo",(unsigned int)soundRates[snesMenuOptions.soundRate]);
			else
				sprintf(menutext[SNES_MENU_SOUND_RATE],"Sound Rate: %d mono",(unsigned int)soundRates[snesMenuOptions.soundRate]);
			break;
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__)
		case SNES_MENU_CPUSPEED:
			sprintf(menutext[SNES_MENU_CPUSPEED],"Cpu Speed: %d",(unsigned int)cpuSpeedLookup[snesMenuOptions.cpuSpeed]);
			break;
#endif			
		case SNES_MENU_SOUND_VOL:
			sprintf(menutext[SNES_MENU_SOUND_VOL],"Volume: %d",snesMenuOptions.volume);
			break;
		
		case SNES_MENU_FRAMESKIP:
			switch(snesMenuOptions.frameSkip)
			{
				case 0:
					sprintf(menutext[SNES_MENU_FRAMESKIP],"Frameskip: AUTO");
					break;
				default:
					sprintf(menutext[SNES_MENU_FRAMESKIP],"Frameskip: %d",snesMenuOptions.frameSkip-1);
					break;
			}
			break;
			
		case SNES_MENU_REGION:
			switch(snesMenuOptions.region)
			{
				case 0:
					sprintf(menutext[SNES_MENU_REGION],"Region: AUTO");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_REGION],"Region: NTSC");
					break;
				case 2:
					sprintf(menutext[SNES_MENU_REGION],"Region: PAL");
					break;
			}
			break;

		case SNES_MENU_FPS:
			switch(snesMenuOptions.showFps)
			{
				case 0:
					sprintf(menutext[SNES_MENU_FPS],"Show FPS: OFF");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_FPS],"Show FPS: ON");
					break;
			}
			break;
#if defined(__GP2X__)			
		case SNES_MENU_GAMMA:
			sprintf(menutext[SNES_MENU_GAMMA],"Brightness: %d",snesMenuOptions.gamma+100);
			break;
#endif		
		case SNES_MENU_TRANSPARENCY:
			switch(snesMenuOptions.transparency)
			{
				case 0:
					sprintf(menutext[SNES_MENU_TRANSPARENCY],"Transparencies: OFF");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_TRANSPARENCY],"Transparencies: ON");
					break;
			}
			break;
			
		case SNES_MENU_LOAD_GLOBAL:
			sprintf(menutext[SNES_MENU_LOAD_GLOBAL],"Load Global Settings");
			break;
			
		case SNES_MENU_SAVE_GLOBAL:
			sprintf(menutext[SNES_MENU_SAVE_GLOBAL],"Save Global Settings");
			break;
			
		case SNES_MENU_DELETE_GLOBAL:
			sprintf(menutext[SNES_MENU_DELETE_GLOBAL],"Delete Global Settings");
			break;
			
		case SNES_MENU_LOAD_CURRENT:
			sprintf(menutext[SNES_MENU_LOAD_CURRENT],"Load Settings For Current Game");
			break;
		
		case SNES_MENU_SAVE_CURRENT:
			sprintf(menutext[SNES_MENU_SAVE_CURRENT],"Save Settings For Current Game");
			break;
			
		case SNES_MENU_DELETE_CURRENT:
			sprintf(menutext[SNES_MENU_DELETE_CURRENT],"Delete Settings For Current Game");
			break;
			
		case SNES_MENU_SET_ROMDIR:
			sprintf(menutext[SNES_MENU_SET_ROMDIR],"Save Current Rom Directory");
			break;
		
		case SNES_MENU_CLEAR_ROMDIR:
			sprintf(menutext[SNES_MENU_CLEAR_ROMDIR],"Reset Default Rom Directory");
			break;
			
		case SNES_MENU_RETURN:
			sprintf(menutext[SNES_MENU_RETURN],"Back");
			break;
		case SNES_MENU_RENDER_MODE:
			switch(snesMenuOptions.renderMode)
			{
				case RENDER_MODE_UNSCALED:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Unscaled");
					break;
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__)
				case RENDER_MODE_SCALED:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Fullscreen");
					break;
				case RENDER_MODE_HORIZONTAL_SCALED:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Horizontal");
					break;
#elif defined(__MIYOO__)
				case RENDER_MODE_SCALED:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Fullscreen (smooth)");
					break;
				case RENDER_MODE_HORIZONTAL_SCALED:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Horizontal (smooth)");
					break;
				case RENDER_MODE_SCALED_FAST:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Fullscreen");
					break;
				case RENDER_MODE_HORIZONTAL_SCALED_FAST:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Horizontal ");
					break;
#endif
				default:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Scaling: Unscaled");
					break;
			}
			break;
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__) || defined(__MIYOO__)
		case SNES_MENU_ACTION_BUTTONS:
			switch(snesMenuOptions.actionButtons)
			{
				case 0:
					sprintf(menutext[SNES_MENU_ACTION_BUTTONS],"Action Buttons: Normal");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_ACTION_BUTTONS],"Action Buttons: Swapped");
					break;
			}
			break;
#endif
		case SNES_MENU_AUTO_SAVE_SRAM:
			switch(snesMenuOptions.autoSram)
			{
				case 0:
					sprintf(menutext[SNES_MENU_AUTO_SAVE_SRAM],"Saving SRAM: Manual");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_AUTO_SAVE_SRAM],"Saving SRAM: Automatic");
					break;
				case 2:
					sprintf(menutext[SNES_MENU_AUTO_SAVE_SRAM],"Saving SRAM: Manual + indicator");
					break;
			}
			break;
		case SNES_MENU_EMULATION_TYPE:
			switch(snesMenuOptions.asmspc700)
			{
				case 0:
					sprintf(menutext[SNES_MENU_EMULATION_TYPE],"Emulation (Reset Required): S9xAPU");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_EMULATION_TYPE],"Emulation (Reset Required): SPC700");
					break;
			}
			break;
		case SNES_MENU_LOAD_ROM_ON_INIT:
			switch(snesMenuOptions.loadOnInit)
			{
				case 0:
					sprintf(menutext[SNES_MENU_LOAD_ROM_ON_INIT],"Load last played ROM at startup: OFF");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_LOAD_ROM_ON_INIT],"Load last played ROM at startup: ON");
					break;
			}
			break;
		case SNES_MENU_ADVANCED_HACKS:
			sprintf(menutext[SNES_MENU_ADVANCED_HACKS],"Advanced hacks");
			break;
	}
}

static 
void SNESHacksUpdateText(int menu_index)
{
	switch(menu_index)
	{
		case HACKS_MENU_SNESADVANCE_DAT:
			if (snesMenuOptions.SpeedHacks)
				sprintf(menutext[HACKS_MENU_SNESADVANCE_DAT],"Apply snesadvance.dat on ROM load: ON");
			else
				sprintf(menutext[HACKS_MENU_SNESADVANCE_DAT],"Apply snesadvance.dat on ROM load: OFF");
			break;
		case HACKS_MENU_AUDIO:
			if(snesMenuOptions.soundHack)
				sprintf(menutext[HACKS_MENU_AUDIO],"Audio Performance hack: ON");
			else
				sprintf(menutext[HACKS_MENU_AUDIO],"Audio Performance hack: OFF");
		break;
#ifdef __OLD_RASTER_FX__
		case HACKS_MENU_DELAYED_RASTER_FX:
			sprintf(menutext[HACKS_MENU_DELAYED_RASTER_FX], "Delayed Raster FX: %s", snesMenuOptions.delayedRasterFX ? "ON" : "OFF");
			break;
#endif
		case HACKS_MENU_PALETTE:
			if(snesMenuOptions.graphHacks & PPU_IGNORE_PALWRITE)
				sprintf(menutext[HACKS_MENU_PALETTE],"Ignore Palette writes: ON");
			else
				sprintf(menutext[HACKS_MENU_PALETTE],"Ignore Palette writes: OFF");
		break;
		case HACKS_MENU_FIXEDCOL:
			if(snesMenuOptions.graphHacks & PPU_IGNORE_FIXEDCOLCHANGES)
				sprintf(menutext[HACKS_MENU_FIXEDCOL],"Ignore Fixed Colour: ON");
			else
				sprintf(menutext[HACKS_MENU_FIXEDCOL],"Ignore Fixed Colour: OFF");
		break;
		case HACKS_MENU_WINDOW:
			if(snesMenuOptions.graphHacks & PPU_IGNORE_WINDOW)
				sprintf(menutext[HACKS_MENU_WINDOW],"Ignore Windows clipping: ON");
			else
				sprintf(menutext[HACKS_MENU_WINDOW],"Ignore Windows clipping: OFF");
		break;
		case HACKS_MENU_ADDSUB:
			if(snesMenuOptions.graphHacks & PPU_IGNORE_ADDSUB)
				sprintf(menutext[HACKS_MENU_ADDSUB],"Ignore Add/Sub modes: ON");
			else
				sprintf(menutext[HACKS_MENU_ADDSUB],"Ignore Add/Sub modes: OFF");
		break;
		case HACKS_MENU_OBJ:
			if(snesMenuOptions.graphHacks & GFX_IGNORE_OBJ)
				sprintf(menutext[HACKS_MENU_OBJ],"Ignore objects layer: ON");
			else
				sprintf(menutext[HACKS_MENU_OBJ],"Ignore objects layer: OFF");
		break;
		case HACKS_MENU_BG0:
			if(snesMenuOptions.graphHacks & GFX_IGNORE_BG0)
				sprintf(menutext[HACKS_MENU_BG0],"Ignore background layer 0: ON");
			else
				sprintf(menutext[HACKS_MENU_BG0],"Ignore background layer 0: OFF");
		break;
		case HACKS_MENU_BG1:
			if(snesMenuOptions.graphHacks & GFX_IGNORE_BG1)
				sprintf(menutext[HACKS_MENU_BG1],"Ignore background layer 1: ON");
			else
				sprintf(menutext[HACKS_MENU_BG1],"Ignore background layer 1: OFF");
		break;
		case HACKS_MENU_BG2:
			if(snesMenuOptions.graphHacks & GFX_IGNORE_BG2)
				sprintf(menutext[HACKS_MENU_BG2],"Ignore background layer 2: ON");
			else
				sprintf(menutext[HACKS_MENU_BG2],"Ignore background layer 2: OFF");
		break;
		case HACKS_MENU_BG3:
			if(snesMenuOptions.graphHacks & GFX_IGNORE_BG3)
				sprintf(menutext[HACKS_MENU_BG3],"Ignore background layer 3: ON");
			else
				sprintf(menutext[HACKS_MENU_BG3],"Ignore background layer 3: OFF");
		break;
		case HACKS_MENU_RETURN:
			sprintf(menutext[HACKS_MENU_RETURN],"Back");
			break;


	}
}

static
void SNESHacksUpdateText_All()
{
	SNESHacksUpdateText(HACKS_MENU_AUDIO);
	SNESHacksUpdateText(HACKS_MENU_SNESADVANCE_DAT);
#ifdef __OLD_RASTER_FX__
	SNESHacksUpdateText(HACKS_MENU_DELAYED_RASTER_FX);
#endif
	SNESHacksUpdateText(HACKS_MENU_PALETTE);
	SNESHacksUpdateText(HACKS_MENU_FIXEDCOL);
	SNESHacksUpdateText(HACKS_MENU_WINDOW);
	SNESHacksUpdateText(HACKS_MENU_ADDSUB);
	SNESHacksUpdateText(HACKS_MENU_OBJ);
	SNESHacksUpdateText(HACKS_MENU_BG0);
	SNESHacksUpdateText(HACKS_MENU_BG1);
	SNESHacksUpdateText(HACKS_MENU_BG2);
	SNESHacksUpdateText(HACKS_MENU_BG3);
	SNESHacksUpdateText(HACKS_MENU_RETURN);
}

static
int SNESHacksMenu(void)
{
	int menuExit=0,menuCount=HACKS_MENU_COUNT,menufocus=0,menuSmooth=0;
	int action=0;
	int subaction=0;

	memset(&headerDone,0,sizeof(headerDone));
  
	//Update all items
	SNESHacksUpdateText_All();
	
	while (!menuExit)
	{
		InputUpdate(0);

		// Change which rom is focused on:
		if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
		if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
    
		if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) menuExit=1;
    
		if (menufocus>menuCount-1)
		{
			menufocus=0;
			menuSmooth=(menufocus<<8)-1;
		}   
		else if (menufocus<0) 
		{
			menufocus=menuCount-1;
			menuSmooth=(menufocus<<8)-1;

		}

		if (Inp.held[INP_BUTTON_LEFT]==1||
			  Inp.held[INP_BUTTON_RIGHT]==1||
			  Inp.repeat[INP_BUTTON_LEFT]||
			  Inp.repeat[INP_BUTTON_RIGHT])
		{
			switch(menufocus)
			{
				case HACKS_MENU_SNESADVANCE_DAT:
					snesMenuOptions.SpeedHacks^=1;
					SNESHacksUpdateText(HACKS_MENU_SNESADVANCE_DAT);
					break;
				case HACKS_MENU_AUDIO:
					snesMenuOptions.soundHack^=1;
					SNESHacksUpdateText(HACKS_MENU_AUDIO);
					break;
#ifdef __OLD_RASTER_FX__
				case HACKS_MENU_DELAYED_RASTER_FX:
					snesMenuOptions.delayedRasterFX ^= 1;
					SNESHacksUpdateText(HACKS_MENU_DELAYED_RASTER_FX);
					break;
#endif
				case HACKS_MENU_PALETTE:
					if (snesMenuOptions.graphHacks & PPU_IGNORE_PALWRITE)
						snesMenuOptions.graphHacks &= ~PPU_IGNORE_PALWRITE;
					else
						snesMenuOptions.graphHacks |= PPU_IGNORE_PALWRITE;					
					SNESHacksUpdateText(HACKS_MENU_PALETTE);
					break;
				case HACKS_MENU_FIXEDCOL:
					if (snesMenuOptions.graphHacks & PPU_IGNORE_FIXEDCOLCHANGES)
						snesMenuOptions.graphHacks &= ~PPU_IGNORE_FIXEDCOLCHANGES;
					else
						snesMenuOptions.graphHacks |= PPU_IGNORE_FIXEDCOLCHANGES;					
					SNESHacksUpdateText(HACKS_MENU_FIXEDCOL);
					break;
				case HACKS_MENU_WINDOW:
					if (snesMenuOptions.graphHacks & PPU_IGNORE_WINDOW)
						snesMenuOptions.graphHacks &= ~PPU_IGNORE_WINDOW;
					else
						snesMenuOptions.graphHacks |= PPU_IGNORE_WINDOW;					
					SNESHacksUpdateText(HACKS_MENU_WINDOW);
					break;
				case HACKS_MENU_ADDSUB:
					if (snesMenuOptions.graphHacks & PPU_IGNORE_ADDSUB)
						snesMenuOptions.graphHacks &= ~PPU_IGNORE_ADDSUB;
					else
						snesMenuOptions.graphHacks |= PPU_IGNORE_ADDSUB;					
					SNESHacksUpdateText(HACKS_MENU_ADDSUB);

					break;
				case HACKS_MENU_OBJ:
					if (snesMenuOptions.graphHacks & GFX_IGNORE_OBJ)
						snesMenuOptions.graphHacks &= ~GFX_IGNORE_OBJ;
					else
						snesMenuOptions.graphHacks |= GFX_IGNORE_OBJ;					
					SNESHacksUpdateText(HACKS_MENU_OBJ);
					break;
				case HACKS_MENU_BG0:
					if (snesMenuOptions.graphHacks & GFX_IGNORE_BG0)
						snesMenuOptions.graphHacks &= ~GFX_IGNORE_BG0;
					else
						snesMenuOptions.graphHacks |= GFX_IGNORE_BG0;					
					SNESHacksUpdateText(HACKS_MENU_BG0);
					break;
				case HACKS_MENU_BG1:
					if (snesMenuOptions.graphHacks & GFX_IGNORE_BG1)
						snesMenuOptions.graphHacks &= ~GFX_IGNORE_BG1;
					else
						snesMenuOptions.graphHacks |= GFX_IGNORE_BG1;					
					SNESHacksUpdateText(HACKS_MENU_BG1);
					break;
				case HACKS_MENU_BG2:
					if (snesMenuOptions.graphHacks & GFX_IGNORE_BG2)
						snesMenuOptions.graphHacks &= ~GFX_IGNORE_BG2;
					else
						snesMenuOptions.graphHacks |= GFX_IGNORE_BG2;					
					SNESHacksUpdateText(HACKS_MENU_BG2);
					break;
				case HACKS_MENU_BG3:
					if (snesMenuOptions.graphHacks & GFX_IGNORE_BG3)
						snesMenuOptions.graphHacks &= ~GFX_IGNORE_BG3;
					else
						snesMenuOptions.graphHacks |= GFX_IGNORE_BG3;					
					SNESHacksUpdateText(HACKS_MENU_BG3);
					break;
			}
		}
		if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
		{
			switch(menufocus)
			{
				case HACKS_MENU_RETURN:
					menuExit=1;
					break;
			}	
		}
		// Draw screen:
		menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
		RenderMenu("SNES Advanced Hacks", menuCount,menuSmooth,menufocus);
		MenuFlip();
       
	}
  
  return action;
}

static
void SNESOptionsUpdateText_All()
{
	SNESOptionsUpdateText(SNES_MENU_SOUND);
	SNESOptionsUpdateText(SNES_MENU_SOUND_RATE);
	SNESOptionsUpdateText(SNES_MENU_SOUND_VOL);
	SNESOptionsUpdateText(SNES_MENU_FRAMESKIP);
	SNESOptionsUpdateText(SNES_MENU_REGION);
	SNESOptionsUpdateText(SNES_MENU_FPS);
	SNESOptionsUpdateText(SNES_MENU_TRANSPARENCY);
	SNESOptionsUpdateText(SNES_MENU_LOAD_GLOBAL);
	SNESOptionsUpdateText(SNES_MENU_SAVE_GLOBAL);
	SNESOptionsUpdateText(SNES_MENU_DELETE_GLOBAL);
	SNESOptionsUpdateText(SNES_MENU_LOAD_CURRENT);
	SNESOptionsUpdateText(SNES_MENU_SAVE_CURRENT);
	SNESOptionsUpdateText(SNES_MENU_DELETE_CURRENT);
	SNESOptionsUpdateText(SNES_MENU_SET_ROMDIR);
	SNESOptionsUpdateText(SNES_MENU_CLEAR_ROMDIR);
	SNESOptionsUpdateText(SNES_MENU_RETURN);
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__)
	SNESOptionsUpdateText(SNES_MENU_RENDER_MODE);
	SNESOptionsUpdateText(SNES_MENU_CPUSPEED);
	SNESOptionsUpdateText(SNES_MENU_ACTION_BUTTONS);
#endif
#if defined(__MIYOO__)
	SNESOptionsUpdateText(SNES_MENU_RENDER_MODE);
	SNESOptionsUpdateText(SNES_MENU_ACTION_BUTTONS);
#endif
#if defined(__GP2X__)
	SNESOptionsUpdateText(SNES_MENU_GAMMA);
#endif
	SNESOptionsUpdateText(SNES_MENU_EMULATION_TYPE);
	SNESOptionsUpdateText(SNES_MENU_AUTO_SAVE_SRAM);
	SNESOptionsUpdateText(SNES_MENU_LOAD_ROM_ON_INIT);
	SNESOptionsUpdateText(SNES_MENU_ADVANCED_HACKS);
}

static
int SNESOptionsMenu(void)
{
	int menuExit=0,menuCount=SNES_MENU_COUNT,menufocus=0,menuSmooth=0;
	int action=0;
	int subaction=0;

	memset(&headerDone,0,sizeof(headerDone));
  
	//Update all items
	SNESOptionsUpdateText_All();
	
	while (!menuExit)
	{
		InputUpdate(0);

		// Change which rom is focused on:
		if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
		if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
    
		if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) menuExit=1;
    
		if (menufocus>menuCount-1)
		{
			menufocus=0;
			menuSmooth=(menufocus<<8)-1;
		}   
		else if (menufocus<0) 
		{
			menufocus=menuCount-1;
			menuSmooth=(menufocus<<8)-1;
		}

		if (Inp.held[INP_BUTTON_LEFT]==1||
			  Inp.held[INP_BUTTON_RIGHT]==1||
			  Inp.repeat[INP_BUTTON_LEFT]||
			  Inp.repeat[INP_BUTTON_RIGHT])
		{
			switch(menufocus)
			{
				case SNES_MENU_SOUND:
					snesMenuOptions.soundOn^=1;
					SNESOptionsUpdateText(SNES_MENU_SOUND);
					break;
				case SNES_MENU_SOUND_RATE:
					if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
					{
						if (!snesMenuOptions.stereo)
							snesMenuOptions.stereo = 1;
						else
						{
						snesMenuOptions.soundRate++;
							snesMenuOptions.stereo = 0;
						}
#if defined(__WIZ__)
						if(snesMenuOptions.soundRate>2) snesMenuOptions.soundRate=0;
#else
						if(snesMenuOptions.soundRate>4) snesMenuOptions.soundRate=0;
#endif
					}
					else
					{
						if (snesMenuOptions.stereo)
							snesMenuOptions.stereo = 0;
						else
						{
							snesMenuOptions.soundRate--;
							snesMenuOptions.stereo = 1;
						}
#if defined(__WIZ__)
						if(snesMenuOptions.soundRate>2) snesMenuOptions.soundRate=2;
#else
						if(snesMenuOptions.soundRate>4) snesMenuOptions.soundRate=4;
#endif
					}
					SNESOptionsUpdateText(SNES_MENU_SOUND_RATE);
					break;
				case SNES_MENU_SOUND_VOL:
					if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
					{
						snesMenuOptions.volume+=1;
						if(snesMenuOptions.volume>100) snesMenuOptions.volume=0;
					}
					else
					{
						snesMenuOptions.volume-=1;
						if(snesMenuOptions.volume>100) snesMenuOptions.volume=100;
					}
					SNESOptionsUpdateText(SNES_MENU_SOUND_VOL);
					break;
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__)
				case SNES_MENU_CPUSPEED:
					if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
					{
						snesMenuOptions.cpuSpeed++;
						if(snesMenuOptions.cpuSpeed>45) snesMenuOptions.cpuSpeed=0;
					}
					else
					{
						snesMenuOptions.cpuSpeed--;
						if(snesMenuOptions.cpuSpeed>45) snesMenuOptions.cpuSpeed=45;
					}
					SNESOptionsUpdateText(SNES_MENU_CPUSPEED);
					break;
#endif
				case SNES_MENU_FRAMESKIP:
					if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
					{
						snesMenuOptions.frameSkip++;
						if(snesMenuOptions.frameSkip>6) snesMenuOptions.frameSkip=0;
					}
					else
					{
						snesMenuOptions.frameSkip--;
						if(snesMenuOptions.frameSkip>6) snesMenuOptions.frameSkip=6;
					}
					SNESOptionsUpdateText(SNES_MENU_FRAMESKIP);
					break;
				case SNES_MENU_REGION:
					if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
					{
						snesMenuOptions.region++;
						if(snesMenuOptions.region>2) snesMenuOptions.region=0;
					}
					else
					{
						snesMenuOptions.region--;
						if(snesMenuOptions.region>2) snesMenuOptions.region=2;
					}
					SNESOptionsUpdateText(SNES_MENU_REGION);
					break;
				case SNES_MENU_FPS:
					snesMenuOptions.showFps^=1;
					SNESOptionsUpdateText(SNES_MENU_FPS);
					break;
#if defined(__GP2X__)
				case SNES_MENU_GAMMA:
					if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
					{
						snesMenuOptions.gamma++;
						if(snesMenuOptions.gamma>100) snesMenuOptions.gamma=100;
					}
					else
					{
						snesMenuOptions.gamma--;
						if(snesMenuOptions.gamma<-100) snesMenuOptions.gamma=-100;
					}
					set_gamma(snesMenuOptions.gamma+100);
					SNESOptionsUpdateText(SNES_MENU_GAMMA);
					break;
#endif
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__) || defined(__MIYOO__)
				case SNES_MENU_ACTION_BUTTONS:
					snesMenuOptions.actionButtons^=1;
					SNESOptionsUpdateText(SNES_MENU_ACTION_BUTTONS);
					break;
#endif
				case SNES_MENU_TRANSPARENCY:
					snesMenuOptions.transparency^=1;
					SNESOptionsUpdateText(SNES_MENU_TRANSPARENCY);
					break;
#if defined(__GP2X__) || defined(__WIZ__) || defined(__PANDORA__) || defined(__MIYOO__)
				case SNES_MENU_RENDER_MODE:
					if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
					{
						snesMenuOptions.renderMode++;
						if (snesMenuOptions.renderMode > RENDER_MODE_HORIZONTAL_SCALED)
							snesMenuOptions.renderMode = RENDER_MODE_UNSCALED;
					}
					else
					{
						snesMenuOptions.renderMode--;
						if (snesMenuOptions.renderMode > RENDER_MODE_HORIZONTAL_SCALED)
							snesMenuOptions.renderMode = RENDER_MODE_HORIZONTAL_SCALED;
					}
					SNESOptionsUpdateText(SNES_MENU_RENDER_MODE);
					break;
#endif
				case SNES_MENU_AUTO_SAVE_SRAM:
					//snesMenuOptions.autoSram^=1;
					if ((++snesMenuOptions.autoSram) > 2) snesMenuOptions.autoSram = 0;
					SNESOptionsUpdateText(SNES_MENU_AUTO_SAVE_SRAM);
					break;
				case SNES_MENU_EMULATION_TYPE:
					snesMenuOptions.asmspc700^=1;
					SNESOptionsUpdateText(SNES_MENU_EMULATION_TYPE);
					break;
				case SNES_MENU_LOAD_ROM_ON_INIT:
					snesMenuOptions.loadOnInit ^= 1;
					SNESOptionsUpdateText(SNES_MENU_LOAD_ROM_ON_INIT);				
					break;

			}
		}
		if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
		{
			switch(menufocus)
			{
				case SNES_MENU_ADVANCED_HACKS:
					memset(&headerDone,0,sizeof(headerDone));
					subaction = SNESHacksMenu();
					memset(&headerDone,0,sizeof(headerDone));
					SNESOptionsUpdateText_All();
					break;
				case SNES_MENU_LOAD_GLOBAL:
					LoadMenuOptions(snesOptionsDir, MENU_OPTIONS_FILENAME, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
					SNESOptionsUpdateText_All();
					break;
				case SNES_MENU_SAVE_GLOBAL:
					SaveMenuOptions(snesOptionsDir, MENU_OPTIONS_FILENAME, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
					break;
				case SNES_MENU_DELETE_GLOBAL:
					DeleteMenuOptions(snesOptionsDir,MENU_OPTIONS_FILENAME,MENU_OPTIONS_EXT,1);
					break;
				case SNES_MENU_LOAD_CURRENT:
					if(currentRomFilename[0]!=0)
					{
						LoadMenuOptions(snesOptionsDir, currentRomFilename, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
						SNESOptionsUpdateText_All();
					}
					break;
				case SNES_MENU_SAVE_CURRENT:
					if(currentRomFilename[0]!=0)
					{
						SaveMenuOptions(snesOptionsDir, currentRomFilename, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
					}
					break;
				case SNES_MENU_DELETE_CURRENT:
					if(currentRomFilename[0]!=0)
					{
						DeleteMenuOptions(snesOptionsDir, currentRomFilename, MENU_OPTIONS_EXT,1);
					}
					break;
				case SNES_MENU_SET_ROMDIR:
					SaveMenuOptions(snesOptionsDir, DEFAULT_ROM_DIR_FILENAME, DEFAULT_ROM_DIR_EXT, romDir, strlen(romDir),1);
					strcpy(snesRomDir,romDir);
					break;
				case SNES_MENU_CLEAR_ROMDIR:
					DeleteMenuOptions(snesOptionsDir, DEFAULT_ROM_DIR_FILENAME, DEFAULT_ROM_DIR_EXT,1);
					strcpy(snesRomDir,currentWorkingDir);
					break;
				case SNES_MENU_RETURN:
					menuExit=1;
					break;
			}	
		}
		// Draw screen:
		menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
		RenderMenu("SNES Options", menuCount,menuSmooth,menufocus);
		MenuFlip();
       
	}
  
  return action;
}

static int screenshotSaved;
static void MainMenuUpdateText(void)
{
	sprintf(menutext[MAIN_MENU_ROM_SELECT],"Select Rom");
	sprintf(menutext[MAIN_MENU_MANAGE_SAVE_STATE],"Manage Save States");
	sprintf(menutext[MAIN_MENU_SAVE_SRAM],"Save SRAM");
	if (!screenshotSaved) sprintf(menutext[MAIN_MENU_SAVE_SCREENSHOT],"Save Screenshot");
	else sprintf(menutext[MAIN_MENU_SAVE_SCREENSHOT],"[Screenshot already saved]");
	sprintf(menutext[MAIN_MENU_SNES_OPTIONS],"SNES Options");
	sprintf(menutext[MAIN_MENU_RESET_GAME],"Reset Game");
	sprintf(menutext[MAIN_MENU_EXIT_APP],"Exit Application");
	sprintf(menutext[MAIN_MENU_RETURN],"Return To Game");
}


int MainMenu(int prevaction)
{
	int menuExit=0,menuCount=MAIN_MENU_COUNT,menufocus=0,menuSmooth=0;
	int action=prevaction;
	int subaction=0;
	screenshotSaved = 0;	
		
	gp_setCpuspeed(MENU_CPU_SPEED);
	
	gp_initGraphics(16,currFB,1);
	gp_clearFramebuffer16((unsigned short*)framebuffer16[currFB],0x0);
	MenuFlip();
#if !defined(__WIZ__) && !defined(__MIYOO__)
	gp_video_RGB_setscaling(320,240);
#endif
	memset(&headerDone,0,sizeof(headerDone));
	MainMenuUpdateText();
	
	while (!menuExit)
	{
		InputUpdate(0);

		// Change which rom is focused on:
		if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
		if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
    
		if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) 
		{
			if(currentRomFilename[0]!=0)
			{
				menuExit=1;
			}
		}
    
		if (menufocus>menuCount-1)
		{
			menufocus=0;
			menuSmooth=(menufocus<<8)-1;
		}   
		else if (menufocus<0) 
		{
			menufocus=menuCount-1;
			menuSmooth=(menufocus<<8)-1;
		}

		if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
		{
			switch(menufocus)
			{
				case MAIN_MENU_ROM_SELECT:
					memset(&headerDone,0,sizeof(headerDone));
					subaction=LoadRomMenu();
					memset(&headerDone,0,sizeof(headerDone));
					if(subaction)
					{
						action=subaction;
						menuExit=1;
					}
					MainMenuUpdateText();
					break;

				case MAIN_MENU_MANAGE_SAVE_STATE:
					if(currentRomFilename[0]!=0)
					{
						memset(&headerDone,0,sizeof(headerDone));
						subaction=SaveStateMenu();
						if (subaction==100)
						{
							menuExit=1;
						}
						memset(&headerDone,0,sizeof(headerDone));
					}
					MainMenuUpdateText();
					break;
				case MAIN_MENU_SAVE_SRAM:
					if(currentRomFilename[0]!=0)
					{
						S9xSaveSRAM();
					}
					break;
				case MAIN_MENU_SNES_OPTIONS:

					memset(&headerDone,0,sizeof(headerDone));
					subaction=SNESOptionsMenu();
					memset(&headerDone,0,sizeof(headerDone));
					MainMenuUpdateText();
					break;
				case MAIN_MENU_SAVE_SCREENSHOT:
					if (!screenshotSaved) {
						ShowMessage("Saving screenshot...", 1);
						if (saveScreenShot() == 0) {
							screenshotSaved = 1;
							MainMenuUpdateText();
						}
					} 
					break;
				case MAIN_MENU_RESET_GAME	:
					if(currentRomFilename[0]!=0)
					{
						switch(currentEmuMode)
						{
							case EMU_MODE_SNES:
								action=EVENT_RESET_SNES_ROM;
								menuExit=1;
								break;
						}
					}
					break;
				case MAIN_MENU_RETURN:
					if(currentRomFilename[0]!=0)
					{
						menuExit=1;
					}
					break;
				case MAIN_MENU_EXIT_APP:
					action=EVENT_EXIT_APP;
					menuExit=1;
					break;
			}	
		}
		// Draw screen:
		menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
		RenderMenu("Main Menu", menuCount,menuSmooth,menufocus);
		MenuFlip();
       

	}
	
  WaitForButtonsUp();
  
  return action;
}



