#ifndef _DISPLAY_EPD_W21_H_
#define _DISPLAY_EPD_W21_H_


#define EPD_WIDTH   800
#define EPD_HEIGHT  480



void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);
//8bit
#define Black   0x00  
#define White   0x11  
#define Green   0x66  
#define Blue    0x55  
#define Red     0x33  
#define Yellow  0x22 
#define Clean   0x77  

//4bit
#define black   0x00  
#define white   0x01  
#define green   0x06  
#define blue    0x05  
#define red     0x03  
#define yellow  0x02 
#define clean   0x07  



#define PSR         0x00
#define PWRR         0x01
#define POF         0x02
#define POFS        0x03
#define PON         0x04
#define BTST1       0x05
#define BTST2       0x06
#define DSLP        0x07
#define BTST3       0x08
#define DTM         0x10
#define DRF         0x12
#define PLL         0x30
#define CDI         0x50
#define TCON        0x60
#define TRES        0x61
#define REV         0x70
#define VDCS        0x82
#define T_VDCS      0x84
#define PWS         0xE3
//EPD
void EPD_W21_Init(void);
void EPD_init(void);
void PIC_display(const unsigned char* picData);
void EPD_sleep(void);
void EPD_refresh(void);
void lcd_chkstatus(void);
void PIC_display_Clear(void);
void EPD_horizontal(void);
void EPD_vertical(void);
void E6_color(unsigned char color);
void EPD_Display_White(void);
void EPD_Display_Black(void);
void EPD_Display_Yellow(void);
void EPD_Display_blue(void);
void EPD_Display_Green(void);
void EPD_Display_red(void);
void EPD_init_fast(void);
#endif
/***********************************************************
            end file
***********************************************************/
