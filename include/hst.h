/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_H
#define HST_H
#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 * Environments
 */

/* Each process and event is identified by a number. */
typedef uint64_t csp_id;
#define CSP_ID_FMT "0x%016" PRIx64

#define CSP_ID_NONE ((csp_id) 0)
#define CSP_PROCESS_NONE CSP_ID_NONE

struct csp {
    csp_id tau;
    csp_id tick;
    csp_id skip;
    csp_id stop;
};

struct csp *
csp_new(void);

void
csp_free(struct csp *csp);

/* Return the ID of the event with the given name.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
csp_id
csp_get_event_id(struct csp *csp, const char *name);

/* Return the ID of the event with the given name.  `name` does not need to be
 * NUL-terminated, but it cannot contain any NULs.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
csp_id
csp_get_sized_event_id(struct csp *csp, const char *name, size_t name_length);

/* Return the name of the event with the given ID.  If you haven't created an
 * event with that ID (via csp_get_event_id), we return NULL. */
const char *
csp_get_event_name(struct csp *csp, csp_id event);

/*------------------------------------------------------------------------------
 * Sets
 */

/* We preallocate a certain number of entries in the csp_id_set struct itself,
 * to minimize malloc overhead for small sets.  We automatically calculate a
 * number that makes the size of the csp_id_set itself a nice multiple of
 * sizeof(void *).  On 64-bit platforms this should currently evaluate to 12. */
#define CSP_ID_SET_INTERNAL_SIZE                      \
    (((sizeof(void *) * 16) /* target overall size */ \
      - sizeof(csp_id)      /* hash */                \
      - sizeof(csp_id *)    /* ids */                 \
      - sizeof(size_t)      /* count */               \
      - sizeof(size_t)      /* allocated_count */     \
      ) /                                             \
     sizeof(csp_id))

/* A set of IDs, stored as a sorted array.  This type is read-only; to construct
 * a set, use a csp_id_set_builder. */
struct csp_id_set {
    csp_id hash;
    csp_id *ids;
    size_t count;
    size_t allocated_count;
    csp_id internal[CSP_ID_SET_INTERNAL_SIZE];
};

void
csp_id_set_init(struct csp_id_set *set);

void
csp_id_set_done(struct csp_id_set *set);

bool
csp_id_set_eq(const struct csp_id_set *set1, const struct csp_id_set *set2);

/* Return whether set1 ⊆ set2 */
bool
csp_id_set_subseteq(const struct csp_id_set *set1,
                    const struct csp_id_set *set2);

/* Fills a set with a copy of another set, without having to go through a
 * builder first.  You must have already initialized `set`.  This is guaranteed
 * to be equivalent to (and likely more efficient than):
 *
 *     struct csp_id_set_builder  builder;
 *     csp_id_set_builder_init(&builder);
 *     csp_id_set_builder_merge(&builder, other);
 *     csp_id_set_build(set, &builder);
 *     csp_id_set_builder_done(&builder);
 */
void
csp_id_set_clone(struct csp_id_set *set, const struct csp_id_set *other);

/* Shortcut for constructing an ID set with one element, without having to go
 * through a builder.  This is guaranteed to be equivalent to (and likely more
 * efficient than):
 *
 *     struct csp_id_set_builder  builder;
 *     csp_id_set_builder_init(&builder);
 *     csp_id_set_builder_add(&builder, event);
 *     csp_id_set_build(set, &builder);
 *     csp_id_set_builder_done(&builder);
 */
void
csp_id_set_fill_single(struct csp_id_set *set, csp_id event);

/* Shortcut for constructing an ID set with two elements, without having to go
 * through a builder.  This is guaranteed to be equivalent to (and likely more
 * efficient than):
 *
 *     struct csp_id_set_builder  builder;
 *     csp_id_set_builder_init(&builder);
 *     csp_id_set_builder_add(&builder, e1);
 *     csp_id_set_builder_add(&builder, e2);
 *     csp_id_set_build(set, &builder);
 *     csp_id_set_builder_done(&builder);
 */
void
csp_id_set_fill_double(struct csp_id_set *set, csp_id e1, csp_id e2);

/* A writeable view of a set of IDs. */
struct csp_id_set_builder {
    csp_id hash;
    void *working_set;
};

/* Initialize and clear a set builder. */
void
csp_id_set_builder_init(struct csp_id_set_builder *builder);

void
csp_id_set_builder_done(struct csp_id_set_builder *builder);

/* Add a single ID to a set builder.  Return whether the ID is new (i.e., it
 * wasn't already in `builder`.) */
bool
csp_id_set_builder_add(struct csp_id_set_builder *builder, csp_id id);

/* Add several IDs to a set builder.  `ids` does not need to be sorted, and it's
 * okay for it to contain duplicates. */
void
csp_id_set_builder_add_many(struct csp_id_set_builder *builder, size_t count,
                            csp_id *ids);

/* Remove a single ID from a set builder.  Returns whether that ID was in the
 * builder or not. */
bool
csp_id_set_builder_remove(struct csp_id_set_builder *builder, csp_id id);

/* Remove several IDs from a set builder.  `ids` does not need to be sorted, and
 * it's okay for it to contain duplicates. */
void
csp_id_set_builder_remove_many(struct csp_id_set_builder *builder, size_t count,
                               csp_id *ids);

/* Add the contents of an existing set to a set builder. */
void
csp_id_set_builder_merge(struct csp_id_set_builder *builder,
                         const struct csp_id_set *set);

/* "Lock" the contents of a set builder, filling in a (read-only)
 * set.  The set builder will be cleared in the process, allowing you to
 * reuse it if you need to build several sets. */
void
csp_id_set_build(struct csp_id_set *set, struct csp_id_set_builder *builder);

/* "Lock" the contents of a set builder, filling in a (read-only)
 * set.  The set builder will NOT be cleared in the process, allowing you to
 * easily build several sets that have similar contents. */
void
csp_id_set_build_and_keep(struct csp_id_set *set,
                          struct csp_id_set_builder *builder);

/*------------------------------------------------------------------------------
 * Pairs
 */

struct csp_id_pair {
    csp_id from;
    csp_id to;
};

/* We preallocate a certain number of entries in the csp_id_pair_array struct
 * itself, to minimize malloc overhead for small arrays.  We automatically
 * calculate a number that makes the size of the csp_id_pair_array itself a nice
 * multiple of sizeof(void *).  On 64-bit platforms this should currently
 * evaluate to 6. */
#define CSP_ID_PAIR_ARRAY_INTERNAL_SIZE                        \
    (((sizeof(void *) * 16)          /* target overall size */ \
      - sizeof(struct csp_id_pair *) /* pairs */               \
      - sizeof(size_t)               /* count */               \
      - sizeof(size_t)               /* allocated_count */     \
      ) /                                                      \
     sizeof(struct csp_id_pair))

/* An array of pairs of IDs. */
struct csp_id_pair_array {
    struct csp_id_pair *pairs;
    size_t count;
    size_t allocated_count;
    struct csp_id_pair internal[CSP_ID_PAIR_ARRAY_INTERNAL_SIZE];
};

void
csp_id_pair_array_init(struct csp_id_pair_array *array);

void
csp_id_pair_array_done(struct csp_id_pair_array *array);

/* Ensure that the array's `pairs` field has enough allocated space to hold
 * `count` pairs.  The array's `count` will be set to `count` after this
 * returns; you must then fill in the `pairs` field with the actual ID pairs. */
void
csp_id_pair_array_ensure_size(struct csp_id_pair_array *array, size_t count);

bool
csp_id_pair_array_eq(const struct csp_id_pair_array *a1,
                     const struct csp_id_pair_array *a2);

/* We preallocate a certain number of entries in the csp_id_pair_set struct
 * itself, to minimize malloc overhead for small sets.  We automatically
 * calculate a number that makes the size of the csp_id_pair_set itself a nice
 * multiple of sizeof(void *).  On 64-bit platforms this should currently
 * evaluate to 6. */
#define CSP_ID_PAIR_SET_INTERNAL_SIZE                        \
    (((sizeof(void *) * 16)          /* target overall size */ \
      - sizeof(struct csp_id_pair *) /* pairs */               \
      - sizeof(size_t)               /* count */               \
      - sizeof(size_t)               /* allocated_count */     \
      ) /                                                      \
     sizeof(struct csp_id_pair))

/* An set of pairs of IDs. */
struct csp_id_pair_set {
    struct csp_id_pair *pairs;
    size_t count;
    size_t allocated_count;
    struct csp_id_pair internal[CSP_ID_PAIR_SET_INTERNAL_SIZE];
};

void
csp_id_pair_set_init(struct csp_id_pair_set *set);

void
csp_id_pair_set_done(struct csp_id_pair_set *set);

/* Update `dest` to contain the union of `dest` and `src`. */
void
csp_id_pair_set_union(struct csp_id_pair_set *dest,
                      const struct csp_id_pair_set *src);

bool
csp_id_pair_set_contains(const struct csp_id_pair_set *set,
                         struct csp_id_pair pair);

bool
csp_id_pair_set_eq(const struct csp_id_pair_set *set1,
                   const struct csp_id_pair_set *set2);

/* A writeable view of a set of pairs of IDs. */
struct csp_id_pair_set_builder {
    size_t count;
    void *working_set;
};

/* Initialize and clear a set builder. */
void
csp_id_pair_set_builder_init(struct csp_id_pair_set_builder *builder);

void
csp_id_pair_set_builder_done(struct csp_id_pair_set_builder *builder);

/* Add a single pair to a set builder. */
void
csp_id_pair_set_builder_add(struct csp_id_pair_set_builder *builder,
                            struct csp_id_pair pair);

/* "Lock" the contents of a pair set builder, filling in a (read-only)
 * set.  The set builder will be cleared in the process, allowing you to
 * reuse it if you need to build several sets. */
void
csp_id_pair_set_build(struct csp_id_pair_set *set,
                      struct csp_id_pair_set_builder *builder);

/*------------------------------------------------------------------------------
 * Equivalences
 */

/* Stores information about the "equivalence classes" of a set of processes.
 * All of the processes that have "equivalent" behavior (according to one of
 * CSP's semantic models) belong to the same equivalence class. */
struct csp_equivalences {
    void *classes;
    void *members;
};

void
csp_equivalences_init(struct csp_equivalences *equiv);

void
csp_equivalences_done(struct csp_equivalences *equiv);

/* Add a member to an equivalence class.  If the member was already in an
 * equivalence class, it is removed from that one before adding it to the new
 * one. */
void
csp_equivalences_add(struct csp_equivalences *equiv, csp_id class_id,
                     csp_id member_id);

/* Add the IDs of all of the equivalence classes to a set builder. */
void
csp_equivalences_build_classes(struct csp_equivalences *equiv,
                               struct csp_id_set_builder *builder);

/* Return the class that a member belongs to, or CSP_ID_NONE if that member
 * hasn't been added to an equivalence class yet. */
csp_id
csp_equivalences_get_class(struct csp_equivalences *equiv, csp_id member_id);

/* Add all of the members of an equivalence class to a set builder. */
void
csp_equivalences_build_members(struct csp_equivalences *equiv, csp_id class_id,
                               struct csp_id_set_builder *builder);

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp_process_iface {
    void (*initials)(struct csp *csp, struct csp_id_set_builder *builder,
                     void *ud);

    void (*afters)(struct csp *csp, csp_id initial,
                   struct csp_id_set_builder *builder, void *ud);

    csp_id (*get_id)(struct csp *csp, const void *temp_ud);

    size_t (*ud_size)(struct csp *csp, const void *temp_ud);

    void (*init_ud)(struct csp *csp, void *ud, const void *temp_ud);

    void (*done_ud)(struct csp *csp, void *ud);
};

/* Creates a new process.
 *
 * You provide a userdata object that contains whatever you need to define the
 * new process, and an interface that defines several callbacks related to the
 * process.  You pass in a "temporary" userdata, which only needs to exist for
 * the duration of this function call.  (So it's safe to allocate on the stack,
 * for instance.)
 *
 * This function is safe to call multiple times for the "same" process (i.e., a
 * process with exactly the same definition).  The first thing we do is call the
 * process's `get_id` callback to produce the ID of the new process.  If there
 * is already an existing process with the same ID, then we do nothing else and
 * return the ID of the existing process.
 *
 * If the process is new, then we need to turn the "temporary" userdata into a
 * "permanent" one.  To do this, we'll first call the process's `ud_size`
 * callback, which should return the size of the permanent userdata.  We'll then
 * allocate this much memory, and call the process's `init_ud` callback to fill
 * it in from the temporary userdata.
 *
 * If `ud` is non-NULL, we will fill it in with a pointer to the permanent
 * userdata that we create for you. */
csp_id
csp_process_init(struct csp *csp, const void *temp_ud, void **ud,
                 const struct csp_process_iface *iface);

void
csp_process_build_initials(struct csp *csp, csp_id process,
                           struct csp_id_set_builder *builder);

void
csp_process_build_afters(struct csp *csp, csp_id process, csp_id initial,
                         struct csp_id_set_builder *builder);

/*------------------------------------------------------------------------------
 * Constructing process IDs
 */

/* We want to use reproducible IDs for our processes, which only depend on the
 * definition of the process.  That is, if you try to define two processes with
 * exactly the same definition, you should end up with the same ID for each one,
 * without having to coordinate with anyone.
 *
 * That means we need some way to record what the definition of a process is,
 * and a way to translate those definitions into an ID.  We don't need a
 * super-precise definition of the process — for instance, we don't need the
 * full AST of a CSPM term.  It's enough to have a list of all of the "inputs"
 * that are needed for each kind of operator, and some tag to distinguish one
 * operator from another.
 *
 * The "scope" below is the operator tag.  You just declare a scope somewhere in
 * the file that deals with a particular operator.  For instance, the file that
 * defines the prefix (→) operator would include:
 *
 *     static struct csp_id_scope  prefix;
 *
 * You don't need to fill the struct in, it just needs to exist.  That scope
 * then provides a unique basis to generate IDs for all prefix processes.  The
 * prefix operator (a → B) has two inputs: the event `a` and the process `B`.
 * Both of those are represented internally by IDs, and so once you have the
 * prefix scope, and the IDs for event `a` and process `B`, you can easily
 * (and reproducibly) generate the ID for any prefix process:
 *
 *     static csp_id
 *     prefix_id(csp_id a, csp_id B)
 *     {
 *         csp_id  id = csp_id_start(&prefix);
 *         id = csp_id_add_id(id, a);
 *         id = csp_id_add_id(id, B);
 *         return id;
 *     }
 */

struct csp_id_scope {
    unsigned int unused;
};

csp_id
csp_id_start(struct csp_id_scope *scope);

csp_id
csp_id_add_id(csp_id id, csp_id id_to_add);

csp_id
csp_id_add_id_set(csp_id id, const struct csp_id_set *set);

csp_id
csp_id_add_name(csp_id id, const char *name);

csp_id
csp_id_add_name_sized(csp_id id, const char *name, size_t name_length);

/*------------------------------------------------------------------------------
 * Operators
 */

/* In all of the below:  `p` and `q` are processes.  `ps` is a set of processes.
 * `a` is an event. */

csp_id
csp_external_choice(struct csp *csp, csp_id p, csp_id q);

csp_id
csp_internal_choice(struct csp *csp, csp_id p, csp_id q);

csp_id
csp_prefix(struct csp *csp, csp_id a, csp_id p);

csp_id
csp_replicated_external_choice(struct csp *csp, const struct csp_id_set *ps);

csp_id
csp_replicated_internal_choice(struct csp *csp, const struct csp_id_set *ps);

csp_id
csp_sequential_composition(struct csp *csp, csp_id p, csp_id q);

/*------------------------------------------------------------------------------
 * Recursion
 */

/* A "recursion scope" is the main building block that you need to create
 * mutually recursive processes.  You can create one or more "recursion targets"
 * within the scope, each of which maps a name to a process.  But importantly,
 * you don't have to know in advance which process you're going to map each name
 * to.  That lets you define a name for a process, and then use that same name
 * in the definition of the process.  Presto, recursion! */

struct csp_recursion_scope {
    csp_id scope;
    size_t unfilled_count;
    void *names;
};

/* Initialize a new recursion scope.  You are responsible for passing in a
 * different `scope_id` for each recursion scope that you create. */
void
csp_recursion_scope_init(struct csp *csp, struct csp_recursion_scope *scope);

void
csp_recursion_scope_done(struct csp_recursion_scope *scope);

/* Return the process ID of the recursion target with the given name, creating
 * it if necessary.  The recursion target will initially be empty; you must
 * "fill" it by calling csp_recursion_scope_fill before destroying the scope. */
csp_id
csp_recursion_scope_get(struct csp *csp, struct csp_recursion_scope *scope,
                        const char *name);

/* Same as csp_recursion_scope_add, but providing an explicit length for `name`.
 * `name` does not need to be NUL-terminated, but it cannot contain any NULs. */
csp_id
csp_recursion_scope_get_sized(struct csp *csp,
                              struct csp_recursion_scope *scope,
                              const char *name, size_t name_length);

/* Fill a recursion target.  You must call this exactly once for each recursion
 * target that you create.  Returns false if this recursion target doesn't
 * exist, or if it has been filled already.  After this function returns, the
 * recursion target will behave exactly like `process`. */
bool
csp_recursion_scope_fill(struct csp_recursion_scope *scope, const char *name,
                         csp_id process);

/* Same as csp_recursion_scope_fill, but providing an explicit length for
 * `name`.  `name` does not need to be NUL-terminated, but it cannot contain any
 * NULs. */
bool
csp_recursion_scope_fill_sized(struct csp_recursion_scope *scope,
                               const char *name, size_t name_length,
                               csp_id process);

/*------------------------------------------------------------------------------
 * CSP₀
 */

/* Load in a CSP₀ process from an in-memory string, placing the ID of the new
 * process in `dest`.  Returns 0 on success.  If the CSP₀ process is invalid,
 * returns -1. */
int
csp_load_csp0_string(struct csp *csp, const char *str, csp_id *dest);

/*------------------------------------------------------------------------------
 * Process behavior
 */

enum csp_semantic_model { CSP_TRACES };

struct csp_behavior {
    enum csp_semantic_model model;
    csp_id hash;
    struct csp_id_set initials;
};

void
csp_behavior_init(struct csp_behavior *behavior);

void
csp_behavior_done(struct csp_behavior *behavior);

bool
csp_behavior_eq(const struct csp_behavior *b1, const struct csp_behavior *b2);

/* Fill in `behavior` with the behavior of `process` in the given semantic
 * model.  You must have already initialized `behavior`. */
void
csp_process_get_behavior(struct csp *csp, csp_id process,
                         enum csp_semantic_model model,
                         struct csp_behavior *behavior);

/* Fill in `behavior` with the behavior of a set of `processes` in the given
 * semantic model.  `processes` should be τ-closed, and so τ won't be included
 * in the set's behavior.  You must have already initialized `behavior`. */
void
csp_process_set_get_behavior(struct csp *csp,
                             const struct csp_id_set *processes,
                             enum csp_semantic_model model,
                             struct csp_behavior *behavior);

/*------------------------------------------------------------------------------
 * Denotational semantics
 */

/* We preallocate a certain number of entries in the csp_trace struct itself, to
 * minimize malloc overhead for small traces.  We automatically calculate a
 * number that makes the size of the csp_trace itself a nice multiple of
 * sizeof(void *).  On 64-bit platforms this should currently evaluate to 13. */
#define CSP_TRACE_INTERNAL_SIZE                       \
    (((sizeof(void *) * 16) /* target overall size */ \
      - sizeof(csp_id *)    /* ids */                 \
      - sizeof(size_t)      /* count */               \
      - sizeof(size_t)      /* allocated_count */     \
      ) /                                             \
     sizeof(csp_id))

/* A sequence of events. */
struct csp_trace {
    csp_id *events;
    size_t count;
    size_t allocated_count;
    csp_id internal[CSP_TRACE_INTERNAL_SIZE];
};

void
csp_trace_init(struct csp_trace *set);

void
csp_trace_done(struct csp_trace *set);

/* Ensure that the traces's `events` field has enough allocated space to hold
 * `count` events.  The trace's `count` will be set to `count` after this
 * returns; you must then fill in the `events` field with the actual events. */
void
csp_trace_ensure_size(struct csp_trace *trace, size_t count);

bool
csp_trace_eq(const struct csp_trace *trace1, const struct csp_trace *trace2);

/*------------------------------------------------------------------------------
 * Normalized LTS
 */

#define CSP_NODE_NONE CSP_ID_NONE

struct csp_normalized_lts;

struct csp_normalized_lts *
csp_normalized_lts_new(struct csp *csp, enum csp_semantic_model model);

void
csp_normalized_lts_free(struct csp_normalized_lts *lts);

/* Create a new normalized LTS node for the given set of processes, if it
 * doesn't already exist.  Returns whether a new node was created.  Places the
 * ID of the node into `id` either way.  We will make a copy of `processes` if
 * needed, so it's safe for you to reuse it after this function returns. */
bool
csp_normalized_lts_add_node(struct csp_normalized_lts *lts,
                            const struct csp_id_set *processes, csp_id *id);

/* Adds a new edge to a normalized LTS.  `from` and `to` must the IDs of nodes
 * that you've already created via csp_normalized_lts_add_node.  `event` must be
 * an event ID.  If there is already an edge with the same `from` and `event`,
 * it must also have the same `to`. */
void
csp_normalized_lts_add_edge(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event, csp_id to);

/* Add a "normalized root".  During a refinement check, this lets us keep track
 * of which normalized LTS node your Spec process belongs to.  (You won't have
 * to call this function directly; we'll keep track of this for you for each
 * process that you prenormalize.) */
void
csp_normalized_lts_add_normalized_root(struct csp_normalized_lts *lts,
                                       csp_id root_id,
                                       csp_id normalized_root_id);

/* Return the normalized LTS node that a particular original non-normalized
 * process belongs to.  (That is, if you start a refinement check for a
 * particular Spec process, which normalized LTS node should you start the
 * breadth-first search from?) */
csp_id
csp_normalized_lts_get_normalized_root(struct csp_normalized_lts *lts,
                                       csp_id root_id);

/* Return the behavior for a normalized LTS node.  `id` must a node that you've
 * already created via csp_normalized_lts_add_node.  We retain ownership of the
 * returned behavior. */
const struct csp_behavior *
csp_normalized_lts_get_node_behavior(struct csp_normalized_lts *lts, csp_id id);

/* Return the outgoing edges for a normalized LTS node.  `id` must a node that
 * you've already created via csp_normalized_lts_add_node. */
void
csp_normalized_lts_get_node_edges(struct csp_normalized_lts *lts, csp_id id,
                                  struct csp_id_pair_array *edges);

/* Return the set of processes for a normalized LTS node.  `id` must a node that
 * you've already created via csp_normalized_lts_add_node.  We retain ownership
 * of the returned set. */
const struct csp_id_set *
csp_normalized_lts_get_node_processes(struct csp_normalized_lts *lts,
                                      csp_id id);

/* Return the normalized node that can be reached by following `event` from the
 * given `from` process.  Returns CSP_NODE_NONE if there is no such node. */
csp_id
csp_normalized_lts_get_edge(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event);

/* Add the IDs of all of the normalized nodes in `lts` to `builder`. */
void
csp_normalized_lts_build_all_nodes(struct csp_normalized_lts *lts,
                                   struct csp_id_set_builder *builder);

/* Bisimulate all of the nodes in a normalized LTS, to find nodes that have
 * equivalent behavior.  Each node in the normalized LTS will have an entry in
 * `equivalence`; the value of each entry will be the "representative node" for
 * the equivalence class that the node belongs to.  All nodes in the same
 * equivalence class will have the same representative node. */
void
csp_normalized_lts_bisimulate(struct csp_normalized_lts *lts,
                              struct csp_equivalences *equiv);

/* Merge together equivalent nodes in the normalized LTS, according to the given
 * equivalence relation, placing the result into a new normalized LTS. */
void
csp_normalized_lts_merge_equivalences(struct csp_normalized_lts *dest,
                                      struct csp_equivalences *equiv);


/*------------------------------------------------------------------------------
 * Refinement
 */

/* Finds the closure of a set of initial processes for a particular event.  This
 * is the set of processes that can be reached from any of the initial processes
 * by only following (any number of occurrences of) that event.  The event will
 * usually be τ.
 *
 * `processes` should contain the initial processes to calculate the closure
 * for; it will be updated to contain all of the processes in the closure (which
 * must always include the initial processes). */
void
csp_process_find_closure(struct csp *csp, csp_id event,
                         struct csp_id_set *processes);

/* Prenormalizes a process, adding it to a normalized LTS.  Returns the ID of
 * the normalized LTS node representing the process. */
csp_id
csp_process_prenormalize(struct csp *csp, struct csp_normalized_lts *lts,
                         csp_id process);

#ifdef __cplusplus
}
#endif
#endif /* HST_H */
