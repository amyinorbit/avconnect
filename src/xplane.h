/*===--------------------------------------------------------------------------------------------===
 * xplane.h - description
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
#ifndef _XPLANE_H_
#define _XPLANE_H_

#include <XPLMDefs.h>
#include <XPLMUtilities.h>
#include <uns1/uns1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t size;
    const char *data;
} sasl_string_t;

PLUGIN_API int XPluginStart(char *name, char *sig, char *desc);
PLUGIN_API void XPluginStop(void);
PLUGIN_API int XPluginEnable(void);
PLUGIN_API void XPluginDisable(void);
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param);

const char *get_plugin_dir(void);
const char *get_xplane_dir(void);
const char *get_plane_dir(void); 

#ifdef __cplusplus
}
#endif

#endif /* ifndef _XPLANE_H_ */

