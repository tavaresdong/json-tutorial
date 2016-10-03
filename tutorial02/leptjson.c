#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <regex.h>
#include <math.h>
#include <errno.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/* Deprecated, use lept_parse_literal
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
*/

static int lept_parse_literal(lept_context* c, lept_value* v,
                              const char* literal, lept_type parsed_type) {
    size_t i = 0;
    for (; literal[i] != '\0'; ++i) {
        if (*(c->json) != literal[i]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        ++c->json;
    }
    v->type = parsed_type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    regex_t regex;
    int regi;

    /* test for " after zero should be '.' or nothing " */
    if (c->json[0] == '0' && !(c->json[1] == '\0' || c->json[1] == '.')) {
        ++c->json;
        v->n = 0.0;
        v->type = LEPT_NUMBER;
        return LEPT_PARSE_OK;
    }

    regi = regcomp(&regex, "^-?(0|[1-9][0-9]*)(.[0-9]+)?([eE][+-]?[0-9]*)?$",
                   REG_EXTENDED|REG_NOSUB);
    assert(regi == 0); 
    regi = regexec(&regex, c->json, 0, NULL, 0);
    if (regi != 0) {
        v->type = LEPT_NULL;
        return LEPT_PARSE_INVALID_VALUE;
    }

    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    if (v->n == HUGE_VAL || v->n == -HUGE_VAL) {
        v->type = LEPT_NULL;
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
