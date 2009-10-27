------------------------------------------------------------------------------
--                                                                          --
--                         GNAT RUN-TIME COMPONENTS                         --
--                                                                          --
--                     S Y S T E M . I M G _ W C H A R                      --
--                                                                          --
--                                 B o d y                                  --
--                                                                          --
--          Copyright (C) 1992-2007, Free Software Foundation, Inc.         --
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

with Interfaces; use Interfaces;

with System.Img_Char; use System.Img_Char;

package body System.Img_WChar is

   --------------------------
   -- Image_Wide_Character --
   --------------------------

   procedure Image_Wide_Character
     (V        : Wide_Character;
      S        : in out String;
      P        : out Natural;
      Ada_2005 : Boolean)
   is
      pragma Assert (S'First = 1);

   begin
      --  Annoying Ada 95 incompatibility with FFFE/FFFF

      if V >= Wide_Character'Val (16#FFFE#)
        and then not Ada_2005
      then
         if V = Wide_Character'Val (16#FFFE#) then
            S (1 .. 4) := "FFFE";
         else
            S (1 .. 4) := "FFFF";
         end if;

         P := 4;

      --  Normal case, same as Wide_Wide_Character

      else
         Image_Wide_Wide_Character
           (Wide_Wide_Character'Val (Wide_Character'Pos (V)), S, P);
      end if;
   end Image_Wide_Character;

   -------------------------------
   -- Image_Wide_Wide_Character --
   -------------------------------

   procedure Image_Wide_Wide_Character
     (V : Wide_Wide_Character;
      S : in out String;
      P : out Natural)
   is
      pragma Assert (S'First = 1);

      Val : Unsigned_32 := Wide_Wide_Character'Pos (V);

   begin
      --  If in range of standard Character, use Character routine

      if Val <= 16#FF# then
         Image_Character (Character'Val (Wide_Wide_Character'Pos (V)), S, P);

      --  Otherwise value returned is Hex_hhhhhhhh

      else
         declare
            Hex : constant array (Unsigned_32 range 0 .. 15) of Character :=
                    "0123456789ABCDEF";

         begin
            S (1 .. 4) := "Hex_";

            for J in reverse 5 .. 12 loop
               S (J) := Hex (Val mod 16);
               Val := Val / 16;
            end loop;

            P := 12;
         end;
      end if;
   end Image_Wide_Wide_Character;

end System.Img_WChar;
