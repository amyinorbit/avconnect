/*===--------------------------------------------------------------------------------------------===
 * parser.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _PARSER_H_
#define _PARSER_H_

#include "mscript_impl.h"

typedef enum {
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_STARSTAR,
    TOKEN_PERCENT,
    TOKEN_CARET,
    TOKEN_TILDE,
    TOKEN_AMP,
    TOKEN_PIPE,
    TOKEN_BANG,
    TOKEN_QUESTION,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_EQUALS,
    TOKEN_LTEQ,
    TOKEN_GTEQ,
    TOKEN_EQEQ,
    TOKEN_PLUSEQ,
    TOKEN_MINUSEQ,
    TOKEN_STAREQ,
    TOKEN_SLASHEQ,
    TOKEN_BANGEQ,
    TOKEN_LTLT,
    TOKEN_GTGT,
    TOKEN_GTGTEQ,
    TOKEN_AMPAMP,
    TOKEN_PIPEPIPE,
    
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_NEWLINE,
    
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL,
    
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_END,
    
    TOKEN_EOF,
    TOKEN_INVALID,
} token_type_t;

typedef struct {
    token_type_t    type;
    int             len;
    int             line;
    bool            start_of_line;
    const char      *start;
    mscript_val_t   value;
} token_t;


typedef struct {
    mscript_t       *vm;
    
    const char      *src;
    const char      *end;
    
    const char      *token_start;
    const char      *current;
    bool            start_of_line;
    
    tok_t           current_token;
    tok_t           prev_token;
    
    bool            had_error;
} parser_t;


void parser_init(parser_t *parser, mscript_t *vm, const char *fname, const char *src, int len);

token_t scan_token(parser_t *parser);
const char *token_name(token_type_t type);

void error_at(parser_t *parser, const token_t *token, const char *fmt, ...);
void error_at_varg(parser_t *parser, const token_t *token, const char *fmt, va_list args);

token_t *previous(parser_t *parser);
token_t *current(parser_t *parser);

void synchronize(parser_t *parser);

bool check(parser_t *parser, token_type_t type);
bool check_terminator(parser_t *parser);
bool match(parser_t *parser, token_type_t type);

void consume_terminator(parser_t *parser, const char *msg);
void advance(parser_t *parser);
void consume(parser_t *parser, token_type_t type, const char *msg);

#endif /* ifndef _PARSER_H_ */
