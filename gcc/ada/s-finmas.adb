------------------------------------------------------------------------------
--                                                                          --
--                         GNAT COMPILER COMPONENTS                         --
--                                                                          --
--           S Y S T E M . F I N A L I Z A T I O N _ M A S T E R S          --
--                                                                          --
--                                 B o d y                                  --
--                                                                          --
--             Copyright (C) 2011, Free Software Foundation, Inc.           --
--                                                                          --
-- GNAT is free software;  you can  redistribute it  and/or modify it under --
-- terms of the  GNU General Public License as published  by the Free Soft- --
-- ware  Foundation;  either version 3,  or (at your option) any later ver- --
-- sion.  GNAT is distributed in the hope that it will be useful, but WITH- --
-- OUT ANY WARRANTY;  without even the  implied warranty of MERCHANTABILITY --
-- or FITNESS FOR A PARTICULAR PURPOSE.                                     --
--                                                                          --
-- As a special exception under Section 7 of GPL version 3, you are granted --
-- additional permissions described in the GCC Runtime Library Exception,   --
-- version 3.1, as published by the Free Software Foundation.               --
--                                                                          --
-- You should have received a copy of the GNU General Public License and    --
-- a copy of the GCC Runtime Library Exception along with this program;     --
-- see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see    --
-- <http://www.gnu.org/licenses/>.                                          --
--                                                                          --
-- GNAT was originally developed  by the GNAT team at  New York University. --
-- Extensive contributions were provided by Ada Core Technologies Inc.      --
--                                                                          --
------------------------------------------------------------------------------

with Ada.Exceptions;          use Ada.Exceptions;
with System.Address_Image;
with System.IO;               use System.IO;
with System.Soft_Links;       use System.Soft_Links;
with System.Storage_Elements; use System.Storage_Elements;

package body System.Finalization_Masters is

   ---------------------------
   -- Add_Offset_To_Address --
   ---------------------------

   function Add_Offset_To_Address
     (Addr   : System.Address;
      Offset : System.Storage_Elements.Storage_Offset) return System.Address
   is
   begin
      return System.Storage_Elements."+" (Addr, Offset);
   end Add_Offset_To_Address;

   ------------
   -- Attach --
   ------------

   procedure Attach (N : not null FM_Node_Ptr; L : not null FM_Node_Ptr) is
   begin
      Lock_Task.all;

      L.Next.Prev := N;
      N.Next := L.Next;
      L.Next := N;
      N.Prev := L;

      Unlock_Task.all;

      --  Note: No need to unlock in case of an exception because the above
      --  code can never raise one.
   end Attach;

   ---------------
   -- Base_Pool --
   ---------------

   function Base_Pool
     (Master : Finalization_Master) return Any_Storage_Pool_Ptr
   is
   begin
      return Master.Base_Pool;
   end Base_Pool;

   ------------
   -- Detach --
   ------------

   procedure Detach (N : not null FM_Node_Ptr) is
   begin
      if N.Prev /= null and then N.Next /= null then
         Lock_Task.all;

         N.Prev.Next := N.Next;
         N.Next.Prev := N.Prev;
         N.Prev := null;
         N.Next := null;

         Unlock_Task.all;
      end if;

      --  Note: No need to unlock in case of an exception because the above
      --  code can never raise one.
   end Detach;

   --------------
   -- Finalize --
   --------------

   overriding procedure Finalize (Master : in out Finalization_Master) is
      Curr_Ptr : FM_Node_Ptr;
      Ex_Occur : Exception_Occurrence;
      Obj_Addr : Address;
      Raised   : Boolean := False;

      function Is_Empty_List (L : not null FM_Node_Ptr) return Boolean;
      --  Determine whether a list contains only one element, the dummy head

      -------------------
      -- Is_Empty_List --
      -------------------

      function Is_Empty_List (L : not null FM_Node_Ptr) return Boolean is
      begin
         return L.Next = L and then L.Prev = L;
      end Is_Empty_List;

   --  Start of processing for Finalize

   begin
      --  It is possible for multiple tasks to cause the finalization of the
      --  same master. Let only one task finalize the objects.

      if Master.Finalization_Started then
         return;
      end if;

      --  Lock the master to prevent any allocations while the objects are
      --  being finalized. The master remains locked because either the master
      --  is explicitly deallocated or the associated access type is about to
      --  go out of scope.

      Master.Finalization_Started := True;

      while not Is_Empty_List (Master.Objects'Unchecked_Access) loop
         Curr_Ptr := Master.Objects.Next;

         Detach (Curr_Ptr);

         if Master.Finalize_Address /= null then

            --  Skip the list header in order to offer proper object layout for
            --  finalization and call Finalize_Address.

            Obj_Addr := Curr_Ptr.all'Address + Header_Offset;

            begin
               Master.Finalize_Address (Obj_Addr);

            exception
               when Fin_Occur : others =>
                  if not Raised then
                     Raised := True;
                     Save_Occurrence (Ex_Occur, Fin_Occur);
                  end if;
            end;
         end if;
      end loop;

      --  If the finalization of a particular object failed or Finalize_Address
      --  was not set, reraise the exception now.

      if Raised then
         Reraise_Occurrence (Ex_Occur);
      end if;
   end Finalize;

   -----------------
   -- Header_Size --
   -----------------

   function Header_Size return System.Storage_Elements.Storage_Count is
   begin
      return FM_Node'Size / Storage_Unit;
   end Header_Size;

   -------------------
   -- Header_Offset --
   -------------------

   function Header_Offset return System.Storage_Elements.Storage_Offset is
   begin
      return FM_Node'Size / Storage_Unit;
   end Header_Offset;

   ----------------
   -- Initialize --
   ----------------

   overriding procedure Initialize (Master : in out Finalization_Master) is
   begin
      --  The dummy head must point to itself in both directions

      Master.Objects.Next := Master.Objects'Unchecked_Access;
      Master.Objects.Prev := Master.Objects'Unchecked_Access;
   end Initialize;

   --------
   -- pm --
   --------

   procedure pm (Master : Finalization_Master) is
      Head      : constant FM_Node_Ptr := Master.Objects'Unrestricted_Access;
      Head_Seen : Boolean := False;
      N_Ptr     : FM_Node_Ptr;

   begin
      --  Output the basic contents of a master

      --    Master   : 0x123456789
      --    Base_Pool: null <or> 0x123456789
      --    Fin_Addr : null <or> 0x123456789
      --    Fin_Start: TRUE <or> FALSE

      Put ("Master   : ");
      Put_Line (Address_Image (Master'Address));

      Put ("Base_Pool: ");

      if Master.Base_Pool = null then
         Put_Line (" null");
      else
         Put_Line (Address_Image (Master.Base_Pool'Address));
      end if;

      Put ("Fin_Addr : ");

      if Master.Finalize_Address = null then
         Put_Line ("null");
      else
         Put_Line (Address_Image (Master.Finalize_Address'Address));
      end if;

      Put ("Fin_Start: ");
      Put_Line (Master.Finalization_Started'Img);

      --  Output all chained elements. The format is the following:

      --    ^ <or> ? <or> null
      --    |Header: 0x123456789 (dummy head)
      --    |  Prev: 0x123456789
      --    |  Next: 0x123456789
      --    V

      --  ^ - the current element points back to the correct element
      --  ? - the current element points back to an erroneous element
      --  n - the current element points back to null

      --  Header - the address of the list header
      --  Prev   - the address of the list header which the current element
      --         - points back to
      --  Next   - the address of the list header which the current element
      --         - points to
      --  (dummy head) - present if dummy head

      N_Ptr := Head;
      while N_Ptr /= null loop -- Should never be null; we being defensive
         Put_Line ("V");

         --  We see the head initially; we want to exit when we see the head a
         --  SECOND time.

         if N_Ptr = Head then
            exit when Head_Seen;

            Head_Seen := True;
         end if;

         --  The current element is null. This should never happen since the
         --  list is circular.

         if N_Ptr.Prev = null then
            Put_Line ("null (ERROR)");

         --  The current element points back to the correct element

         elsif N_Ptr.Prev.Next = N_Ptr then
            Put_Line ("^");

         --  The current element points to an erroneous element

         else
            Put_Line ("? (ERROR)");
         end if;

         --  Output the header and fields

         Put ("|Header: ");
         Put (Address_Image (N_Ptr.all'Address));

         --  Detect the dummy head

         if N_Ptr = Head then
            Put_Line (" (dummy head)");
         else
            Put_Line ("");
         end if;

         Put ("|  Prev: ");

         if N_Ptr.Prev = null then
            Put_Line ("null");
         else
            Put_Line (Address_Image (N_Ptr.Prev.all'Address));
         end if;

         Put ("|  Next: ");

         if N_Ptr.Next = null then
            Put_Line ("null");
         else
            Put_Line (Address_Image (N_Ptr.Next.all'Address));
         end if;

         N_Ptr := N_Ptr.Next;
      end loop;
   end pm;

   -------------------
   -- Set_Base_Pool --
   -------------------

   procedure Set_Base_Pool
     (Master   : in out Finalization_Master;
      Pool_Ptr : Any_Storage_Pool_Ptr)
   is
   begin
      Master.Base_Pool := Pool_Ptr;
   end Set_Base_Pool;

   --------------------------
   -- Set_Finalize_Address --
   --------------------------

   procedure Set_Finalize_Address
     (Master       : in out Finalization_Master;
      Fin_Addr_Ptr : Finalize_Address_Ptr)
   is
   begin
      Master.Finalize_Address := Fin_Addr_Ptr;
   end Set_Finalize_Address;

end System.Finalization_Masters;
