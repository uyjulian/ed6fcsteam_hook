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


NTSTATUS PatchExeText(PVOID BaseAddress);

#define RAW_FILE_MAGIC  TAG4('EDFC')

//0x417C80+F1
static const PED6_DIR_ENTRY *DirCacheTable = (PED6_DIR_ENTRY*)0x5E1968;

#endif // _ED6FC_H_fa318148_52ea_4fd8_8eb7_2af9986ea179_
