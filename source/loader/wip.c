#include <gccore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h" //SAFE_CLOSE
#include "gecko.h"

typedef struct
{
    u32 offset;
    u32 srcaddress;
    u32 dstaddress;
} WIP_Code;

static WIP_Code * CodeList = NULL;
static u32 CodesCount = 0;
static u32 ProcessedLength = 0;
static u32 Counter = 0;

void do_wip_code(u8 * dst, u32 len)
{
    if(!CodeList)
        return;

    if(Counter < 3)
    {
        Counter++;
        return;
    }

    int i = 0;
    int n = 0;
    int offset = 0;

    for(i = 0; i < CodesCount; i++)
    {
        for(n = 0; n < 4; n++)
        {
            offset = CodeList[i].offset+n-ProcessedLength;

            if(offset < 0 || offset >= len)
                continue;

            if(dst[offset] == ((u8 *)&CodeList[i].srcaddress)[n])
            {
                dst[offset] = ((u8 *)&CodeList[i].dstaddress)[n];
                gprintf("WIP: %08X Address Patched.\n", CodeList[i].offset+n);
            }
            else
            {
                gprintf("WIP: %08X Address does not match with WIP entrie.\n", CodeList[i].offset+n);
                gprintf("Destination: %02X | Should be: %02X.\n", dst[offset], ((u8 *)&CodeList[i].srcaddress)[n]);
            }
        }
    }
    ProcessedLength += len;
    Counter++;
}

void wip_reset_counter()
{
    ProcessedLength = 0;
    //alternative dols don't need a skip. only main.dol.
    Counter = 3;
}

void free_wip()
{
	if(CodeList)
        SAFE_FREE(CodeList);

    CodesCount = 0;
    ProcessedLength = 0;
}

int load_wip_patches(const char *dir, const char *gameid)
{
	char filepath[150];
	char GameID[7];
    memcpy(GameID, gameid, 6);
	snprintf(filepath, sizeof(filepath), "%s/%s.wip", dir, GameID);

	FILE * fp = fopen(filepath, "rb");
	if (!fp)
	{
		bzero(GameID, sizeof GameID);
        memcpy(GameID, gameid, 3);
		snprintf(filepath, sizeof(filepath), "%s/%s.wip", dir, GameID);
		fp = fopen(filepath, "rb");
	}

    if (!fp)
        return -1;

    char line[255];
    gprintf("\nLoading WIP code from %s.\n", filepath);

    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '#') continue;

        if(strlen(line) < 26) continue;

        u32 offset = (u32) strtoul(line, NULL, 16);
        u32 srcaddress = (u32) strtoul(line+9, NULL, 16);
        u32 dstaddress = (u32) strtoul(line+18, NULL, 16);

        if(!CodeList) CodeList = malloc(sizeof(WIP_Code));

        WIP_Code * tmp = realloc(CodeList, (CodesCount+1)*sizeof(WIP_Code));
        if(!tmp)
        {
            SAFE_FREE(CodeList);
            SAFE_CLOSE(fp);
            return -1;
        }

        CodeList = tmp;

        CodeList[CodesCount].offset = offset;
        CodeList[CodesCount].srcaddress = srcaddress;
        CodeList[CodesCount].dstaddress = dstaddress;
        CodesCount++;
    }
    SAFE_CLOSE(fp);
    gprintf("\n");

	return 0;
}
