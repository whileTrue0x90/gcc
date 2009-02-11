------------------------------------------------------------------------------
--                                                                          --
--                         GNAT COMPILER COMPONENTS                         --
--                                                                          --
--                     ADA.EXCEPTIONS.STREAM_ATTRIBUTES                     --
--                                                                          --
--                                 B o d y                                  --
--                                                                          --
--          Copyright (C) 1992-2005, Free Software Foundation, Inc.         --
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
-- Extensive contributions were provided by Ada Core Technologies Inc.      --
--                                                                          --
------------------------------------------------------------------------------

pragma Warnings (Off);
--  Allow withing of non-Preelaborated units in Ada 2005 mode where this
--  package will be categorized as Preelaborate. See AI-362 for details.
--  It is safe in the context of the run-time to violate the rules!

with System.Exception_Table;  use System.Exception_Table;
with System.Storage_Elements; use System.Storage_Elements;

pragma Warnings (On);

separate (Ada.Exceptions)
package body Stream_Attributes is

   -------------------
   -- EId_To_String --
   -------------------

   function EId_To_String (X : Exception_Id) return String is
   begin
      if X = Null_Id then
         return "";
      else
         return Exception_Name (X);
      end if;
   end EId_To_String;

   ------------------
   -- EO_To_String --
   ------------------

   --  We use the null string to represent the null occurrence, otherwise
   --  we output the Exception_Information string for the occurrence.

   function EO_To_String (X : Exception_Occurrence) return String is
   begin
      if X.Id = Null_Id then
         return "";
      else
         return Exception_Information (X);
      end if;
   end EO_To_String;

   -------------------
   -- String_To_EId --
   -------------------

   function String_To_EId (S : String) return Exception_Id is
   begin
      if S = "" then
         return Null_Id;
      else
         return Exception_Id (Internal_Exception (S));
      end if;
   end String_To_EId;

   ------------------
   -- String_To_EO --
   ------------------

   function String_To_EO (S : String) return Exception_Occurrence is
      From : Natural;
      To   : Integer;

      X    : aliased Exception_Occurrence;
      --  This is the exception occurrence we will create

      procedure Bad_EO;
      pragma No_Return (Bad_EO);
      --  Signal bad exception occurrence string

      procedure Next_String;
      --  On entry, To points to last character of previous line of the
      --  message, terminated by LF. On return, From .. To are set to
      --  specify the next string, or From > To if there are no more lines.

      procedure Bad_EO is
      begin
         Raise_Exception
           (Program_Error'Identity,
            "bad exception occurrence in stream input");

         --  The following junk raise of Program_Error is required because
         --  this is a No_Return function, and unfortunately Raise_Exception
         --  can return (this particular call can't, but the back end is not
         --  clever enough to know that).

         raise Program_Error;
      end Bad_EO;

      procedure Next_String is
      begin
         From := To + 2;

         if From < S'Last then
            To := From + 1;

            while To < S'Last - 1 loop
               if To >= S'Last then
                  Bad_EO;
               elsif S (To + 1) = ASCII.LF then
                  exit;
               else
                  To := To + 1;
               end if;
            end loop;
         end if;
      end Next_String;

   --  Start of processing for String_To_EO

   begin
      if S = "" then
         return Null_Occurrence;

      else
         X.Cleanup_Flag := False;

         To := S'First - 2;
         Next_String;

         if S (From .. From + 15) /= "Exception name: " then
            Bad_EO;
         end if;

         X.Id := Exception_Id (Internal_Exception (S (From + 16 .. To)));

         Next_String;

         if From <= To and then S (From) = 'M' then
            if S (From .. From + 8) /= "Message: " then
               Bad_EO;
            end if;

            X.Msg_Length := To - From - 8;
            X.Msg (1 .. X.Msg_Length) := S (From + 9 .. To);
            Next_String;

         else
            X.Msg_Length := 0;
         end if;

         X.Pid := 0;

         if From <= To and then S (From) = 'P' then
            if S (From .. From + 3) /= "PID:" then
               Bad_EO;
            end if;

            From := From + 5; -- skip past PID: space

            while From <= To loop
               X.Pid := X.Pid * 10 +
                          (Character'Pos (S (From)) - Character'Pos ('0'));
               From := From + 1;
            end loop;

            Next_String;
         end if;

         X.Num_Tracebacks := 0;

         if From <= To then
            if S (From .. To) /= "Call stack traceback locations:" then
               Bad_EO;
            end if;

            Next_String;
            loop
               exit when From > To;

               declare
                  Ch : Character;
                  C  : Integer_Address;
                  N  : Integer_Address;

               begin
                  if S (From) /= '0'
                    or else S (From + 1) /= 'x'
                  then
                     Bad_EO;
                  else
                     From := From + 2;
                  end if;

                  C := 0;
                  while From <= To loop
                     Ch := S (From);

                     if Ch in '0' .. '9' then
                        N :=
                          Character'Pos (S (From)) - Character'Pos ('0');

                     elsif Ch in 'a' .. 'f' then
                        N :=
                          Character'Pos (S (From)) - Character'Pos ('a') + 10;

                     elsif Ch = ' ' then
                        From := From + 1;
                        exit;

                     else
                        Bad_EO;
                     end if;

                     C := C * 16 + N;

                     From := From + 1;
                  end loop;

                  if X.Num_Tracebacks = Max_Tracebacks then
                     Bad_EO;
                  end if;

                  X.Num_Tracebacks := X.Num_Tracebacks + 1;
                  X.Tracebacks (X.Num_Tracebacks) :=
                    TBE.TB_Entry_For (To_Address (C));
               end;
            end loop;
         end if;

         --  If an exception was converted to a string, it must have
         --  already been raised, so flag it accordingly and we are done.

         X.Exception_Raised := True;
         return X;
      end if;
   end String_To_EO;

end Stream_Attributes;
