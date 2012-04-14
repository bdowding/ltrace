/*
 * This file is part of ltrace.
 * Copyright (C) 2012 Petr Machata, Red Hat Inc.
 * Copyright (C) 2009 Juan Cespedes
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef BREAKPOINT_H
#define BREAKPOINT_H

/* XXX This is currently a very weak abstraction.  We would like to
 * much expand this to allow things like breakpoints on SDT probes and
 * such.
 *
 * In particular, we would like to add a tracepoint abstraction.
 * Tracepoint is a traceable feature--e.g. an exact address, a DWARF
 * symbol, an ELF symbol, a PLT entry, or an SDT probe.  Tracepoints
 * are named and the user can configure which of them he wants to
 * enable.  Realized tracepoints enable breakpoints, which are a
 * low-level realization of high-level tracepoint.
 *
 * Tracepoints are provided by the main binary as well as by any
 * opened libraries: every time an ELF file is mapped into the address
 * space, a new set of tracepoints is extracted, and filtered
 * according to user settings.  Those tracepoints that are left are
 * then realized, and the tracing starts.
 *
 * A scheme like this would take care of gradually introducing
 * breakpoints when the library is mapped, and therefore ready, and
 * would avoid certain hacks.  For example on PPC64, we don't actually
 * add breakpoints to PLT.  Instead, we read the PLT (which contains
 * addresses, not code), to figure out where to put the breakpoints.
 * In prelinked code, that address is non-zero, and points to an
 * address that's not yet mapped.  ptrace then fails when we try to
 * add the breakpoint.
 *
 * Ideally, return breakpoints would be just a special kind of
 * tracepoint that has attached some magic.  Or a feature of a
 * tracepoint.  Service breakpoints like the handling of dlopen would
 * be a low-level breakpoint, likely without tracepoint attached.
 *
 * So that's for sometimes.
 */

#include "sysdep.h"
#include "library.h"

struct Process;
struct breakpoint;

struct bp_callbacks {
	void (*on_hit)(struct breakpoint *bp, struct Process *proc);
	void (*on_continue)(struct breakpoint *bp, struct Process *proc);
};

struct breakpoint {
	struct bp_callbacks *cbs;
	struct library_symbol *libsym;
	void *addr;
	unsigned char orig_value[BREAKPOINT_LENGTH];
	int enabled;
	struct arch_breakpoint_data arch;
};

/* Call on-hit handler of BP, if any is set.  */
void breakpoint_on_hit(struct breakpoint *bp, struct Process *proc);

/* Call on-continue handler of BP.  If none is set, call
 * continue_after_breakpoint.  */
void breakpoint_on_continue(struct breakpoint *bp, struct Process *proc);

/* Initialize a breakpoint structure.  That doesn't actually realize
 * the breakpoint.  The breakpoint is initially assumed to be
 * disabled.  orig_value has to be set separately.  CBS may be
 * NULL.  */
int breakpoint_init(struct breakpoint *bp, struct Process *proc,
		    target_address_t addr, struct library_symbol *libsym);

/* Make a clone of breakpoint BP into the area of memory pointed to by
 * RETP.  The original breakpoint was assigned to process OLD_PROC,
 * the cloned breakpoint will be attached to process NEW_PROC.
 * Returns 0 on success or a negative value on failure.  */
int breakpoint_clone(struct breakpoint *retp, struct Process *new_proc,
		     struct breakpoint *bp, struct Process *old_proc);

/* Set callbacks.  If CBS is non-NULL, then BP->cbs shall be NULL.  */
void breakpoint_set_callbacks(struct breakpoint *bp, struct bp_callbacks *cbs);

/* XXX this is currently not called anywhere.   */
void breakpoint_destroy(struct breakpoint *bp);

/* Call enable_breakpoint the first time it's called.  Returns 0 on
 * success and a negative value on failure.  */
int breakpoint_turn_on(struct breakpoint *bp, struct Process *proc);

/* Call disable_breakpoint when turned off the same number of times
 * that it was turned on.  Returns 0 on success and a negative value
 * on failure.  */
int breakpoint_turn_off(struct breakpoint *bp, struct Process *proc);

/* This is actually several functions rolled in one:
 *  - malloc
 *  - breakpoint_init
 *  - proc_add_breakpoint
 *  - breakpoint_enable
 * XXX I think it should be broken up somehow.  */
struct breakpoint *insert_breakpoint(struct Process *proc, void *addr,
				     struct library_symbol *libsym);

/* Name of a symbol associated with BP.  May be NULL.  */
const char *breakpoint_name(const struct breakpoint *bp);

/* A library that this breakpoint comes from.  May be NULL.  */
struct library *breakpoint_library(const struct breakpoint *bp);

/* Again, this seems to be several interfaces rolled into one:
 *  - breakpoint_disable
 *  - proc_remove_breakpoint
 *  - breakpoint_destroy
 * XXX */
void delete_breakpoint(struct Process *proc, void *addr);

/* XXX some of the following belongs to proc.h/proc.c.  */
struct breakpoint *address2bpstruct(struct Process *proc, void *addr);
void enable_all_breakpoints(struct Process *proc);
void disable_all_breakpoints(struct Process *proc);
int breakpoints_init(struct Process *proc);

#endif /* BREAKPOINT_H */
