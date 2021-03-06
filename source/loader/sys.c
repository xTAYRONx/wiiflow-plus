#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include "mem2.hpp"
#include <string.h>
#include "sys.h"
#include "gecko.h"

#include <ogc/machine/processor.h>
#include "sha1.h"
#include "fs.h"
#include "mem2.hpp"

/* Variables */
static bool reset = false;
static bool shutdown = false;
bool exiting = false;

static bool return_to_hbc = false;
static bool return_to_menu = false;
static bool return_to_priiloader = false;
static bool return_to_disable = false;
static bool return_to_bootmii = false;


void __Wpad_PowerCallback(s32 chan)
{
	/* Poweroff console */
	shutdown = 1;
}

void Open_Inputs(void)
{
	s32 status = WPAD_GetStatus();
	if(status != WPAD_STATE_ENABLED && status != WPAD_STATE_ENABLING)
	{
		/* Initialize Wiimote subsystem */
		PAD_Init();
		WPAD_Init();

		/* Set POWER button callback */
		WPAD_SetPowerButtonCallback(__Wpad_PowerCallback);
	
		WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	
		WPAD_SetIdleTimeout(60*5); // idle after 5 minutes
	}
}

void Close_Inputs(void)
{
	while(WPAD_GetStatus() == WPAD_STATE_ENABLING); //Possible freeze if i keep this here?
	if(WPAD_GetStatus() == WPAD_STATE_ENABLED)
	{
		WPAD_Flush(WPAD_CHAN_ALL);
		WPAD_Shutdown();
	}
}

bool Sys_Exiting(void)
{
	return reset || shutdown || exiting;
}

void Sys_Test(void)
{
	if(reset || shutdown) Close_Inputs();

	if (reset) SYS_ResetSystem(SYS_RESTART, 0, 0);
	else if (shutdown) SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

void Sys_ExitTo(int option)
{
	return_to_hbc = option == EXIT_TO_HBC;
	return_to_menu = option == EXIT_TO_MENU;
	return_to_priiloader = option == EXIT_TO_PRIILOADER;
	return_to_disable = option == EXIT_TO_DISABLE;
	return_to_bootmii = option == EXIT_TO_BOOTMII;

	//magic word to force wii menu in priiloader.
	if(return_to_menu)
	{
		Write32(0x8132fffb, 0x50756e65);
	}
	else if(return_to_priiloader)
	{
		Write32(0x8132fffb,0x4461636f);
	}
	else
	{
		Write32(0x8132fffb,0xffffffff);
	}
}

void Sys_Exit(void)
{
	if(return_to_disable) return;

	/* Shutdown Inputs */
	Close_Inputs();

	if (return_to_menu || return_to_priiloader) Sys_LoadMenu();
	else if(return_to_bootmii) IOS_ReloadIOS(254);
	if(WII_LaunchTitle(HBC_108)<0)
		if(WII_LaunchTitle(HBC_HAXX)<0)
			if(WII_LaunchTitle(HBC_JODI)<0)
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void __Sys_ResetCallback(void)
{
	reset = true;
}

void __Sys_PowerCallback(void)
{
	shutdown = true;
}


void Sys_Init(void)
{
	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void Sys_LoadMenu(void)
{
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}