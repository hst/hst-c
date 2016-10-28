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

static int
parse_process_set(struct csp0_parse_state *state, struct csp_id_set *set)
{
    struct csp_id_set_builder  builder;
    csp_id  process;
    DEBUG("ENTER  process set");

    csp_id_set_builder_init(&builder);
    require(parse_token(state, "{"));
    skip_whitespace(state);
    if (parse_process(state, &process) == 0) {
        csp_id_set_builder_add(&builder, process);
        skip_whitespace(state);
        while (parse_token(state, ",") == 0) {
            skip_whitespace(state);
            if (unlikely(parse_process(state, &process) != 0)) {
                // Expected process after `,`
                csp_id_set_build(set, &builder);
                csp_id_set_builder_done(&builder);
                DEBUG("FAIL   process set");
                return -1;
            }
            csp_id_set_builder_add(&builder, process);
            skip_whitespace(state);
        }
    }
    csp_id_set_build(set, &builder);
    csp_id_set_builder_done(&builder);
    if (unlikely(parse_token(state, "}") != 0)) {
        // Expected process `}`
        DEBUG("FAIL   process set");
        return -1;
    }
    DEBUG("ACCEPT process set (size=%u)", (unsigned int) set->count);
    return 0;
}

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
        DEBUG("ACCEPT (P) " CSP_ID_FMT, *dest);
        return 0;
    }

    // STOP
    if (parse_token(state, "STOP") == 0) {
        *dest = state->csp->stop;
        DEBUG("ACCEPT STOP " CSP_ID_FMT, state->csp->stop);
        return 0;
    }

    // SKIP
    if (parse_token(state, "SKIP") == 0) {
        *dest = state->csp->skip;
        DEBUG("ACCEPT SKIP " CSP_ID_FMT, state->csp->skip);
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
    DEBUG("ACCEPT process2 → " CSP_ID_FMT, *dest);
    return 0;
}

static int
parse_process3(struct csp0_parse_state *state, csp_id *dest)
{
    // process3 = process2 (; process3)?

    csp_id  lhs;
    csp_id  rhs;
    DEBUG("ENTER  process3");

    require(parse_process2(state, &lhs));
    skip_whitespace(state);
    if (parse_token(state, ";") != 0) {
        *dest = lhs;
        DEBUG("PASS   process3");
        return 0;
    }
    skip_whitespace(state);
    if (parse_process3(state, &rhs) != 0) {
        // Expected process after ;
        DEBUG("FAIL   process3");
        return -1;
    }
    *dest = csp_sequential_composition(state->csp, lhs, rhs);
    DEBUG("ACCEPT process3 ; " CSP_ID_FMT, *dest);
    return 0;
}

#define parse_process5  parse_process3  /* NIY */

static int
parse_process6(struct csp0_parse_state *state, csp_id *dest)
{
    // process6 = process5 (□ process6)?

    csp_id  lhs;
    csp_id  rhs;
    DEBUG("ENTER  process6");

    require(parse_process5(state, &lhs));
    skip_whitespace(state);
    if (parse_token(state, "[]") != 0 && parse_token(state, "□") != 0) {
        *dest = lhs;
        DEBUG("PASS   process6");
        return 0;
    }
    skip_whitespace(state);
    if (parse_process6(state, &rhs) != 0) {
        // Expected process after □
        DEBUG("FAIL   process6");
        return -1;
    }
    *dest = csp_external_choice(state->csp, lhs, rhs);
    DEBUG("ACCEPT process6 □ " CSP_ID_FMT, *dest);
    return 0;
}

static int
parse_process7(struct csp0_parse_state *state, csp_id *dest)
{
    // process7 = process6 (⊓ process7)?

    csp_id  lhs;
    csp_id  rhs;
    DEBUG("ENTER  process7");

    require(parse_process6(state, &lhs));
    skip_whitespace(state);
    if (parse_token(state, "|~|") != 0 && parse_token(state, "⊓") != 0) {
        *dest = lhs;
        DEBUG("PASS   process7");
        return 0;
    }
    skip_whitespace(state);
    if (parse_process7(state, &rhs) != 0) {
        // Expected process after ⊓
        DEBUG("FAIL   process7");
        return -1;
    }
    *dest = csp_internal_choice(state->csp, lhs, rhs);
    DEBUG("ACCEPT process7 ⊓ " CSP_ID_FMT, *dest);
    return 0;
}

#define parse_process10  parse_process7  /* NIY */

static int
parse_process11(struct csp0_parse_state *state, csp_id *dest)
{
    // process11 = process10 | □ {process} | ⊓ {process}

    struct csp_id_set  processes;
    DEBUG("ENTER  process11");

    // □ {process}
    if (parse_token(state, "[]") == 0 || parse_token(state, "□") == 0) {
        skip_whitespace(state);
        csp_id_set_init(&processes);
        if (unlikely(parse_process_set(state, &processes) != 0)) {
            csp_id_set_done(&processes);
            DEBUG("FAIL   process11");
            return -1;
        }
        *dest = csp_replicated_external_choice(state->csp, &processes);
        csp_id_set_done(&processes);
        DEBUG("ACCEPT process11 " CSP_ID_FMT, *dest);
        return 0;
    }

    // ⊓ {process}
    if (parse_token(state, "|~|") == 0 || parse_token(state, "⊓") == 0) {
        skip_whitespace(state);
        csp_id_set_init(&processes);
        if (unlikely(parse_process_set(state, &processes) != 0)) {
            csp_id_set_done(&processes);
            DEBUG("FAIL   process11");
            return -1;
        }
        *dest = csp_replicated_internal_choice(state->csp, &processes);
        csp_id_set_done(&processes);
        DEBUG("ACCEPT process11 " CSP_ID_FMT, *dest);
        return 0;
    }

    // process10
    if (likely(parse_process10(state, dest) == 0)) {
        DEBUG("PASS   process11");
        return 0;
    } else {
        DEBUG("FAIL   process11");
        return -1;
    }
}

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
        return -1;
    }
    return 0;
}