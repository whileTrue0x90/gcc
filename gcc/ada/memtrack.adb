------------------------------------------------------------------------------
--                                                                          --
--                         GNAT RUN-TIME COMPONENTS                         --
--                                                                          --
--                         S Y S T E M . M E M O R Y                        --
--                                                                          --
--                                 S p e c                                  --
--                                                                          --
--                            $Revision: 1.2 $
--                                                                          --
--             Copyright (C) 2001 Free Software Foundation, Inc.            --
--                                                                          --
-- This specification is derived from the Ada Reference Manual for use with --
-- GNAT. The copyright notice above, and the license provisions that follow --
-- apply solely to the  contents of the part following the private keyword. --
--                                                                          --
-- GNAT is free software;  you can  redistribute it  and/or modify it under --
-- terms of the  GNU General Public License as published  by the Free Soft- --
-- ware  Foundation;  either version 2,  or (at your option) any later ver- --
-- sion.  GNAT is distributed in the hope that it will be useful, but WITH- --
-- OUT ANY WARRANTY;  without even the  implied warranty of MERCHANTABILITY --
-- or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License --
-- for  more details.  You should have  received  a copy of the GNU General --
-- Public License  distributed with GNAT;  see file COPYING.  If not, write --
-- to  the Free Software Foundation,  59 Temple Place - Suite 330,  Boston, --
-- MA 02111-1307, USA.                                                      --
--                                                                          --
-- As a special exception,  if other files  instantiate  generics from this --
-- unit, or you link  this unit with other files  to produce an executable, --
-- this  unit  does not  by itself cause  the resulting  executable  to  be --
-- covered  by the  GNU  General  Public  License.  This exception does not --
-- however invalidate  any other reasons why  the executable file  might be --
-- covered by the  GNU Public License.                                      --
--                                                                          --
-- GNAT was originally developed  by the GNAT team at  New York University. --
-- Extensive contributions were provided by Ada Core Technologies Inc.      --
--                                                                          --
------------------------------------------------------------------------------

--  This version contains allocation tracking capability.
--  The object file corresponding to this instrumented version is to be found
--  in libgmem.
--  When enabled, the subsystem logs all the calls to __gnat_malloc and
--  __gnat_free. This log can then be processed by gnatmem to detect
--  dynamic memory leaks.
--
--  To use this functionality, you must compile your application with -g
--  and then link with this object file:
--
--     gnatmake -g program -largs -lgmem
--
--  After compilation, you may use your program as usual except that upon
--  completion, it will generate in the current directory the file gmem.out.
--
--  You can then investigate for possible memory leaks and mismatch by calling
--  gnatmem with this file as an input:
--
--    gnatmem -i gmem.out program
--
--  See gnatmem section in the GNAT User's Guide for more details.
--
--  NOTE: This capability is currently supported on the following targets:
--
--    Windows
--    GNU/Linux
--    HP-UX
--    Irix
--    Solaris
--    Tru64

pragma Source_File_Name (System.Memory, Body_File_Name => "memtrack.adb");

with Ada.Exceptions;
with System.Soft_Links;
with System.Traceback;

package body System.Memory is

   use Ada.Exceptions;
   use System.Soft_Links;
   use System.Traceback;

   function c_malloc (Size : size_t) return System.Address;
   pragma Import (C, c_malloc, "malloc");

   procedure c_free (Ptr : System.Address);
   pragma Import (C, c_free, "free");

   function c_realloc
     (Ptr : System.Address; Size : size_t) return System.Address;
   pragma Import (C, c_realloc, "realloc");

   type File_Ptr is new System.Address;

   function fopen (Path : String; Mode : String) return File_Ptr;
   pragma Import (C, fopen);

   procedure fwrite
     (Ptr    : System.Address;
      Size   : size_t;
      Nmemb  : size_t;
      Stream : File_Ptr);

   procedure fwrite
     (Str    : String;
      Size   : size_t;
      Nmemb  : size_t;
      Stream : File_Ptr);
   pragma Import (C, fwrite);

   procedure fputc (C : Integer; Stream : File_Ptr);
   pragma Import (C, fputc);

   procedure fclose (Stream : File_Ptr);
   pragma Import (C, fclose);

   procedure Finalize;
   --  Replace the default __gnat_finalize to properly close the log file.
   pragma Export (C, Finalize, "__gnat_finalize");

   Address_Size    : constant := System.Address'Max_Size_In_Storage_Elements;
   --  Size in bytes of a pointer

   Max_Call_Stack  : constant := 200;
   --  Maximum number of frames supported

   Skip_Frame : constant := 1;
   --  Number of frames to remove from the call stack to hide functions from
   --  this unit.

   Tracebk   : aliased array (0 .. Max_Call_Stack) of System.Address;
   Num_Calls : aliased Integer := 0;
   --  Store the current call stack from Alloc and Free

   Gmemfname : constant String := "gmem.out" & ASCII.NUL;
   --  Allocation log of a program is saved in a file gmem.out
   --  ??? What about Ada.Command_Line.Command_Name & ".out" instead of static
   --  gmem.out

   Gmemfile  : File_Ptr;
   --  Global C file pointer to the allocation log

   procedure Gmem_Initialize;
   --  Initialization routine; opens the file and writes a header string. This
   --  header string is used as a magic-tag to know if the .out file is to be
   --  handled by GDB or by the GMEM (instrumented malloc/free) implementation.

   -----------
   -- Alloc --
   -----------

   function Alloc (Size : size_t) return System.Address is
      Result      : aliased System.Address;
      Actual_Size : aliased size_t := Size;

   begin
      if Size = size_t'Last then
         Raise_Exception (Storage_Error'Identity, "object too large");
      end if;

      --  Change size from zero to non-zero. We still want a proper pointer
      --  for the zero case because pointers to zero length objects have to
      --  be distinct, but we can't just go ahead and allocate zero bytes,
      --  since some malloc's return zero for a zero argument.

      if Size = 0 then
         Actual_Size := 1;
      end if;

      Lock_Task.all;

      Result := c_malloc (Actual_Size);

      --  Logs allocation call
      --  format is:
      --   'A' <mem addr> <size chunk> <len backtrace> <addr1> ... <addrn>

      Gmem_Initialize;
      Call_Chain (Tracebk'Address, Max_Call_Stack, Num_Calls);
      Num_Calls := Num_Calls - Skip_Frame;
      fputc (Character'Pos ('A'), Gmemfile);
      fwrite (Result'Address, Address_Size, 1, Gmemfile);
      fwrite (Actual_Size'Address, size_t'Max_Size_In_Storage_Elements, 1,
              Gmemfile);
      fwrite (Num_Calls'Address, Integer'Max_Size_In_Storage_Elements, 1,
              Gmemfile);
      fwrite (Tracebk (Skip_Frame)'Address, Address_Size, size_t (Num_Calls),
              Gmemfile);

      Unlock_Task.all;

      if Result = System.Null_Address then
         Raise_Exception (Storage_Error'Identity, "heap exhausted");
      end if;

      return Result;
   end Alloc;

   --------------
   -- Finalize --
   --------------

   Needs_Init : Boolean := True;
   --  Reset after first call to Gmem_Initialize

   procedure Finalize is
   begin
      if not Needs_Init then
         fclose (Gmemfile);
      end if;
   end Finalize;

   ----------
   -- Free --
   ----------

   procedure Free (Ptr : System.Address) is
      Addr : aliased constant System.Address := Ptr;
   begin
      Lock_Task.all;

      --  Logs deallocation call
      --  format is:
      --   'D' <mem addr> <len backtrace> <addr1> ... <addrn>

      Gmem_Initialize;
      Call_Chain (Tracebk'Address, Max_Call_Stack, Num_Calls);
      Num_Calls := Num_Calls - Skip_Frame;
      fputc (Character'Pos ('D'), Gmemfile);
      fwrite (Addr'Address, Address_Size, 1, Gmemfile);
      fwrite (Num_Calls'Address, Integer'Max_Size_In_Storage_Elements, 1,
              Gmemfile);
      fwrite (Tracebk (Skip_Frame)'Address, Address_Size, size_t (Num_Calls),
              Gmemfile);

      c_free (Ptr);

      Unlock_Task.all;
   end Free;

   ---------------------
   -- Gmem_Initialize --
   ---------------------

   procedure Gmem_Initialize is
   begin
      if Needs_Init then
         Needs_Init := False;
         Gmemfile := fopen (Gmemfname, "wb" & ASCII.NUL);
         fwrite ("GMEM DUMP" & ASCII.LF, 10, 1, Gmemfile);
      end if;
   end Gmem_Initialize;

   -------------
   -- Realloc --
   -------------

   function Realloc
     (Ptr : System.Address; Size : size_t) return System.Address
   is
      Result : System.Address;
   begin
      if Size = size_t'Last then
         Raise_Exception (Storage_Error'Identity, "object too large");
      end if;

      Abort_Defer.all;
      Result := c_realloc (Ptr, Size);
      Abort_Undefer.all;

      if Result = System.Null_Address then
         Raise_Exception (Storage_Error'Identity, "heap exhausted");
      end if;

      return Result;
   end Realloc;

end System.Memory;
