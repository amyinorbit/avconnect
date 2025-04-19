/*===--------------------------------------------------------------------------------------------===
 * inputs.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _INPUTS_H_
#define _INPUTS_H_

#include <stdbool.h>
#include <XPLMUtilities.h>

#ifdef __cplusplus
extern "C" {
#endif
    
typedef enum {
    AV_IN_ENCODER,
    AV_IN_BUTTON,
    AV_IN_MUX,
} av_in_type_t;


typedef struct {
    char            path[128];
    bool            has_changed;
    bool            has_resolved;
    XPLMCommandRef  ref;
} av_cmd_t;


typedef struct {
    av_in_type_t    type;
    char            name[32];
    char            comment[64];
} av_in_t;

typedef struct {
    av_in_t         base;
    av_cmd_t        cmd_up;
    av_cmd_t        cmd_dn;
} av_in_encoder_t;

typedef struct {
    av_in_t         base;
    av_cmd_t        cmd;
} av_in_button_t;

typedef struct {
    av_in_t         base;
    int             pin;
    av_cmd_t        cmd;
} av_in_mux_t;

#ifdef __cplusplus
}
#endif

#endif /* ifndef _INPUTS_H_ */
