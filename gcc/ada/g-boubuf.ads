------------------------------------------------------------------------------
--                                                                          --
--                         GNAT LIBRARY COMPONENTS                          --
--                                                                          --
--                  G N A T . B O U N D E D _ B U F F E R S                 --
--                                                                          --
--                                 S p e c                                  --
--                                                                          --
--                     Copyright (C) 2003-2005, AdaCore                     --
--                                                                          --
-- GNAT is free software;  you can  redistribute it  and/or modify it under --
-- terms of the  GNU General Public License as published  by the Free Soft- --
-- ware  Foundation;  either version 2,  or (at your option) any later ver- --
-- sion.  GNAT is distributed in the hope that it will be useful, but WITH- --
-- OUT ANY WARRANTY;  without even the  implied warranty of MERCHANTABILITY --
-- or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License --
-- for  more details.  You should have  received  a copy of the GNU General --
-- Public License  distributed with GNAT;  see file COPYING.  If not, write --
-- to  the  Free Software Foundation,  51  Franklin  Street,  Fifth  Floor, --
-- Boston, MA 02110-1301, USA.                                              --
--                                                                          --
-- As a special exception,  if other files  instantiate  generics from this --
-- unit, or you link  this unit with other files  to produce an executable, --
-- this  unit  does not  by itself cause  the resulting  executable  to  be --
-- covered  by the  GNU  General  Public  License.  This exception does not --
-- however invalidate  any other reasons why  the executable file  might be --
-- covered by the  GNU Public License.                                      --
--                                                                          --
-- GNAT was originally developed  by the GNAT team at  New York University. --
-- It is now maintained by Ada Core Technologies Inc (http://www.gnat.com). --
--                                                                          --
------------------------------------------------------------------------------

--  This package provides a thread-safe generic bounded buffer abstraction.
--  Instances are useful directly or as parts of the implementations of other
--  abstractions, such as mailboxes.

--  Bounded_Buffer is declared explicitly as a protected type, rather than as
--  a simple limited private type completed as a protected type, so that
--  clients may make calls accordingly (i.e., conditional/timed entry calls).

with System;

generic
   type Element is private;
   --  The type of the values contained within buffer objects

package GNAT.Bounded_Buffers is
   pragma Pure;

   type Content is array (Positive range <>) of Element;
   --  Content is an internal artefact that cannot be hidden because protected
   --  types cannot contain type declarations.

   Default_Ceiling : constant System.Priority := System.Default_Priority;
   --  A convenience value for the Ceiling discriminant

   protected type Bounded_Buffer
      (Capacity : Positive;
      --  Objects of type Bounded_Buffer specify the maximum number of Element
      --  values they can hold via the discriminant Capacity.

      Ceiling : System.Priority)
      --  Users must specify the ceiling priority for the object. If the
      --  Real-Time Systems Annex is not in use this value is not important.
   is
      pragma Priority (Ceiling);

      entry Insert (Item : in Element);
      --  Insert Item into the buffer, blocks caller until space is available

      entry Remove (Item : out Element);
      --  Remove next available Element from buffer. Blocks caller until an
      --  Element is available.

      function Empty return Boolean;
      --  Returns whether the instance contains any Elements.
      --  Note: State may change immediately after call returns.

      function Full return Boolean;
      --  Returns whether any space remains within the instance.
      --  Note: State may change immediately after call returns.

      function Extent return Natural;
      --  Returns the number of Element values currently held
      --  within the instance.
      --  Note: State may change immediately after call returns.

   private
      Values   : Content (1 .. Capacity);
      --  The container for the values held by the buffer instance

      Next_In  : Positive := 1;
      --  The index of the next Element inserted. Wraps around

      Next_Out : Positive := 1;
      --  The index of the next Element removed. Wraps around

      Count    : Natural  := 0;
      --  The number of Elements currently held
   end Bounded_Buffer;

end GNAT.Bounded_Buffers;
