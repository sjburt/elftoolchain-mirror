/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libdwarf/dwarf_abbrev.c,v 1.1 2008/05/22 02:14:23 jb Exp $
 */

#include <stdlib.h>
#include "_libdwarf.h"

static int
abbrev_add(Dwarf_CU cu, uint64_t entry, uint64_t tag, uint8_t children,
    uint64_t aboff, Dwarf_Abbrev *abp, Dwarf_Error *error)
{
	Dwarf_Abbrev ab;
	int ret = DWARF_E_NONE;

	if ((ab = malloc(sizeof(struct _Dwarf_Abbrev))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return DWARF_E_MEMORY;
	}

	/* Initialise the abbrev structure. */
	ab->ab_entry	= entry;
	ab->ab_tag	= tag;
	ab->ab_children	= children;
	ab->ab_offset	= aboff;
	ab->ab_length	= 0;	/* unknown yet. */
	ab->ab_atnum	= 0;	/* unknown yet. */

	/* Initialise the list of attribute definitions. */
	STAILQ_INIT(&ab->ab_attrdef);

	/* Add the abbrev to the list in the compilation unit. */
	STAILQ_INSERT_TAIL(&cu->cu_abbrev, ab, ab_next);

	if (abp != NULL)
		*abp = ab;

	return (ret);
}

static int
attrdef_add(Dwarf_Abbrev ab, uint64_t attr, uint64_t form, uint64_t adoff,
    Dwarf_AttrDef *adp, Dwarf_Error *error)
{
	Dwarf_AttrDef ad;
	
	if (ab == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DWARF_E_ARGUMENT);
	}

	if ((ad = malloc(sizeof(struct _Dwarf_AttrDef))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	/* Initialise the attribute definition structure. */
	ad->ad_attrib	= attr;
	ad->ad_form	= form;
	ad->ad_offset	= adoff;

	/* Add the attribute definition to the list in the abbrev. */
	STAILQ_INSERT_TAIL(&ab->ab_attrdef, ad, ad_next);

	/* Increase number of attribute counter. */
	ab->ab_atnum++;

	if (adp != NULL)
		*adp = ad;

	return (DWARF_E_NONE);
}

int
abbrev_init(Dwarf_Debug dbg, Dwarf_CU cu, Dwarf_Error *error)
{
	Dwarf_Abbrev ab;
	Elf_Data *d;
	int ret = DWARF_E_NONE;
	uint64_t attr;
	uint64_t entry;
	uint64_t form;
	uint64_t offset;
	uint64_t aboff;
	uint64_t adoff;
	uint64_t tag;
	u_int8_t children;

	d = dbg->dbg_s[DWARF_debug_abbrev].s_data;

	offset = cu->cu_abbrev_offset;

	while (offset < d->d_size) {
		aboff = offset;

		entry = read_uleb128(&d, &offset);
		/* Check if this is the end of the data: */
		if (entry == 0)
			break;

		tag = read_uleb128(&d, &offset);

		children = dbg->read(&d, &offset, 1);

		if ((ret = abbrev_add(cu, entry, tag, children, aboff, &ab,
		    error)) != DWARF_E_NONE)
			break;

		do {
			adoff = offset;
			attr = read_uleb128(&d, &offset);
			form = read_uleb128(&d, &offset);
			if (attr != 0)
				if ((ret = attrdef_add(ab, attr, form, adoff,
				    NULL, error)) != DWARF_E_NONE)
					return (ret);
		} while (attr != 0);

		ab->ab_length = offset - aboff;
	}

	return (ret);
}

Dwarf_Abbrev
abbrev_find(Dwarf_CU cu, uint64_t entry)
{
	Dwarf_Abbrev ab;

	ab = NULL;
	STAILQ_FOREACH(ab, &cu->cu_abbrev, ab_next) {
		if (ab->ab_entry == entry)
			break;
	}

	return (ab);
}
