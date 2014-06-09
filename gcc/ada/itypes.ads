------------------------------------------------------------------------------
--                                                                          --
--                         GNAT COMPILER COMPONENTS                         --
--                                                                          --
--                               I T Y P E S                                --
--                                                                          --
--                                 S p e c                                  --
--                                                                          --
--          Copyright (C) 1992-2010, Free Software Foundation, Inc.         --
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

--  This package contains declarations for handling of implicit types

with Einfo;    use Einfo;
with Sem_Util; use Sem_Util;
with Types;    use Types;

package Itypes is

   --------------------
   -- Implicit Types --
   --------------------

   --  Implicit types (Itypes) are types and subtypes created by the semantic
   --  phase or the expander to reflect the underlying semantics. These could
   --  be generated by building trees for corresponding declarations and then
   --  analyzing these trees, but there are three reasons for not doing this
   --  in some cases:

   --    1. The declarations would require more tree nodes

   --    2. In some cases, the elaboration of these types is associated
   --       with internal nodes in the tree.

   --    3. For some types, notably class wide types, there is no Ada
   --       declaration that would correspond to the desired entity.

   --  So instead, implicit types are constructed by simply creating an
   --  appropriate entity with the help of routines in this package. These
   --  entities are fully decorated, as described in Einfo (just as though
   --  they had been created by the normal analysis procedure).

   --  The type declaration declaring an Itype must be analyzed with checks
   --  off because this declaration has not been inserted in the tree (if it
   --  has been then it is not an Itype), and hence checks that would be
   --  generated during the analysis cannot be inserted in the tree. At any
   --  rate, Itype analysis should always be done with checks off, otherwise
   --  duplicate checks will most likely be emitted.

   --  Unlike types declared explicitly, implicit types are defined on first
   --  use, which means that Gigi detects the use of such types, and defines
   --  them at the point of the first use automatically.

   --  Although Itypes are not explicitly declared, they are associated with
   --  a specific node in the tree (roughly the node that caused them to be
   --  created), via the Associated_Node_For_Itype field. This association is
   --  used particularly by New_Copy_Tree, which uses it to determine whether
   --  or not to copy a referenced Itype. If the associated node is part of
   --  the tree to be copied by New_Copy_Tree, then (since the idea of the
   --  call to New_Copy_Tree is to create a complete duplicate of a tree,
   --  as though it had appeared separately in the source), the Itype in
   --  question is duplicated as part of the New_Copy_Tree processing.

   --  As a consequence of this copying mechanism, the association between
   --  Itypes and associated nodes must be one-to-one: several Itypes must
   --  not share an associated node. For example, the semantic decoration
   --  of an array aggregate generates several Itypes: for each index subtype
   --  and for the array subtype. The associated node of each index subtype
   --  is the corresponding range expression.

   --  Notes on the use of the Parent field of an Itype

   --    In some cases, we do create a declaration node for an itype, and in
   --    such cases, the Parent field of the Itype points to this declaration
   --    in the normal manner. This case can be detected by checking for a
   --    non-empty Parent field referencing a declaration whose Defining_Entity
   --    is the Itype in question.

   --    In some other cases, where we don't generate such a declaration, as
   --    described above, the Itype is attached to the tree implicitly by being
   --    referenced elsewhere, e.g. as the Etype of some object. In this case
   --    the Parent field may be Empty.

   --    In other cases where we don't generate a declaration for the Itype,
   --    the Itype may be attached to an arbitrary node in the tree, using
   --    the Parent field. This Parent field may even reference a declaration
   --    for a related different entity (hence the description of the tests
   --    needed for the case where a declaration for the Itype is created).

   ------------------
   -- Create_Itype --
   ------------------

   function Create_Itype
     (Ekind        : Entity_Kind;
      Related_Nod  : Node_Id;
      Related_Id   : Entity_Id := Empty;
      Suffix       : Character := ' ';
      Suffix_Index : Nat       := 0;
      Scope_Id     : Entity_Id := Current_Scope) return Entity_Id;
   --  Used to create a new Itype
   --
   --  Related_Nod is the node for which this Itype was created. It is
   --  set as the Associated_Node_For_Itype of the new Itype. The Sloc of
   --  the new Itype is that of this node.
   --
   --  Related_Id is present only if the implicit type name may be referenced
   --  as a public symbol, and thus needs a unique external name. The name
   --  is created by a call to:
   --
   --    New_External_Name (Chars (Related_Id), Suffix, Suffix_Index, 'T')
   --
   --  If the implicit type does not need an external name, then the
   --  Related_Id parameter is omitted (and hence Empty). In this case
   --  Suffix and Suffix_Index are ignored and the implicit type name is
   --  created by a call to Make_Temporary.
   --
   --  Note that in all cases, the name starts with "T". This is used
   --  to identify implicit types in the error message handling circuits.
   --
   --  The Scope_Id parameter specifies the scope of the created type, and
   --  is normally the Current_Scope as shown, but can be set otherwise.
   --
   --  The size/align fields are initialized to unknown (Uint_0).
   --
   --  If Ekind is in Access_Subprogram_Kind, Can_Use_Internal_Rep is set True,
   --  unless Always_Compatible_Rep_On_Target is True.

   ---------------------------------
   -- Create_Null_Excluding_Itype --
   ---------------------------------

   function Create_Null_Excluding_Itype
      (T           : Entity_Id;
       Related_Nod : Node_Id;
       Scope_Id    : Entity_Id := Current_Scope) return Entity_Id;
   --  Ada 2005 (AI-231): T is an access type and this subprogram creates and
   --  returns an internal access-subtype declaration of T that has the null
   --  exclusion attribute set to True.
   --
   --  Usage of null-excluding Itypes
   --  ------------------------------
   --
   --      type T1 is access ...
   --      type T2 is not null T1;
   --
   --      type Rec is record
   --         Comp : not null T1;
   --      end record;
   --
   --      type Arr is array (...) of not null T1;
   --
   --  Instead of associating the not-null attribute with the defining ids of
   --  these declarations, we generate an internal subtype declaration of T1
   --  that has the null exclusion attribute set to true.

end Itypes;
