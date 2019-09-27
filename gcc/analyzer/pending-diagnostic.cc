/* Classes for analyzer diagnostics.
   Copyright (C) 2019 Free Software Foundation, Inc.
   Contributed by David Malcolm <dmalcolm@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "gcc-plugin.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "diagnostic.h"
#include "analyzer/analyzer.h"
#include "analyzer/pending-diagnostic.h"

/* Generate a label_text by printing FMT.

   Use a clone of the global_dc for formatting callbacks.

   Use this evdesc::event_desc's m_colorize flag to control colorization
   (so that e.g. we can disable it for JSON output).  */

label_text
evdesc::event_desc::formatted_print (const char *fmt, ...) const
{
  pretty_printer *pp = global_dc->printer->clone ();

  pp_show_color (pp) = m_colorize;

  text_info ti;
  rich_location rich_loc (line_table, UNKNOWN_LOCATION);
  va_list ap;
  va_start (ap, fmt);
  ti.format_spec = _(fmt);
  ti.args_ptr = &ap;
  ti.err_no = 0;
  ti.x_data = NULL;
  ti.m_richloc = &rich_loc;
  pp_format (pp, &ti);
  pp_output_formatted_text (pp);
  va_end (ap);

  label_text result = label_text::take (xstrdup (pp_formatted_text (pp)));
  delete pp;
  return result;
}
