/*===--------------------------------------------------------------------------------------------===
 * serial_config.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include <stdint.h>
#include "device_impl.h"
#include <ctype.h>


typedef struct {
    char        *str;
    char        *cmd_end;
} parser_t;

typedef struct {
    char *start;
    char *end;
    bool is_last;
} token_t;

static char *skip_white_space(char *str) {
    if(str == NULL)
        return NULL;
    while(isspace(*str))
        str += 1;
    return *str != '\0' ? str : NULL;
}

static char *get_param_end(char *str) {
    if(str == NULL)
        return NULL;
    while(true) {
        if(*str == '\0' || *str == ':' || *str == '.')
            return str;
        str += 1;
    }
    return NULL;
}

static char *get_cmd_end(char *str) {
    if(str == NULL)
        return NULL;
    while(true) {
        if(*str == '\0')
            return NULL;
        if(*str == ':')
            break;
        str += 1;
    }
    return str;
}

static bool get_next_token(char *str, const char *cmd_end, token_t *tok) {
    if(str == NULL || cmd_end == NULL)
        return false;
    char *start = skip_white_space(str);
    if(start == NULL)
        return false;
    char *end = get_param_end(start);
    
    if(end == NULL || end == start)
        return false;
    
    tok->start = start;
    tok->end = end;
    tok->is_last = end == cmd_end;
    return true;
}

static const char *parser_get_param_str(parser_t *parser) {
    token_t tok;
    char *str = parser->str;
    const char *end = parser->cmd_end;
    if(!get_next_token(str, end, &tok))
        return NULL;
    
    *tok.end = '\0';
    parser->str = tok.end + 1;
    return tok.start;
}

static int32_t parser_get_param_int(parser_t *parser) {
    token_t tok;
    char *str = parser->str;
    const char *end = parser->cmd_end;
    if(!get_next_token(str, end, &tok))
        return INT32_MAX;
    
    *tok.end = '\0';
    int32_t val = (int32_t)atoi(tok.start);
    parser->str = tok.end + 1;
    return val;
}

static int32_t parser_get_cmd(parser_t *parser) {
    char *str = parser->str;
    if(str == NULL)
        return -1;
    parser->cmd_end = get_cmd_end(str);
    if(parser->cmd_end == NULL)
        return -1;
    return parser_get_param_int(parser);
}

static void parser_skip_param(parser_t *parser) {
    token_t tok;
    char *str = parser->str;
    const char *end = parser->cmd_end;
    if(!get_next_token(str, end, &tok))
        return;
    parser->str = tok.end + 1;
}

static void parser_skip_cmd(parser_t *parser) {
    if(parser->cmd_end == NULL)
        return;
    parser->str = parser->cmd_end;
}


// Mark: config parsers

static void parse_button(parser_t *parser, av_device_t *dev) {
    parser_skip_param(parser);
    const char *name = parser_get_param_str(parser);
    if(name == NULL)
        return;
    av_device_add_in_button_str(dev, name);
}

static void parse_encoder(parser_t *parser, av_device_t *dev) {
    parser_skip_param(parser); // Left Pin
    parser_skip_param(parser); // Right Pin
    parser_skip_param(parser); // Encoder type
    const char *name = parser_get_param_str(parser);
    if(name == NULL)
        return;
    av_device_add_in_encoder_str(dev, name);
}

static void parse_mux(parser_t *parser, av_device_t *dev) {
    parser_skip_param(parser); // Pin 1
    parser_skip_param(parser); // Pin 2
    parser_skip_param(parser); // Pin 3
    parser_skip_param(parser); // Pin 4
    const char *name = parser_get_param_str(parser);
    if(name == NULL)
        return;
    av_device_add_in_mux_str(dev, name);
}

static void parse_pwm(parser_t *parser, av_device_t *dev) {
    int32_t pin = parser_get_param_int(parser);
    parser_skip_param(parser);
    if(pin == INT32_MAX)
        return;
    av_device_add_out_pwm_id(dev, pin);
}

static void parse_sreg(parser_t *parser, av_device_t *dev) {
    parser_skip_param(parser); // Latch
    parser_skip_param(parser); // Clock
    parser_skip_param(parser); // Data
    int32_t module_count = parser_get_param_int(parser);
    if(module_count == INT32_MAX || module_count > 10 || module_count < 0)
        return;
    for(int32_t i = 0; i < module_count; ++i) {
        av_device_add_out_sreg_id(dev, i);
    }
}

void parse_config(av_device_t *dev, char *str) {
    
    parser_t parser = {
        .str = str,
        .cmd_end = NULL
    };
    
    int32_t cmd = 0;
    while((cmd = parser_get_cmd(&parser)) >= 0) {
        switch(cmd) {
            case 1: parse_button(&parser, dev);      break;
            case 3: parse_pwm(&parser, dev);         break;
            case 8: parse_encoder(&parser, dev);     break;
            case 10: parse_sreg(&parser, dev);       break;
            case 14: parse_mux(&parser, dev);        break;
        }
    }
    parser_skip_cmd(&parser);
}
