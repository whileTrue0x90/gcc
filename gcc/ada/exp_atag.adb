------------------------------------------------------------------------------
--                                                                          --
--                         GNAT COMPILER COMPONENTS                         --
--                                                                          --
--                             E X P _ A T A G                              --
--                                                                          --
--                                 S p e c                                  --
--                                                                          --
--          Copyright (C) 2006-2008, Free Software Foundation, Inc.         --
--                                                                          --
-- GNAT is free software;  you can  redistribute it  and/or modify it under --
-- terms of the  GNU General Public License as published  by the Free Soft- --
-- ware  Foundation;  either version 3,  or (at your option) any later ver- --
-- sion.  GNAT is distributed in the hope that it will be useful, but WITH- --
-- OUT ANY WARRANTY;  without even the  implied warranty of MERCHANTABILITY --
-- or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License --
-- for  more details.  You should have  received  a copy of the GNU General --
-- Public License  distributed with GNAT; see file COPYING3.  If not, go to --
-- http://www.gnu.org/licenses for a complete copy of the license.          --
--                                                                          --
-- GNAT was originally developed  by the GNAT team at  New York University. --
-- Extensive contributions were provided by Ada Core Technologies Inc.      --
--                                                                          --
------------------------------------------------------------------------------

with Einfo;    use Einfo;
with Elists;   use Elists;
with Exp_Util; use Exp_Util;
with Namet;    use Namet;
with Nlists;   use Nlists;
with Nmake;    use Nmake;
with Rtsfind;  use Rtsfind;
with Sinfo;    use Sinfo;
with Sem_Aux;  use Sem_Aux;
with Sem_Util; use Sem_Util;
with Stand;    use Stand;
with Snames;   use Snames;
with Tbuild;   use Tbuild;

package body Exp_Atag is

   -----------------------
   -- Local Subprograms --
   -----------------------

   function Build_DT
     (Loc      : Source_Ptr;
      Tag_Node : Node_Id) return Node_Id;
   --  Build code that displaces the Tag to reference the base of the wrapper
   --  record
   --
   --  Generates:
   --    To_Dispatch_Table_Ptr
   --      (To_Address (Tag_Node) - Tag_Node.Prims_Ptr'Position);

   function Build_TSD (Loc : Source_Ptr; Tag_Node : Node_Id) return Node_Id;
   --  Build code that retrieves the address of the record containing the Type
   --  Specific Data generated by GNAT.
   --
   --  Generate: To_Type_Specific_Data_Ptr
   --              (To_Addr_Ptr (To_Address (Tag) - Typeinfo_Offset).all);

   ------------------------------------------------
   -- Build_Common_Dispatching_Select_Statements --
   ------------------------------------------------

   procedure Build_Common_Dispatching_Select_Statements
     (Loc    : Source_Ptr;
      DT_Ptr : Entity_Id;
      Stmts  : List_Id)
   is
   begin
      --  Generate:
      --    C := get_prim_op_kind (tag! (<type>VP), S);

      --  where C is the out parameter capturing the call kind and S is the
      --  dispatch table slot number.

      Append_To (Stmts,
        Make_Assignment_Statement (Loc,
          Name =>
            Make_Identifier (Loc, Name_uC),
          Expression =>
            Make_Function_Call (Loc,
              Name => New_Occurrence_Of (RTE (RE_Get_Prim_Op_Kind), Loc),
              Parameter_Associations => New_List (
                Unchecked_Convert_To (RTE (RE_Tag),
                  New_Reference_To (DT_Ptr, Loc)),
                Make_Identifier (Loc, Name_uS)))));

      --  Generate:

      --    if C = POK_Procedure
      --      or else C = POK_Protected_Procedure
      --      or else C = POK_Task_Procedure;
      --    then
      --       F := True;
      --       return;

      --  where F is the out parameter capturing the status of a potential
      --  entry call.

      Append_To (Stmts,
        Make_If_Statement (Loc,

          Condition =>
            Make_Or_Else (Loc,
              Left_Opnd =>
                Make_Op_Eq (Loc,
                  Left_Opnd =>
                    Make_Identifier (Loc, Name_uC),
                  Right_Opnd =>
                    New_Reference_To (RTE (RE_POK_Procedure), Loc)),
              Right_Opnd =>
                Make_Or_Else (Loc,
                  Left_Opnd =>
                    Make_Op_Eq (Loc,
                      Left_Opnd =>
                        Make_Identifier (Loc, Name_uC),
                      Right_Opnd =>
                        New_Reference_To (RTE (
                          RE_POK_Protected_Procedure), Loc)),
                  Right_Opnd =>
                    Make_Op_Eq (Loc,
                      Left_Opnd =>
                        Make_Identifier (Loc, Name_uC),
                      Right_Opnd =>
                        New_Reference_To (RTE (
                          RE_POK_Task_Procedure), Loc)))),

          Then_Statements =>
            New_List (
              Make_Assignment_Statement (Loc,
                Name       => Make_Identifier (Loc, Name_uF),
                Expression => New_Reference_To (Standard_True, Loc)),
              Make_Simple_Return_Statement (Loc))));
   end Build_Common_Dispatching_Select_Statements;

   -------------------------
   -- Build_CW_Membership --
   -------------------------

   function Build_CW_Membership
     (Loc          : Source_Ptr;
      Obj_Tag_Node : Node_Id;
      Typ_Tag_Node : Node_Id) return Node_Id
   is
      function Build_Pos return Node_Id;
      --  Generate TSD (Obj_Tag).Idepth - TSD (Typ_Tag).Idepth;

      function Build_Pos return Node_Id is
      begin
         return
            Make_Op_Subtract (Loc,
              Left_Opnd =>
                Make_Selected_Component (Loc,
                  Prefix => Build_TSD (Loc, Duplicate_Subexpr (Obj_Tag_Node)),
                  Selector_Name =>
                    New_Reference_To (RTE_Record_Component (RE_Idepth), Loc)),

              Right_Opnd =>
                Make_Selected_Component (Loc,
                  Prefix => Build_TSD (Loc, Duplicate_Subexpr (Typ_Tag_Node)),
                  Selector_Name =>
                    New_Reference_To (RTE_Record_Component (RE_Idepth), Loc)));
      end Build_Pos;

   --  Start of processing for Build_CW_Membership

   begin
      return
        Make_And_Then (Loc,
          Left_Opnd =>
            Make_Op_Ge (Loc,
              Left_Opnd  => Build_Pos,
              Right_Opnd => Make_Integer_Literal (Loc, Uint_0)),

          Right_Opnd =>
            Make_Op_Eq (Loc,
              Left_Opnd =>
                Make_Indexed_Component (Loc,
                  Prefix =>
                    Make_Selected_Component (Loc,
                      Prefix => Build_TSD (Loc, Obj_Tag_Node),
                      Selector_Name =>
                        New_Reference_To
                          (RTE_Record_Component (RE_Tags_Table), Loc)),
                  Expressions =>
                    New_List (Build_Pos)),

              Right_Opnd => Typ_Tag_Node));
   end Build_CW_Membership;

   --------------
   -- Build_DT --
   --------------

   function Build_DT
     (Loc      : Source_Ptr;
      Tag_Node : Node_Id) return Node_Id is
   begin
      return
        Make_Function_Call (Loc,
          Name => New_Reference_To (RTE (RE_DT), Loc),
          Parameter_Associations => New_List (
            Unchecked_Convert_To (RTE (RE_Tag), Tag_Node)));
   end Build_DT;

   ----------------------------
   -- Build_Get_Access_Level --
   ----------------------------

   function Build_Get_Access_Level
     (Loc      : Source_Ptr;
      Tag_Node : Node_Id) return Node_Id
   is
   begin
      return
        Make_Selected_Component (Loc,
          Prefix => Build_TSD (Loc, Tag_Node),
          Selector_Name =>
            New_Reference_To
              (RTE_Record_Component (RE_Access_Level), Loc));
   end Build_Get_Access_Level;

   ------------------------------------------
   -- Build_Get_Predefined_Prim_Op_Address --
   ------------------------------------------

   function Build_Get_Predefined_Prim_Op_Address
     (Loc      : Source_Ptr;
      Tag_Node : Node_Id;
      Position : Uint) return Node_Id
   is
   begin
      --  Build code that retrieves the address of the dispatch table
      --  containing the predefined Ada primitives:
      --
      --  Generate:
      --    To_Predef_Prims_Table_Ptr
      --     (To_Addr_Ptr (To_Address (Tag) - Predef_Prims_Offset).all);

      return
        Make_Indexed_Component (Loc,
          Prefix =>
            Unchecked_Convert_To (RTE (RE_Predef_Prims_Table_Ptr),
              Make_Explicit_Dereference (Loc,
                Unchecked_Convert_To (RTE (RE_Addr_Ptr),
                  Make_Function_Call (Loc,
                    Name =>
                      Make_Expanded_Name (Loc,
                        Chars => Name_Op_Subtract,
                        Prefix =>
                          New_Reference_To
                            (RTU_Entity (System_Storage_Elements), Loc),
                        Selector_Name =>
                          Make_Identifier (Loc,
                            Chars => Name_Op_Subtract)),
                    Parameter_Associations => New_List (
                      Unchecked_Convert_To (RTE (RE_Address), Tag_Node),
                      New_Reference_To (RTE (RE_DT_Predef_Prims_Offset),
                                        Loc)))))),
          Expressions =>
            New_List (Make_Integer_Literal (Loc, Position)));
   end Build_Get_Predefined_Prim_Op_Address;

   -------------------------
   -- Build_Inherit_Prims --
   -------------------------

   function Build_Inherit_Prims
     (Loc          : Source_Ptr;
      Typ          : Entity_Id;
      Old_Tag_Node : Node_Id;
      New_Tag_Node : Node_Id;
      Num_Prims    : Nat) return Node_Id
   is
   begin
      if RTE_Available (RE_DT) then
         return
           Make_Assignment_Statement (Loc,
             Name =>
               Make_Slice (Loc,
                 Prefix =>
                   Make_Selected_Component (Loc,
                     Prefix =>
                       Build_DT (Loc, New_Tag_Node),
                     Selector_Name =>
                       New_Reference_To
                         (RTE_Record_Component (RE_Prims_Ptr), Loc)),
                 Discrete_Range =>
                   Make_Range (Loc,
                   Low_Bound  => Make_Integer_Literal (Loc, 1),
                   High_Bound => Make_Integer_Literal (Loc, Num_Prims))),

             Expression =>
               Make_Slice (Loc,
                 Prefix =>
                   Make_Selected_Component (Loc,
                     Prefix =>
                       Build_DT (Loc, Old_Tag_Node),
                     Selector_Name =>
                       New_Reference_To
                         (RTE_Record_Component (RE_Prims_Ptr), Loc)),
                 Discrete_Range =>
                   Make_Range (Loc,
                     Low_Bound  => Make_Integer_Literal (Loc, 1),
                     High_Bound => Make_Integer_Literal (Loc, Num_Prims))));
      else
         return
           Make_Assignment_Statement (Loc,
             Name =>
               Make_Slice (Loc,
                 Prefix =>
                   Unchecked_Convert_To
                     (Node (Last_Elmt (Access_Disp_Table (Typ))),
                      New_Tag_Node),
                 Discrete_Range =>
                   Make_Range (Loc,
                   Low_Bound  => Make_Integer_Literal (Loc, 1),
                   High_Bound => Make_Integer_Literal (Loc, Num_Prims))),

             Expression =>
               Make_Slice (Loc,
                 Prefix =>
                   Unchecked_Convert_To
                     (Node (Last_Elmt (Access_Disp_Table (Typ))),
                      Old_Tag_Node),
                 Discrete_Range =>
                   Make_Range (Loc,
                     Low_Bound  => Make_Integer_Literal (Loc, 1),
                     High_Bound => Make_Integer_Literal (Loc, Num_Prims))));
      end if;
   end Build_Inherit_Prims;

   -------------------------------
   -- Build_Get_Prim_Op_Address --
   -------------------------------

   function Build_Get_Prim_Op_Address
     (Loc      : Source_Ptr;
      Typ      : Entity_Id;
      Tag_Node : Node_Id;
      Position : Uint) return Node_Id
   is
   begin
      pragma Assert
        (Position <= DT_Entry_Count (First_Tag_Component (Typ)));

      --  At the end of the Access_Disp_Table list we have the type
      --  declaration required to convert the tag into a pointer to
      --  the prims_ptr table (see Freeze_Record_Type).

      return
        Make_Indexed_Component (Loc,
          Prefix =>
            Unchecked_Convert_To
              (Node (Last_Elmt (Access_Disp_Table (Typ))), Tag_Node),
          Expressions => New_List (Make_Integer_Literal (Loc, Position)));
   end Build_Get_Prim_Op_Address;

   -----------------------------
   -- Build_Get_Transportable --
   -----------------------------

   function Build_Get_Transportable
     (Loc      : Source_Ptr;
      Tag_Node : Node_Id) return Node_Id
   is
   begin
      return
        Make_Selected_Component (Loc,
          Prefix => Build_TSD (Loc, Tag_Node),
          Selector_Name =>
            New_Reference_To
              (RTE_Record_Component (RE_Transportable), Loc));
   end Build_Get_Transportable;

   ------------------------------------
   -- Build_Inherit_Predefined_Prims --
   ------------------------------------

   function Build_Inherit_Predefined_Prims
     (Loc          : Source_Ptr;
      Old_Tag_Node : Node_Id;
      New_Tag_Node : Node_Id) return Node_Id
   is
   begin
      return
        Make_Assignment_Statement (Loc,
          Name =>
            Make_Slice (Loc,
              Prefix =>
                Make_Explicit_Dereference (Loc,
                  Unchecked_Convert_To (RTE (RE_Predef_Prims_Table_Ptr),
                    Make_Explicit_Dereference (Loc,
                      Unchecked_Convert_To (RTE (RE_Addr_Ptr),
                        New_Tag_Node)))),
              Discrete_Range => Make_Range (Loc,
                Make_Integer_Literal (Loc, Uint_1),
                New_Reference_To (RTE (RE_Max_Predef_Prims), Loc))),

          Expression =>
            Make_Slice (Loc,
              Prefix =>
                Make_Explicit_Dereference (Loc,
                  Unchecked_Convert_To (RTE (RE_Predef_Prims_Table_Ptr),
                    Make_Explicit_Dereference (Loc,
                      Unchecked_Convert_To (RTE (RE_Addr_Ptr),
                        Old_Tag_Node)))),
              Discrete_Range =>
                Make_Range (Loc,
                  Make_Integer_Literal (Loc, 1),
                  New_Reference_To (RTE (RE_Max_Predef_Prims), Loc))));
   end Build_Inherit_Predefined_Prims;

   -------------------------
   -- Build_Offset_To_Top --
   -------------------------

   function Build_Offset_To_Top
     (Loc       : Source_Ptr;
      This_Node : Node_Id) return Node_Id
   is
      Tag_Node : Node_Id;

   begin
      Tag_Node :=
        Make_Explicit_Dereference (Loc,
          Unchecked_Convert_To (RTE (RE_Tag_Ptr), This_Node));

      return
        Make_Explicit_Dereference (Loc,
          Unchecked_Convert_To (RTE (RE_Offset_To_Top_Ptr),
            Make_Function_Call (Loc,
              Name =>
                Make_Expanded_Name (Loc,
                  Chars => Name_Op_Subtract,
                  Prefix => New_Reference_To
                             (RTU_Entity (System_Storage_Elements), Loc),
                  Selector_Name => Make_Identifier (Loc,
                                     Chars => Name_Op_Subtract)),
              Parameter_Associations => New_List (
                Unchecked_Convert_To (RTE (RE_Address), Tag_Node),
                New_Reference_To (RTE (RE_DT_Offset_To_Top_Offset),
                                  Loc)))));
   end Build_Offset_To_Top;

   ------------------------------------------
   -- Build_Set_Predefined_Prim_Op_Address --
   ------------------------------------------

   function Build_Set_Predefined_Prim_Op_Address
     (Loc          : Source_Ptr;
      Tag_Node     : Node_Id;
      Position     : Uint;
      Address_Node : Node_Id) return Node_Id
   is
   begin
      return
         Make_Assignment_Statement (Loc,
           Name =>
             Make_Indexed_Component (Loc,
               Prefix =>
                 Unchecked_Convert_To (RTE (RE_Predef_Prims_Table_Ptr),
                   Make_Explicit_Dereference (Loc,
                     Unchecked_Convert_To (RTE (RE_Addr_Ptr), Tag_Node))),
               Expressions =>
                 New_List (Make_Integer_Literal (Loc, Position))),

           Expression => Address_Node);
   end Build_Set_Predefined_Prim_Op_Address;

   -------------------------------
   -- Build_Set_Prim_Op_Address --
   -------------------------------

   function Build_Set_Prim_Op_Address
     (Loc          : Source_Ptr;
      Typ          : Entity_Id;
      Tag_Node     : Node_Id;
      Position     : Uint;
      Address_Node : Node_Id) return Node_Id
   is
   begin
      return
        Make_Assignment_Statement (Loc,
          Name       => Build_Get_Prim_Op_Address
                          (Loc, Typ, Tag_Node, Position),
          Expression => Address_Node);
   end Build_Set_Prim_Op_Address;

   -----------------------------
   -- Build_Set_Size_Function --
   -----------------------------

   function Build_Set_Size_Function
     (Loc       : Source_Ptr;
      Tag_Node  : Node_Id;
      Size_Func : Entity_Id) return Node_Id is
   begin
      pragma Assert (Chars (Size_Func) = Name_uSize
        and then RTE_Record_Component_Available (RE_Size_Func));
      return
        Make_Assignment_Statement (Loc,
          Name =>
            Make_Selected_Component (Loc,
              Prefix => Build_TSD (Loc, Tag_Node),
              Selector_Name =>
                New_Reference_To
                  (RTE_Record_Component (RE_Size_Func), Loc)),
          Expression =>
            Unchecked_Convert_To (RTE (RE_Size_Ptr),
              Make_Attribute_Reference (Loc,
                Prefix => New_Reference_To (Size_Func, Loc),
                Attribute_Name => Name_Unrestricted_Access)));
   end Build_Set_Size_Function;

   ------------------------------------
   -- Build_Set_Static_Offset_To_Top --
   ------------------------------------

   function Build_Set_Static_Offset_To_Top
     (Loc          : Source_Ptr;
      Iface_Tag    : Node_Id;
      Offset_Value : Node_Id) return Node_Id is
   begin
      return
        Make_Assignment_Statement (Loc,
          Make_Explicit_Dereference (Loc,
            Unchecked_Convert_To (RTE (RE_Offset_To_Top_Ptr),
              Make_Function_Call (Loc,
                Name =>
                  Make_Expanded_Name (Loc,
                    Chars => Name_Op_Subtract,
                    Prefix => New_Reference_To
                               (RTU_Entity (System_Storage_Elements), Loc),
                    Selector_Name => Make_Identifier (Loc,
                                       Chars => Name_Op_Subtract)),
                Parameter_Associations => New_List (
                  Unchecked_Convert_To (RTE (RE_Address), Iface_Tag),
                  New_Reference_To (RTE (RE_DT_Offset_To_Top_Offset),
                                    Loc))))),
          Offset_Value);
   end Build_Set_Static_Offset_To_Top;

   ---------------
   -- Build_TSD --
   ---------------

   function Build_TSD (Loc : Source_Ptr; Tag_Node : Node_Id) return Node_Id is
   begin
      return
        Unchecked_Convert_To (RTE (RE_Type_Specific_Data_Ptr),
          Make_Explicit_Dereference (Loc,
            Prefix => Unchecked_Convert_To (RTE (RE_Addr_Ptr),
              Make_Function_Call (Loc,
                Name =>
                  Make_Expanded_Name (Loc,
                    Chars => Name_Op_Subtract,
                    Prefix =>
                      New_Reference_To
                        (RTU_Entity (System_Storage_Elements), Loc),
                    Selector_Name =>
                      Make_Identifier (Loc,
                        Chars => Name_Op_Subtract)),

                Parameter_Associations => New_List (
                  Unchecked_Convert_To (RTE (RE_Address), Tag_Node),
                    New_Reference_To
                      (RTE (RE_DT_Typeinfo_Ptr_Size), Loc))))));
   end Build_TSD;

end Exp_Atag;
