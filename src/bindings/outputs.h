/*===--------------------------------------------------------------------------------------------===
 * outputs.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _OUTPUTS_H_
#define _OUTPUTS_H_

#include <XPLMDataAccess.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        AV_OUT_PWM,
        AV_OUT_SHIFT_REG,
    } av_out_type_t;
    
    typedef struct {
        av_out_type_t   type;
        char            comment[64];
    } av_out_t;


#ifdef __cplusplus
}
#endif


#endif /* ifndef _OUTPUTS_H_ */


