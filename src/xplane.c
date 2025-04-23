/*===--------------------------------------------------------------------------------------------===
 * xplane.c - UNS-1 plugin implementation
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * NOTICE:  All information contained herein is, and remains the property
 * of Amy Alex Parent. The intellectual and technical concepts contained
 * herein are proprietary to Amy Alex Parent and may be covered by U.S. and
 * Foreign Patents, patents in process, and are protected by trade secret
 * or copyright law. Dissemination of this information or reproduction of
 * this material is strictly forbidden unless prior written permission is
 * obtained from Amy Alex Parent.
 *===--------------------------------------------------------------------------------------------===
*/
#include <XPLMPlugin.h>
#include <XPLMPlanes.h>
#include <XPLMProcessing.h>

#include <acfutils/assert.h>
#include <acfutils/core.h>
#include <acfutils/crc64.h>
#include <acfutils/dr.h>
#include <acfutils/dr_cmd_reg.h>
#include <acfutils/glew.h>
#include <acfutils/helpers.h>
#include <acfutils/log.h>

#include "xplane.h"
#include "avconnect.h"

#define PLUGIN_NAME "AvConnect"
#define PLUGIN_DESCRIPTION "Connector plugin for serial devices"
#define PLUGIN_SIG "com.amyinorbit.avconnect"

static char xplane_dir[512];
static char plane_dir[512];
static char plugin_dir[512];
static char conf_dir[512];
static XPLMMenuID plugin_menu = NULL;

static void fix_path(char *path) {
    fix_pathsep(path);
    size_t len = strlen(path);
    VERIFY3U(len, !=, 0);
    if(path[len-1] == DIRSEP) {
        path[len-1] = '\0';
    }
}

static void get_paths() {
    char name[512]; // Useless, but we have to supply a pointer for the plane's name.
    
    XPLMGetSystemPath(xplane_dir);
	XPLMGetPluginInfo(XPLMGetMyID(), NULL, plugin_dir, NULL, NULL);
    XPLMGetNthAircraftModel(XPLM_USER_AIRCRAFT, name, plane_dir);
    
    snprintf(conf_dir, sizeof(conf_dir), "%s/Output/Preferences", xplane_dir);
    
#if	IBM
	fix_pathsep(xplane_dir);
	fix_pathsep(plugin_dir);
	fix_pathsep(plane_dir);
	fix_pathsep(conf_dir);
#endif	/* IBM */
    
	/* cut off the trailing path component (our filename) */
    char *p = NULL;
    if((p = strrchr(plane_dir, DIRSEP)) != NULL)
        *p = '\0';
    
    p = NULL;
	if ((p = strrchr(plugin_dir, DIRSEP)) != NULL)
		*p = '\0';
	/*
	 * Cut off an optional '32' or '64' trailing component. Please note
	 * that XPLM 3.0 now supports OS-specific suffixes, so clamp those
	 * away as well.
	 */
	if ((p = strrchr(plugin_dir, DIRSEP)) != NULL) {
		if (strcmp(p + 1, "64") == 0 || strcmp(p + 1, "32") == 0 ||
		    strcmp(p + 1, "win_x64") == 0 ||
		    strcmp(p + 1, "mac_x64") == 0 ||
		    strcmp(p + 1, "lin_x64") == 0)
			*p = '\0';
	}
    
    fix_path(plugin_dir);
    fix_path(xplane_dir);
    fix_path(plane_dir);
    fix_path(conf_dir);
}

static void log_printer(const char *msg) {
    fprintf(stderr, "%s", msg);
    XPLMDebugString(msg);
}

PLUGIN_API int XPluginStart(char *name, char *sig, char *desc) {
    
    log_init(log_printer, "avconnect");
    crc64_init();
    crc64_srand(microclock());
    
    glewInit();
    logMsg("AvConnect libacfutils %s)", libacfutils_version);
    
	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
	strcpy(name, PLUGIN_NAME);
	strcpy(sig, PLUGIN_SIG);
	strcpy(desc, PLUGIN_DESCRIPTION);
    
    get_paths();
    dcr_init();
    
    return 1;
}

PLUGIN_API void XPluginStop(void) {
    dcr_fini();
}

typedef enum {
    MENU_RELOAD = 0x10,
} menu_action_t;

static void menu_handler(void *menu_ref, void *item_ref) {
    UNUSED(menu_ref);
    UNUSED(item_ref);
    
    switch((menu_action_t)((intptr_t)item_ref)) {
    case MENU_RELOAD:
        XPLMReloadPlugins();
        break;
    }
}

PLUGIN_API int XPluginEnable(void) {
    
    XPLMMenuID plugins = XPLMFindPluginsMenu();
    int menu_item = XPLMAppendMenuItem(plugins, "AvConnect", NULL, 0);
    plugin_menu = XPLMCreateMenu("AvConnect", plugins, menu_item, menu_handler, NULL);
    XPLMAppendMenuItem(plugin_menu, "Reload Plugins", (void *)MENU_RELOAD, 0);
    avconnect_init();
    avconnect_conf_check_reload(false);
    return 1;
}

PLUGIN_API void XPluginDisable(void) {
    avconnect_fini();
    XPLMDestroyMenu(plugin_menu);
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
    UNUSED(from);
    UNUSED(msg);
    UNUSED(param);
    
    switch(msg) {
        case XPLM_MSG_PLANE_LOADED:
        if((intptr_t)param == 0) {
            get_paths();
            avconnect_conf_check_reload(true);
        }
        break;
    }
}

const char *get_plugin_dir(void) {
    return plugin_dir;
}

const char *get_xplane_dir(void) {
    return xplane_dir;
}

const char *get_plane_dir(void) {
    return plane_dir;
}

const char *get_conf_dir(void) {
    return conf_dir;
}

XPLMMenuID get_plugin_menu(void) {
    return plugin_menu;
}

#if	IBM
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID resvd)
{
	UNUSED(hinst);
	UNUSED(resvd);
	lacf_glew_dllmain_hook(reason);
	return (TRUE);
}
#endif	/* IBM */
