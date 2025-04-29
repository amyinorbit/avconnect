/*===--------------------------------------------------------------------------------------------===
 * parser_tokens.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "parser.h"

static const char *token_names[] = {

    [TOKEN_PLUS] = "plus",
    [TOKEN_MINUS] = "minus",
    [TOKEN_SLASH] = "slash",
    [TOKEN_STAR] = "star",
    [TOKEN_STARSTAR] = "star_star",
    [TOKEN_PERCENT] = "percent",
    [TOKEN_CARET] = "caret",
    [TOKEN_TILDE] = "tilde",
    [TOKEN_AMP] = "amp",
    [TOKEN_PIPE] = "pipe",
    [TOKEN_BANG] = "bang",
    [TOKEN_QUESTION] = "question",
    [TOKEN_LT] = "less",
    [TOKEN_GT] = "greater",
    [TOKEN_EQUALS] = "equal",
    [TOKEN_LTEQ] = "less_equal",
    [TOKEN_GTEQ] = "greater_equal",
    [TOKEN_EQEQ] = "equal_equal",
    [TOKEN_PLUSEQ] = "plus_eqal",
    [TOKEN_MINUSEQ] = "minus_eqal",
    [TOKEN_STAREQ] = "star_eqal",
    [TOKEN_SLASHEQ] = "slash_eqal",
    [TOKEN_BANGEQ] = "bang_eqal",
    [TOKEN_LTLT] = "less_less",
    [TOKEN_GTGT] = "greater_greater",
    [TOKEN_GTGTEQ] = "greater_greater_equal",
    [TOKEN_AMPAMP] = "and_and",
    [TOKEN_PIPEPIPE] = "pipe_pipe",
    
    [TOKEN_SEMICOLON] = "semicolon",
    [TOKEN_COMMA] = "comma",
    [TOKEN_NEWLINE] = "newline",

    [TOKEN_TRUE] = "true",
    [TOKEN_FALSE] = "false",
    [TOKEN_NIL] = "nil",
    
    [TOKEN_NUMBER] = "number_literal",
    [TOKEN_STRING] = "string_literal",
    [TOKEN_IDENTIFIER] = "identifier",
    
    [TOKEN_IF] = "if",
    [TOKEN_THEN] = "then",
    [TOKEN_ELSE] = "else",
    [TOKEN_END] = "end",
    
    [TOKEN_EOF] = "eof",
    [TOKEN_INVALID] = "invalid",
};

const char *token_name(token_type_t type) {
    return token_names[type];
}

static bool is_at_end(const parser_t *parser) {
    return *parser->current == '\0' || parser->current == parser->end;
}

static inline const char *src_end(const parser_t *parser) {
    return parser->end;
}

static inline size_t src_left(const parser_t *parser) {
    return (size_t)(src_end(parser) - parser->current);
}

static unicode_scalar_t lex_advance(parser_t *parser) {
    unicode_scalar_t current = parser->copy;
    parser->current += unicode_utf8_size(parser->copy);

    uint8_t size = 0;
    parser->copy = unicode_utf8_read(parser->current, src_left(parser), &size);
    return size ? current : '\0';
}

static unicode_scalar_t lex_peek(const parser_t *parser) {
    return parser->copy;
}

static unicode_scalar_t lex_peek_next(const parser_t *parser) {
    if(is_at_end(parser)) return '\0';
    uint8_t current_size = unicode_utf8_size(parser->copy);
    uint8_t size = 0;
    unicode_scalar_t nc = unicode_utf8_read(parser->current + current_size,
                                            src_left(parser)-current_size,
                                            &size);
    return size ? nc : '\0';
}


static void skip_whitespace(parser_t *parser) {
    for(;;) {
        unicode_scalar_t c = lex_peek(parser);
        switch(c) {
        case ' ':
        case '\t':
        case '\r':
            lex_advance(parser);
            break;
            
        case '\n':
            lex_advance(parser);
            parser->line ++;
            parser->start_of_line = true;
            break;
            
        case '#':
            while(lex_peek(parser) != '\n' && !is_at_end(parser)) {
                lex_advance(parser);
            }
            break;
        case '/':
            if(lex_peek_next(parser) == '/') {
                while(lex_peek(parser) != '\n' && !is_at_end(parser)) {
                    lex_advance(parser);
                }
            } else {
                return;
            }
            break;
            
        default:
            return;
        }
    }
}

token_t scan_token(parser_t *parser) {
    skip_whitespace(parser);
    parser->token_start = parser->src;
    if(is_at_end(parser))
        return make_token(parser, TOKEN_EOF);
    
    token_t tok = error_token(parser);
    // TODO: diagnostic
    return tok;
}

