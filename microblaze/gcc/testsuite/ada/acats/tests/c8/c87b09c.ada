-- C87B09C.ADA

--                             Grant of Unlimited Rights
--
--     Under contracts F33600-87-D-0337, F33600-84-D-0280, MDA903-79-C-0687,
--     F08630-91-C-0015, and DCA100-97-D-0025, the U.S. Government obtained 
--     unlimited rights in the software and documentation contained herein.
--     Unlimited rights are defined in DFAR 252.227-7013(a)(19).  By making 
--     this public release, the Government intends to confer upon all 
--     recipients unlimited rights  equal to those held by the Government.  
--     These rights include rights to use, duplicate, release or disclose the 
--     released technical data and computer software in whole or in part, in 
--     any manner and for any purpose whatsoever, and to have or permit others 
--     to do so.
--
--                                    DISCLAIMER
--
--     ALL MATERIALS OR INFORMATION HEREIN RELEASED, MADE AVAILABLE OR
--     DISCLOSED ARE AS IS.  THE GOVERNMENT MAKES NO EXPRESS OR IMPLIED 
--     WARRANTY AS TO ANY MATTER WHATSOEVER, INCLUDING THE CONDITIONS OF THE
--     SOFTWARE, DOCUMENTATION OR OTHER INFORMATION RELEASED, MADE AVAILABLE 
--     OR DISCLOSED, OR THE OWNERSHIP, MERCHANTABILITY, OR FITNESS FOR A
--     PARTICULAR PURPOSE OF SAID MATERIAL.
--*
-- CHECK THAT OVERLOADING RESOLUTION USES THE RULE THAT:
--
-- IN A FLOATING POINT TYPE DEFINITION, THE DIGITS EXPRESSION MUST
-- BE OF SOME INTEGRAL TYPE. SIMILARLY, THE DELTA EXPRESSION IN A
-- FIXED POINT TYPE DEFINITION MUST BE OF SOME REAL TYPE.
  
-- TRH  30 JUNE 82
  
WITH REPORT; USE REPORT;
   
PROCEDURE C87B09C IS
     
     FUNCTION "+" (X : INTEGER) RETURN FLOAT IS
     BEGIN 
           FAILED ("DIGITS EXPRESSION MUST BE OF AN INTEGRAL TYPE");
           RETURN 2.0;
     END "+";

     FUNCTION "+" (X : FLOAT) RETURN INTEGER IS
     BEGIN 
           FAILED ("DELTA EXPRESSION MUST BE OF A REAL TYPE");
           RETURN 2;
     END "+";
  
BEGIN
     TEST ("C87B09C","OVERLOADED DIGITS/DELTA EXPRESSIONS IN " &
           "REAL TYPE DEFINITIONS");
   
     DECLARE
          TYPE EXACT IS DIGITS "+" (4);
          TYPE CENTI IS DELTA  "+" (0.01) RANGE -2.0 .. 2.0;
          TYPE CLOSE IS DIGITS "+" (2)    RANGE -1.0 .. 1.0;
          TYPE DECI  IS DELTA  "+" (0.1)  RANGE -1.0 .. 1.0;
  
     BEGIN
          NULL;
     END;
    
     RESULT;
END C87B09C;
