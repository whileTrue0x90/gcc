------------------------------------------------------------------------------
--                                                                          --
--                         GNAT COMPILER COMPONENTS                         --
--                                                                          --
--                              G N A T M E M                               --
--                                                                          --
--                                 B o d y                                  --
--                                                                          --
--           Copyright (C) 1997-2004, Ada Core Technologies, Inc.           --
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
-- GNAT was originally developed  by the GNAT team at  New York University. --
-- Extensive contributions were provided by Ada Core Technologies Inc.      --
--                                                                          --
------------------------------------------------------------------------------

--  GNATMEM is a utility that tracks memory leaks. It is based on a simple
--  idea:

--      - Read the allocation log generated by the application linked using
--        instrumented memory allocation and dealocation (see memtrack.adb for
--        this circuitry). To get access to this functionality, the application
--        must be relinked with library libgmem.a:

--            $ gnatmake my_prog -largs -lgmem

--        The running my_prog will produce a file named gmem.out that will be
--        parsed by gnatmem.

--      - Record a reference to the allocated memory on each allocation call.

--      - Suppress this reference on deallocation.

--      - At the end of the program, remaining references are potential leaks.
--        sort them out the best possible way in order to locate the root of
--        the leak.

--   This capability is not supported on all platforms, please refer to
--   memtrack.adb for further information.

--   In order to help finding out the real leaks,  the notion of "allocation
--   root" is defined. An allocation root is a specific point in the program
--   execution generating memory allocation where data is collected (such as
--   number of allocations, amount of memory allocated, high water mark, etc.)

with Gnatvsn; use Gnatvsn;

with Ada.Text_IO;             use Ada.Text_IO;
with Ada.Float_Text_IO;
with Ada.Integer_Text_IO;

with GNAT.Command_Line;       use GNAT.Command_Line;
with GNAT.Heap_Sort_G;
with GNAT.OS_Lib;             use GNAT.OS_Lib;
with GNAT.HTable;             use GNAT.HTable;

with System;                  use System;
with System.Storage_Elements; use System.Storage_Elements;

with Memroot; use Memroot;

procedure Gnatmem is

   ------------------------
   -- Other Declarations --
   ------------------------

   type Storage_Elmt is record
      Elmt : Character;
      --  *  = End of log file
      --  A  = found a ALLOC mark in the log
      --  D  = found a DEALL mark in the log
      Address : Integer_Address;
      Size    : Storage_Count;
   end record;
   --  This needs a comment ???

   Log_Name, Program_Name : String_Access;
   --  These need comments, and should be on separate lines ???

   function Read_Next return Storage_Elmt;
   --  Reads next dynamic storage operation from the log file.

   function Mem_Image (X : Storage_Count) return String;
   --  X is a size in storage_element. Returns a value
   --  in Megabytes, Kilobytes or Bytes as appropriate.

   procedure Process_Arguments;
   --  Read command line arguments

   procedure Usage;
   --  Prints out the option help

   function Gmem_Initialize (Dumpname : String) return Boolean;
   --  Opens the file represented by Dumpname and prepares it for
   --  work. Returns False if the file does not have the correct format, True
   --  otherwise.

   procedure Gmem_A2l_Initialize (Exename : String);
   --  Initialises the convert_addresses interface by supplying it with
   --  the name of the executable file Exename

   -----------------------------------
   -- HTable address --> Allocation --
   -----------------------------------

   type Allocation is record
      Root : Root_Id;
      Size : Storage_Count;
   end record;

   type Address_Range is range 0 .. 4097;
   function H (A : Integer_Address) return Address_Range;
   No_Alloc : constant Allocation := (No_Root_Id, 0);

   package Address_HTable is new GNAT.HTable.Simple_HTable (
     Header_Num => Address_Range,
     Element    => Allocation,
     No_Element => No_Alloc,
     Key        => Integer_Address,
     Hash       => H,
     Equal      => "=");

   BT_Depth   : Integer := 1;

   --  The following need comments ???

   Global_Alloc_Size      : Storage_Count  := 0;
   Global_High_Water_Mark : Storage_Count  := 0;
   Global_Nb_Alloc        : Integer        := 0;
   Global_Nb_Dealloc      : Integer        := 0;
   Nb_Root                : Integer        := 0;
   Nb_Wrong_Deall         : Integer        := 0;
   Minimum_NB_Leaks       : Integer        := 1;

   Tmp_Alloc   : Allocation;
   Quiet_Mode  : Boolean := False;

   ------------------------------
   -- Allocation Roots Sorting --
   ------------------------------

   Sort_Order : String (1 .. 3) := "nwh";
   --  This is the default order in which sorting criteria will be applied
   --  n -  Total number of unfreed allocations
   --  w -  Final watermark
   --  h -  High watermark

   --------------------------------
   -- GMEM functionality binding --
   --------------------------------

   function Gmem_Initialize (Dumpname : String) return Boolean is
      function Initialize (Dumpname : System.Address) return Boolean;
      pragma Import (C, Initialize, "__gnat_gmem_initialize");

      S : aliased String := Dumpname & ASCII.NUL;

   begin
      return Initialize (S'Address);
   end Gmem_Initialize;

   procedure Gmem_A2l_Initialize (Exename : String) is
      procedure A2l_Initialize (Exename : System.Address);
      pragma Import (C, A2l_Initialize, "__gnat_gmem_a2l_initialize");

      S : aliased String := Exename & ASCII.NUL;

   begin
      A2l_Initialize (S'Address);
   end Gmem_A2l_Initialize;

   function Read_Next return Storage_Elmt is
      procedure Read_Next (buf : System.Address);
      pragma Import (C, Read_Next, "__gnat_gmem_read_next");

      S : Storage_Elmt;

   begin
      Read_Next (S'Address);
      return S;
   end Read_Next;

   -------
   -- H --
   -------

   function H (A : Integer_Address) return Address_Range is
   begin
      return Address_Range (A mod Integer_Address (Address_Range'Last));
   end H;

   ---------------
   -- Mem_Image --
   ---------------

   function Mem_Image (X : Storage_Count) return String is
      Ks    : constant Storage_Count := X / 1024;
      Megs  : constant Storage_Count := Ks / 1024;
      Buff  : String (1 .. 7);

   begin
      if Megs /= 0 then
         Ada.Float_Text_IO.Put (Buff, Float (X) / 1024.0 / 1024.0, 2, 0);
         return Buff & " Megabytes";

      elsif Ks /= 0 then
         Ada.Float_Text_IO.Put (Buff, Float (X) / 1024.0, 2, 0);
         return Buff & " Kilobytes";

      else
         Ada.Integer_Text_IO.Put (Buff (1 .. 4), Integer (X));
         return Buff (1 .. 4) & " Bytes";
      end if;
   end Mem_Image;

   -----------
   -- Usage --
   -----------

   procedure Usage is
   begin
      New_Line;
      Put ("GNATMEM ");
      Put_Line (Gnat_Version_String);
      Put_Line ("Copyright 1997-2004 Free Software Foundation, Inc.");
      New_Line;

      Put_Line ("Usage: gnatmem switches [depth] exename");
      New_Line;
      Put_Line ("  depth    backtrace depth to take into account, default is"
                & Integer'Image (BT_Depth));
      Put_Line ("  exename  the name of the executable to be analyzed");
      New_Line;
      Put_Line ("Switches:");
      Put_Line ("  -b n     same as depth parameter");
      Put_Line ("  -i file  read the allocation log from specific file");
      Put_Line ("           default is gmem.out in the current directory");
      Put_Line ("  -m n     masks roots with less than n leaks, default is 1");
      Put_Line ("           specify 0 to see even released allocation roots");
      Put_Line ("  -q       quiet, minimum output");
      Put_Line ("  -s order sort allocation roots according to an order of");
      Put_Line ("           sort criteria");
      GNAT.OS_Lib.OS_Exit (1);
   end Usage;

   -----------------------
   -- Process_Arguments --
   -----------------------

   procedure Process_Arguments is
   begin
      --  Parse the options first

      loop
         case Getopt ("b: m: i: q s:") is
            when ASCII.Nul => exit;

            when 'b' =>
               begin
                  BT_Depth := Natural'Value (Parameter);
               exception
                  when Constraint_Error =>
                     Usage;
               end;

            when 'm' =>
               begin
                  Minimum_NB_Leaks := Natural'Value (Parameter);
               exception
                  when Constraint_Error =>
                     Usage;
               end;

            when 'i' =>
               Log_Name := new String'(Parameter);

            when 'q' =>
               Quiet_Mode := True;

            when 's' =>
               declare
                  S : constant String (Sort_Order'Range) := Parameter;

               begin
                  for J in Sort_Order'Range loop
                     if S (J) = 'n' or else
                        S (J) = 'w' or else
                        S (J) = 'h'
                     then
                        Sort_Order (J) := S (J);
                     else
                        Put_Line ("Invalid sort criteria string.");
                        GNAT.OS_Lib.OS_Exit (1);
                     end if;
                  end loop;
               end;

            when others =>
               null;
         end case;
      end loop;

      --  Set default log file if -i hasn't been specified

      if Log_Name = null then
         Log_Name := new String'("gmem.out");
      end if;

      --  Get the optional backtrace length and program name

      declare
         Str1 : constant String := GNAT.Command_Line.Get_Argument;
         Str2 : constant String := GNAT.Command_Line.Get_Argument;

      begin
         if Str1 = "" then
            Usage;
         end if;

         if Str2 = "" then
            Program_Name := new String'(Str1);
         else
            BT_Depth := Natural'Value (Str1);
            Program_Name := new String'(Str2);
         end if;

      exception
         when Constraint_Error =>
            Usage;
      end;

      --  Ensure presence of executable suffix in Program_Name

      declare
         Suffix : String_Access := Get_Executable_Suffix;
         Tmp    : String_Access;

      begin
         if Suffix.all /= ""
           and then
             Program_Name.all
              (Program_Name.all'Last - Suffix.all'Length + 1 ..
                               Program_Name.all'Last) /= Suffix.all
         then
            Tmp := new String'(Program_Name.all & Suffix.all);
            Free (Program_Name);
            Program_Name := Tmp;
         end if;

         Free (Suffix);

         --  Search the executable on the path. If not found in the PATH, we
         --  default to the current directory. Otherwise, libaddr2line will
         --  fail with an error:

         --     (null): Bad address

         Tmp := Locate_Exec_On_Path (Program_Name.all);

         if Tmp = null then
            Tmp := new String'('.' & Directory_Separator & Program_Name.all);
         end if;

         Free (Program_Name);
         Program_Name := Tmp;
      end;

      if not Is_Regular_File (Log_Name.all) then
         Put_Line ("Couldn't find " & Log_Name.all);
         GNAT.OS_Lib.OS_Exit (1);
      end if;

      if not Gmem_Initialize (Log_Name.all) then
         Put_Line ("File " & Log_Name.all & " is not a gnatmem log file");
         GNAT.OS_Lib.OS_Exit (1);
      end if;

      if not Is_Regular_File (Program_Name.all) then
         Put_Line ("Couldn't find " & Program_Name.all);
      end if;

      Gmem_A2l_Initialize (Program_Name.all);

   exception
      when GNAT.Command_Line.Invalid_Switch =>
         Ada.Text_IO.Put_Line ("Invalid switch : "
                               & GNAT.Command_Line.Full_Switch);
         Usage;
   end Process_Arguments;

   Cur_Elmt : Storage_Elmt;

--  Start of processing for Gnatmem

begin
   Process_Arguments;

   --  Main loop analysing the data generated by the instrumented routines.
   --  For each allocation, the backtrace is kept and stored in a htable
   --  whose entry is the address. For each deallocation, we look for the
   --  corresponding allocation and cancel it.

   Main : loop
      Cur_Elmt := Read_Next;

      case Cur_Elmt.Elmt is
         when '*' =>
            exit Main;

         when 'A' =>

            --  Update global counters if the allocated size is meaningful

            if Quiet_Mode then
               Tmp_Alloc.Root := Read_BT (BT_Depth);

               if Nb_Alloc (Tmp_Alloc.Root) = 0 then
                  Nb_Root := Nb_Root + 1;
               end if;

               Set_Nb_Alloc (Tmp_Alloc.Root, Nb_Alloc (Tmp_Alloc.Root) + 1);
               Address_HTable.Set (Cur_Elmt.Address, Tmp_Alloc);

            elsif Cur_Elmt.Size > 0 then

               Global_Alloc_Size := Global_Alloc_Size + Cur_Elmt.Size;
               Global_Nb_Alloc   := Global_Nb_Alloc + 1;

               if Global_High_Water_Mark < Global_Alloc_Size then
                  Global_High_Water_Mark := Global_Alloc_Size;
               end if;

               --  Read the corresponding back trace

               Tmp_Alloc.Root := Read_BT (BT_Depth);

               --  Update the number of allocation root if this is a new one

               if Nb_Alloc (Tmp_Alloc.Root) = 0 then
                  Nb_Root := Nb_Root + 1;
               end if;

               --  Update allocation root specific counters

               Set_Alloc_Size (Tmp_Alloc.Root,
                 Alloc_Size (Tmp_Alloc.Root) + Cur_Elmt.Size);

               Set_Nb_Alloc (Tmp_Alloc.Root, Nb_Alloc (Tmp_Alloc.Root) + 1);

               if High_Water_Mark (Tmp_Alloc.Root) <
                                               Alloc_Size (Tmp_Alloc.Root)
               then
                  Set_High_Water_Mark (Tmp_Alloc.Root,
                    Alloc_Size (Tmp_Alloc.Root));
               end if;

               --  Associate this allocation root to the allocated address

               Tmp_Alloc.Size := Cur_Elmt.Size;
               Address_HTable.Set (Cur_Elmt.Address, Tmp_Alloc);

            --  non meaningful output, just consumes the backtrace

            else
               Tmp_Alloc.Root := Read_BT (BT_Depth);
            end if;

         when 'D' =>

            --  Get the corresponding Dealloc_Size and Root

            Tmp_Alloc := Address_HTable.Get (Cur_Elmt.Address);

            if Tmp_Alloc.Root = No_Root_Id then

               --  There was no prior allocation at this address, something is
               --  very wrong. Mark this allocation root as problematic

               Tmp_Alloc.Root := Read_BT (BT_Depth);

               if Nb_Alloc (Tmp_Alloc.Root) = 0 then
                  Set_Nb_Alloc (Tmp_Alloc.Root, Nb_Alloc (Tmp_Alloc.Root) - 1);
                  Nb_Wrong_Deall := Nb_Wrong_Deall + 1;
               end if;

            else
               --  Update global counters

               if not Quiet_Mode then
                  Global_Alloc_Size := Global_Alloc_Size - Tmp_Alloc.Size;
               end if;

               Global_Nb_Dealloc   := Global_Nb_Dealloc + 1;

               --  Update allocation root specific counters

               if not Quiet_Mode then
                  Set_Alloc_Size (Tmp_Alloc.Root,
                    Alloc_Size (Tmp_Alloc.Root) - Tmp_Alloc.Size);
               end if;

               Set_Nb_Alloc (Tmp_Alloc.Root, Nb_Alloc (Tmp_Alloc.Root) - 1);

               --  update the number of allocation root if this one disappear

               if Nb_Alloc (Tmp_Alloc.Root) = 0
                 and then Minimum_NB_Leaks > 0 then
                  Nb_Root := Nb_Root - 1;
               end if;

               --  De-associate the deallocated address

               Address_HTable.Remove (Cur_Elmt.Address);
            end if;

         when others =>
            raise Program_Error;
      end case;
   end loop Main;

   --  Print out general information about overall allocation

   if not Quiet_Mode then
      Put_Line ("Global information");
      Put_Line ("------------------");

      Put      ("   Total number of allocations        :");
      Ada.Integer_Text_IO.Put (Global_Nb_Alloc, 4);
      New_Line;

      Put      ("   Total number of deallocations      :");
      Ada.Integer_Text_IO.Put (Global_Nb_Dealloc, 4);
      New_Line;

      Put_Line ("   Final Water Mark (non freed mem)   :"
        & Mem_Image (Global_Alloc_Size));
      Put_Line ("   High Water Mark                    :"
        & Mem_Image (Global_High_Water_Mark));
      New_Line;
   end if;

   --  Print out the back traces corresponding to potential leaks in order
   --  greatest number of non-deallocated allocations

   Print_Back_Traces : declare
      type Root_Array is array (Natural range <>) of Root_Id;
      Leaks   : Root_Array (0 .. Nb_Root);
      Leak_Index   : Natural := 0;

      Bogus_Dealls : Root_Array (1 .. Nb_Wrong_Deall);
      Deall_Index  : Natural := 0;
      Nb_Alloc_J   : Natural := 0;

      procedure Move (From : Natural; To : Natural);
      function  Lt (Op1, Op2 : Natural) return Boolean;
      package   Root_Sort is new GNAT.Heap_Sort_G (Move, Lt);

      procedure Move (From : Natural; To : Natural) is
      begin
         Leaks (To) := Leaks (From);
      end Move;

      function Lt (Op1, Op2 : Natural) return Boolean is
         function Apply_Sort_Criterion (S : Character) return Integer;
         --  Applies a specific sort criterion; returns -1, 0 or 1 if Op1 is
         --  smaller than, equal, or greater than Op2 according to criterion

         function Apply_Sort_Criterion (S : Character) return Integer is
            LOp1, LOp2 : Integer;
         begin
            case S is
               when 'n' =>
                  LOp1 := Nb_Alloc (Leaks (Op1));
                  LOp2 := Nb_Alloc (Leaks (Op2));

               when 'w' =>
                  LOp1 := Integer (Alloc_Size (Leaks (Op1)));
                  LOp2 := Integer (Alloc_Size (Leaks (Op2)));

               when 'h' =>
                  LOp1 := Integer (High_Water_Mark (Leaks (Op1)));
                  LOp2 := Integer (High_Water_Mark (Leaks (Op2)));

               when others =>
                  return 0;  --  Can't actually happen
            end case;

            if LOp1 < LOp2 then
               return -1;
            elsif LOp1 > LOp2 then
               return 1;
            else
               return 0;
            end if;
         exception
            when Constraint_Error =>
               return 0;
         end Apply_Sort_Criterion;

         Result : Integer;

      --  Start of processing for Lt

      begin
         for S in Sort_Order'Range loop
            Result := Apply_Sort_Criterion (Sort_Order (S));
            if Result = -1 then
               return False;
            elsif Result = 1 then
               return True;
            end if;
         end loop;
         return False;
      end Lt;

   --  Start of processing for Print_Back_Traces

   begin
      --  Transfer all the relevant Roots in the Leaks and a
      --  Bogus_Deall arrays

      Tmp_Alloc.Root := Get_First;
      while Tmp_Alloc.Root /= No_Root_Id loop
         if Nb_Alloc (Tmp_Alloc.Root) = 0 and then Minimum_NB_Leaks > 0 then
            null;

         elsif Nb_Alloc (Tmp_Alloc.Root) < 0  then
            Deall_Index := Deall_Index + 1;
            Bogus_Dealls (Deall_Index) := Tmp_Alloc.Root;

         else
            Leak_Index := Leak_Index + 1;
            Leaks (Leak_Index) := Tmp_Alloc.Root;
         end if;

         Tmp_Alloc.Root := Get_Next;
      end loop;

      --  Print out wrong deallocations

      if Nb_Wrong_Deall > 0 then
         Put_Line    ("Releasing deallocated memory at :");
         if not Quiet_Mode then
            Put_Line ("--------------------------------");
         end if;

         for J in  1 .. Bogus_Dealls'Last loop
            Print_BT (Bogus_Dealls (J), Short => Quiet_Mode);
            New_Line;
         end loop;
      end if;

      --  Print out all allocation Leaks

      if Nb_Root > 0 then

         --  Sort the Leaks so that potentially important leaks appear first

         Root_Sort.Sort (Nb_Root);

         for J in  1 .. Leaks'Last loop
            Nb_Alloc_J := Nb_Alloc (Leaks (J));
            if Nb_Alloc_J >= Minimum_NB_Leaks then
               if Quiet_Mode then
                  if Nb_Alloc_J = 1 then
                     Put_Line (" 1 leak at :");
                  else
                     Put_Line (Integer'Image (Nb_Alloc_J) & " leaks at :");
                  end if;

               else
                  Put_Line ("Allocation Root #" & Integer'Image (J));
                  Put_Line ("-------------------");

                  Put      (" Number of non freed allocations    :");
                  Ada.Integer_Text_IO.Put (Nb_Alloc_J, 4);
                  New_Line;

                  Put_Line
                    (" Final Water Mark (non freed mem)   :"
                     & Mem_Image (Alloc_Size (Leaks (J))));

                  Put_Line
                    (" High Water Mark                    :"
                     & Mem_Image (High_Water_Mark (Leaks (J))));

                  Put_Line (" Backtrace                          :");
               end if;

               Print_BT (Leaks (J), Short => Quiet_Mode);
               New_Line;
            end if;
         end loop;
      end if;
   end Print_Back_Traces;
end Gnatmem;
