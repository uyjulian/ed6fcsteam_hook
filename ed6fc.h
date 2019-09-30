#ifndef _ED6FC_H_fa318148_52ea_4fd8_8eb7_2af9986ea179_
#define _ED6FC_H_fa318148_52ea_4fd8_8eb7_2af9986ea179_

#include "ml.h"

using ml::String;
using ml::GrowableArray;

typedef struct
{
    CHAR  FileName[0xC];
    ULONG Unknown;
    ULONG Size;
    ULONG Unknown2[3];
    ULONG Offset;

} ED6_DIR_ENTRY, *PED6_DIR_ENTRY;

typedef struct
{
    DUMMY_STRUCT(0x24);
    ULONG FontSizeIndex;
    ULONG FontWeight;

} ED6_FC_FONT_RENDER, *PED6_FC_FONT_RENDER;

NTSTATUS PatchExeText(PVOID BaseAddress);

#define GET_GLYPHS_BITMAP_VA         
#define DRAW_TALK_TEXT_VA            
#define DRAW_DIALOG_TEXT_VA          
#define LOAD_FILE_FROM_DAT_VA        
#define DECOMPRESS_DATA_VA           
#define WINDOW_POSITION_1_ADDR       
#define WINDOW_POSITION_2_ADDR       
#define COMBAT_STATE_ADDR            
#define JP_FONT_SIZE_LIMIT_ADDR      
#define HP_EP_FONT_SIZE_ADDR         
#define PLACE_NAME_TEXT_X_DELTA_ADDR 

#define RAW_FILE_MAGIC  TAG4('EDFC')

//0x417C80+F1
static const PED6_DIR_ENTRY *DirCacheTable = (PED6_DIR_ENTRY*)0x5E1968;

#endif // _ED6FC_H_fa318148_52ea_4fd8_8eb7_2af9986ea179_
