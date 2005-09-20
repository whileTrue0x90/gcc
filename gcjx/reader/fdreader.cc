// Read from a file descriptor.

// Copyright (C) 2004, 2005 Free Software Foundation, Inc.
//
// This file is part of GCC.
//
// gcjx is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// gcjx is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with gcjx; see the file COPYING.LIB.  If
// not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "typedefs.hh"

#include "reader/fdreader.hh"
#include "reader/mmapbuffer.hh"

fd_reader::~fd_reader ()
{
  ::close (fd);
}

byte_buffer *
fd_reader::read_all ()
{
  // FIXME: configury.
  note_read ();
  return new mmap_byte_buffer (get_location (), fd);
}

time_t
fd_reader::get_mtime ()
{
  struct stat stat_buf;
  if (fstat (fd, &stat_buf) != 0 || ! S_ISREG (stat_buf.st_mode))
    // FIXME should include perror.
    throw class_file_error (get_location (),
			    "couldn't stat or not a regular file");
  return stat_buf.st_mtime;
}
