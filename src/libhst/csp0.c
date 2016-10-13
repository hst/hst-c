/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "ccan/likely/likely.h"
#include "hst.h"

#if defined(CSP0_DEBUG)
#include <stdio.h>
#define XDEBUG(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while (0)
#define DEBUG(...) \
    do { \
        fprintf(stderr, "[0x%02x] ", \
                (unsigned int) (unsigned char) *state->p); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while (0)
#else
#define XDEBUG(...)  /* do nothing */
#define DEBUG(...)   /* do nothing */
#endif

#define require(call) \
    do { \
        int  __rc = (call); \
        if (unlikely(__rc != 0)) { \
            return __rc; \
        } \
    } while (0)

struct csp0_parse_state {
    struct csp  *csp;
    const char  *p;
    const char  *eof;
};

struct csp0_identifier {
    const char  *start;
    size_t  length;
};

static bool
is_space(char ch)
{
    return ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r'
        || ch == '\t' || ch == '\v';
}

static bool
is_idstart(char ch)
{
    return (ch >= 'a' && ch <= 'z')
        || (ch >= 'A' && ch <= 'Z')
        || ch == '_';
}

static bool
is_idchar(char ch)
{
    return (ch >= 'a' && ch <= 'z')
        || (ch >= 'A' && ch <= 'Z')
        || (ch >= '0' && ch <= '9')
        || ch == '_' || ch == '.';
}

static void
skip_whitespace(struct csp0_parse_state *state)
{
    const char  *p = state->p;
    const char  *eof = state->eof;
    DEBUG("skip whitespace");
    while (likely(p < eof) && is_space(*p)) { p++; }
    state->p = p;
}

static int
parse_identifier(struct csp0_parse_state *state, struct csp0_identifier *dest)
{
    const char  *p = state->p;
    const char  *eof = state->eof;
    DEBUG("ENTER  identifier");
    if (unlikely(p == eof)) {
        // Unexpected end of input
        return -1;
    }
    if (*p == '$') {
        const char  *start = p++;
        DEBUG("ENTER  dollar identifier");
        if (unlikely(p == eof)) {
            // Unexpected end of input
            DEBUG("FAIL   dollar identifier");
            return -1;
        } else if (unlikely(!is_idchar(*p))) {
            // Identifier must contain one character after $
            DEBUG("FAIL   dollar identifier");
            return -1;
        }
        while (likely(p < eof) && is_idchar(*p)) { p++; }
        dest->start = start;
        dest->length = (p - start);
        state->p = p;
        DEBUG("ACCEPT dollar identifier `%.*s`", (int) dest->length, start);
        return 0;
    } else if (unlikely(!is_idstart(*p))) {
        // Expected identifier
        DEBUG("FAIL   identifier");
        return -1;
    } else {
        const char  *start = p++;
        while (likely(p < eof) && is_idchar(*p)) { p++; }
        dest->start = start;
        dest->length = (p - start);
        state->p = p;
        DEBUG("ACCEPT identifier `%.*s`", (int) dest->length, start);
        return 0;
    }
}

static int
parse_token(struct csp0_parse_state *state, const char *expected)
{
    const char  *p = state->p;
    const char  *eof = state->eof;
    size_t  expected_length = strlen(expected);
    DEBUG("ENTER  token `%s`", expected);
    if (unlikely((p + expected_length) > eof)) {
        // Unexpected end of input
        DEBUG("FAIL   token `%s`", expected);
        return -1;
    } else if (likely(memcmp(p, expected, expected_length) == 0)) {
        state->p += expected_length;
        DEBUG("ACCEPT token `%s`", expected);
        return 0;
    } else {
        // Expected something else
        DEBUG("FAIL   token `%s`", expected);
        return -1;
    }
}

static int
parse_process(struct csp0_parse_state *state, csp_id *dest);

/*
 * Precedence order (tightest first)
 *  1. () STOP SKIP
 *  2. →
 *  3. ;
 *  4. timeout
 *  5. interrupt
 *  6. □ (infix)
 *  7. ⊓ (infix)
 *  8. ||
 *  9. |||
 * 10. \
 * 11. replicated operators (prefix)
 */

/* Each of these numbered parse_process functions corresponds to one of the
 * entries in the precedence order list. */

static int
parse_process1(struct csp0_parse_state *state, csp_id *dest)
{
    // process1 = (process) | STOP | SKIP
    DEBUG("ENTER  process1");

    // (P)
    if (parse_token(state, "(") == 0) {
        skip_whitespace(state);
        require(parse_process(state, dest));
        skip_whitespace(state);
        require(parse_token(state, ")"));
        DEBUG("ACCEPT (P) 0x%08lx", *dest);
        return 0;
    }

    // STOP
    if (parse_token(state, "STOP") == 0) {
        *dest = csp_process_ref(state->csp, state->csp->stop);
        DEBUG("ACCEPT STOP 0x%08lx", state->csp->stop);
        return 0;
    }

    // SKIP
    if (parse_token(state, "SKIP") == 0) {
        *dest = csp_process_ref(state->csp, state->csp->skip);
        DEBUG("ACCEPT SKIP 0x%08lx", state->csp->skip);
        return 0;
    }

    // Unexpected token
    DEBUG("FAIL   process1");
    return -1;
}

static int
parse_process2(struct csp0_parse_state *state, csp_id *dest)
{
    // process2 = process1 | event → process2

    struct csp0_identifier  id;
    csp_id  event;
    csp_id  after;
    DEBUG("ENTER  process2");

    if (parse_process1(state, dest) == 0) {
        DEBUG("PASS   process2");
        return 0;
    }

    require(parse_identifier(state, &id));
    skip_whitespace(state);
    if (parse_token(state, "->") != 0 && parse_token(state, "→") != 0) {
        // Expected -> or →
        DEBUG("FAIL   process2");
        return -1;
    }
    skip_whitespace(state);
    require(parse_process2(state, &after));
    event = csp_get_sized_event_id(state->csp, id.start, id.length);
    *dest = csp_prefix(state->csp, event, after);
    DEBUG("ACCEPT process2 → 0x%08lx", *dest);
    return 0;
}

#define parse_process11  parse_process2  /* NIY */

static int
parse_process(struct csp0_parse_state *state, csp_id *dest)
{
    return parse_process11(state, dest);
}

int
csp_load_csp0_string(struct csp *csp, const char *str, csp_id *dest)
{
    struct csp0_parse_state  state;
    state.csp = csp;
    state.p = str;
    state.eof = strchr(str, '\0');
    XDEBUG("====== `%s`", str);
    skip_whitespace(&state);
    require(parse_process(&state, dest));
    skip_whitespace(&state);
    if (unlikely(state.p != state.eof)) {
        // Unexpected characters at end of stream
        csp_process_deref(csp, *dest);
        return -1;
    }
    return 0;
}
