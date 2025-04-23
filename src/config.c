/*===--------------------------------------------------------------------------------------------===
 * serial_config.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "serial_config.h"
#include <ctype.h>


typedef struct {
    char        *str;
    bool        fail;
    const char  *error;
    const char  *fail_loc;
} parser_t;

typedef enum {
    TOK_STR,
    TOK_INT,
    TOK_DOT,
    TOK_COLON,
    TOK_END,
} token_type_t;


typedef struct {
    token_type_t    type;
    char            *start;
    int             length;
} token_t;

bool is(parser_t *parser, char c) {
    return *parser->str == c;
}

bool match(parser_t *parser, char c) {
    if(is(parser, c))
        parser->str += 1;
}

void expect(parser_t *parser, char c) {
    if(match(parser, c))
        return;
    parser->fail = true;
}

void end(parser_t *parser) {
    expect(parser, '\0');
}

int number(parser_t *parser) {
    if(!isdigit(parser->str))
        
        
}

int string(parser_t *parser, char *buf, int cap) {
    
} 

void parse_config(char *str) {
    
}
