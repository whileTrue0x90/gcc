------------------------------------------------------------------------------
--                                                                          --
--                         GNAT COMPILER COMPONENTS                         --
--                                                                          --
--                             S E M _ P R A G                              --
--                                                                          --
--                                 S p e c                                  --
--                                                                          --
--          Copyright (C) 1992-2015, Free Software Foundation, Inc.         --
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

--  Pragma handling is isolated in a separate package
--  (logically this processing belongs in chapter 4)

with Namet;  use Namet;
with Opt;    use Opt;
with Snames; use Snames;
with Types;  use Types;

package Sem_Prag is

   --  The following table lists all pragmas that act as an assertion
   --  expression.

   Assertion_Expression_Pragma : constant array (Pragma_Id) of Boolean :=
     (Pragma_Assert               => True,
      Pragma_Assert_And_Cut       => True,
      Pragma_Assume               => True,
      Pragma_Check                => True,
      Pragma_Contract_Cases       => True,
      Pragma_Initial_Condition    => True,
      Pragma_Invariant            => True,
      Pragma_Loop_Invariant       => True,
      Pragma_Loop_Variant         => True,
      Pragma_Post                 => True,
      Pragma_Post_Class           => True,
      Pragma_Postcondition        => True,
      Pragma_Pre                  => True,
      Pragma_Pre_Class            => True,
      Pragma_Precondition         => True,
      Pragma_Predicate            => True,
      Pragma_Refined_Post         => True,
      Pragma_Test_Case            => True,
      Pragma_Type_Invariant       => True,
      Pragma_Type_Invariant_Class => True,
      others                      => False);

   --  The following table lists all the implementation-defined pragmas that
   --  may apply to a body stub (no language defined pragmas apply). The table
   --  should be synchronized with Aspect_On_Body_Or_Stub_OK in unit Aspects if
   --  the pragmas below implement an aspect.

   Pragma_On_Body_Or_Stub_OK : constant array (Pragma_Id) of Boolean :=
     (Pragma_Refined_Depends => True,
      Pragma_Refined_Global  => True,
      Pragma_Refined_Post    => True,
      Pragma_SPARK_Mode      => True,
      Pragma_Warnings        => True,
      others                 => False);

   -----------------
   -- Subprograms --
   -----------------

   procedure Analyze_Pragma (N : Node_Id);
   --  Analyze procedure for pragma reference node N

   procedure Analyze_Contract_Cases_In_Decl_Part (N : Node_Id);
   --  Perform full analysis and expansion of delayed pragma Contract_Cases

   procedure Analyze_Depends_In_Decl_Part (N : Node_Id);
   --  Perform full analysis of delayed pragma Depends. This routine is also
   --  capable of performing basic analysis of pragma Refined_Depends.

   procedure Analyze_External_Property_In_Decl_Part
     (N        : Node_Id;
      Expr_Val : out Boolean);
   --  Perform full analysis of delayed pragmas Async_Readers, Async_Writers,
   --  Effective_Reads and Effective_Writes. Flag Expr_Val contains the Boolean
   --  argument of the pragma or a default True if no argument is present.

   procedure Analyze_Global_In_Decl_Part (N : Node_Id);
   --  Perform full analysis of delayed pragma Global. This routine is also
   --  capable of performing basic analysis of pragma Refind_Global.

   procedure Analyze_Initial_Condition_In_Decl_Part (N : Node_Id);
   --  Perform full analysis of delayed pragma Initial_Condition

   procedure Analyze_Initializes_In_Decl_Part (N : Node_Id);
   --  Perform full analysis of delayed pragma Initializes

   procedure Analyze_Pre_Post_Condition_In_Decl_Part (N : Node_Id);
   --  Perform preanalysis of [refined] precondition or postcondition pragma
   --  N that appears on a subprogram declaration or body [stub].

   procedure Analyze_Refined_Depends_In_Decl_Part (N : Node_Id);
   --  Preform full analysis of delayed pragma Refined_Depends. This routine
   --  uses Analyze_Depends_In_Decl_Part as a starting point, then performs
   --  various consistency checks between Depends and Refined_Depends.

   procedure Analyze_Refined_Global_In_Decl_Part (N : Node_Id);
   --  Perform full analysis of delayed pragma Refined_Global. This routine
   --  uses Analyze_Global_In_Decl_Part as a starting point, then performs
   --  various consistency checks between Global and Refined_Global.

   procedure Analyze_Refined_State_In_Decl_Part (N : Node_Id);
   --  Perform full analysis of delayed pragma Refined_State

   procedure Analyze_Test_Case_In_Decl_Part (N : Node_Id);
   --  Perform preanalysis of pragma Test_Case

   procedure Check_Applicable_Policy (N : Node_Id);
   --  N is either an N_Aspect or an N_Pragma node. There are two cases. If
   --  the name of the aspect or pragma is not one of those recognized as
   --  an assertion kind by an Assertion_Policy pragma, then the call has
   --  no effect. Note that in the case of a pragma derived from an aspect,
   --  the name we use for the purpose of this procedure is the aspect name,
   --  which may be different from the pragma name (e.g. Precondition for
   --  Pre aspect). In addition, 'Class aspects are recognized (and the
   --  corresponding special names used in the processing).
   --
   --  If the name is a valid assertion kind name, then the Check_Policy pragma
   --  chain is checked for a matching entry (or for an Assertion entry which
   --  matches all possibilities). If a matching entry is found then the policy
   --  is checked. If it is On or Check, then the Is_Checked flag is set in
   --  the aspect or pragma node. If it is Off, Ignore, or Disable, then the
   --  Is_Ignored flag is set in the aspect or pragma node. Additionally for
   --  policy Disable, the Is_Disabled flag is set.
   --
   --  If no matching Check_Policy pragma is found then the effect depends on
   --  whether -gnata was used, if so, then the call has no effect, otherwise
   --  Is_Ignored (but not Is_Disabled) is set True.

   procedure Check_External_Properties
     (Item : Node_Id;
      AR   : Boolean;
      AW   : Boolean;
      ER   : Boolean;
      EW   : Boolean);
   --  Flags AR, AW, ER and EW denote the static values of external properties
   --  Async_Readers, Async_Writers, Effective_Reads and Effective_Writes. Item
   --  is the related variable or state. Ensure legality of the combination and
   --  issue an error for an illegal combination.

   procedure Check_Missing_Part_Of (Item_Id : Entity_Id);
   --  Determine whether the placement within the state space of an abstract
   --  state, variable or package instantiation denoted by Item_Id requires the
   --  use of indicator/option Part_Of. If this is the case, emit an error.

   procedure Collect_Subprogram_Inputs_Outputs
     (Subp_Id      : Entity_Id;
      Synthesize   : Boolean := False;
      Subp_Inputs  : in out Elist_Id;
      Subp_Outputs : in out Elist_Id;
      Global_Seen  : out Boolean);
   --  Subsidiary to the analysis of pragmas Depends, Global, Refined_Depends
   --  and Refined_Global. The routine is also used by GNATprove. Collect all
   --  inputs and outputs of subprogram Subp_Id in lists Subp_Inputs (inputs)
   --  and Subp_Outputs (outputs). The inputs and outputs are gathered from:
   --    1) The formal parameters of the subprogram
   --    2) The items of pragma [Refined_]Global
   --         or
   --    3) The items of pragma [Refined_]Depends if there is no pragma
   --       [Refined_]Global present and flag Synthesize is set to True.
   --  If the subprogram has no inputs and/or outputs, then the returned list
   --  is No_Elist. Flag Global_Seen is set when the related subprogram has
   --  pragma [Refined_]Global.

   function Delay_Config_Pragma_Analyze (N : Node_Id) return Boolean;
   --  N is a pragma appearing in a configuration pragma file. Most such
   --  pragmas are analyzed when the file is read, before parsing and analyzing
   --  the main unit. However, the analysis of certain pragmas results in
   --  adding information to the compiled main unit, and this cannot be done
   --  till the main unit is processed. Such pragmas return True from this
   --  function and in Frontend pragmas where Delay_Config_Pragma_Analyze is
   --  True have their analysis delayed until after the main program is parsed
   --  and analyzed.

   function Find_Related_Subprogram_Or_Body
     (Prag      : Node_Id;
      Do_Checks : Boolean := False) return Node_Id;
   --  Subsidiary to the analysis of pragmas Contract_Cases, Depends, Global,
   --  Refined_Depends, Refined_Global and Refined_Post and attribute 'Result.
   --  Find the declaration of the related subprogram [body or stub] subject
   --  to pragma Prag. If flag Do_Checks is set, the routine reports duplicate
   --  pragmas and detects improper use of refinement pragmas in stand alone
   --  expression functions. The returned value depends on the related pragma
   --  as follows:
   --    1) Pragmas Contract_Cases, Depends and Global yield the corresponding
   --       N_Subprogram_Declaration node or if the pragma applies to a stand
   --       alone body, the N_Subprogram_Body node or Empty if illegal.
   --    2) Pragmas Refined_Depends, Refined_Global and Refined_Post yield
   --       N_Subprogram_Body or N_Subprogram_Body_Stub nodes or Empty if
   --       illegal.

   function Get_SPARK_Mode_From_Pragma (N : Node_Id) return SPARK_Mode_Type;
   --  Given a pragma SPARK_Mode node, return corresponding mode id

   procedure Initialize;
   --  Initializes data structures used for pragma processing. Must be called
   --  before analyzing each new main source program.

   function Is_Config_Static_String (Arg : Node_Id) return Boolean;
   --  This is called for a configuration pragma that requires either string
   --  literal or a concatenation of string literals. We cannot use normal
   --  static string processing because it is too early in the case of the
   --  pragma appearing in a configuration pragmas file. If Arg is of an
   --  appropriate form, then this call obtains the string (doing any necessary
   --  concatenations) and places it in Name_Buffer, setting Name_Len to its
   --  length, and then returns True. If it is not of the correct form, then an
   --  appropriate error message is posted, and False is returned.

   function Is_Elaboration_SPARK_Mode (N : Node_Id) return Boolean;
   --  Determine whether pragma SPARK_Mode appears in the statement part of a
   --  package body.

   function Is_Non_Significant_Pragma_Reference (N : Node_Id) return Boolean;
   --  The node N is a node for an entity and the issue is whether the
   --  occurrence is a reference for the purposes of giving warnings about
   --  unreferenced variables. This function returns True if the reference is
   --  not a reference from this point of view (e.g. the occurrence in a pragma
   --  Pack) and False if it is a real reference (e.g. the occurrence in a
   --  pragma Export);

   function Is_Pragma_String_Literal (Par : Node_Id) return Boolean;
   --  Given an N_Pragma_Argument_Association node, Par, which has the form of
   --  an operator symbol, determines whether or not it should be treated as an
   --  string literal. This is called by Sem_Ch6.Analyze_Operator_Symbol. If
   --  True is returned, the argument is converted to a string literal. If
   --  False is returned, then the argument is treated as an entity reference
   --  to the operator.

   function Is_Private_SPARK_Mode (N : Node_Id) return Boolean;
   --  Determine whether pragma SPARK_Mode appears in the private part of a
   --  package.

   function Is_Valid_Assertion_Kind (Nam : Name_Id) return Boolean;
   --  Returns True if Nam is one of the names recognized as a valid assertion
   --  kind by the Assertion_Policy pragma. Note that the 'Class cases are
   --  represented by the corresponding special names Name_uPre, Name_uPost,
   --  Name_uInvariant, and Name_uType_Invariant (_Pre, _Post, _Invariant,
   --  and _Type_Invariant).

   procedure Process_Compilation_Unit_Pragmas (N : Node_Id);
   --  Called at the start of processing compilation unit N to deal with any
   --  special issues regarding pragmas. In particular, we have to deal with
   --  Suppress_All at this stage, since it can appear after the unit instead
   --  of before (actually we allow it to appear anywhere).

   procedure Relocate_Pragmas_To_Body
     (Subp_Body   : Node_Id;
      Target_Body : Node_Id := Empty);
   --  Resocate all pragmas that follow and apply to subprogram body Subp_Body
   --  to its own declaration list. Candidate pragmas are classified in table
   --  Pragma_On_Body_Or_Stub_OK. If Target_Body is set, the pragma are moved
   --  to the declarations of Target_Body. This formal should be set when
   --  dealing with subprogram body stubs or expression functions.

   procedure Set_Encoded_Interface_Name (E : Entity_Id; S : Node_Id);
   --  This routine is used to set an encoded interface name. The node S is
   --  an N_String_Literal node for the external name to be set, and E is an
   --  entity whose Interface_Name field is to be set. In the normal case where
   --  S contains a name that is a valid C identifier, then S is simply set as
   --  the value of the Interface_Name. Otherwise it is encoded as needed by
   --  particular operating systems. See the body for details of the encoding.

   function Test_Case_Arg
     (Prag        : Node_Id;
      Arg_Nam     : Name_Id;
      From_Aspect : Boolean := False) return Node_Id;
   --  Obtain argument "Name", "Mode", "Ensures" or "Requires" from Test_Case
   --  pragma Prag as denoted by Arg_Nam. When From_Aspect is set, an attempt
   --  is made to retrieve the argument from the corresponding aspect if there
   --  is one. The returned argument has several formats:
   --
   --    N_Pragma_Argument_Association if retrieved directly from the pragma
   --
   --    N_Component_Association if retrieved from the corresponding aspect and
   --    the argument appears in a named association form.
   --
   --    An arbitrary expression if retrieved from the corresponding aspect and
   --    the argument appears in positional form.
   --
   --    Empty if there is no such argument

end Sem_Prag;
