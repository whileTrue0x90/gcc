-- C87B30A.ADA

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
-- THE EXPRESSION OF A COMPONENT ASSOCIATION MUST MATCH THE TYPE OF THE 
-- ASSOCIATED RECORD COMPONENT.
  
-- TRH  9 AUG 82
-- PWN 01/31/95  REMOVED INCONSISTENCIES WITH ADA 9X.
  
WITH REPORT; USE REPORT;
   
PROCEDURE C87B30A IS
 
     TYPE REC IS
          RECORD
               W, X : FLOAT;
               Y, Z : INTEGER;
          END RECORD;
 
     TYPE FLAG IS (PASS, FAIL);
     
     GENERIC
          TYPE T IS PRIVATE;
          ARG  : IN T;
          STAT : IN FLAG;
     FUNCTION F1 RETURN T;
 
     FUNCTION F1 RETURN T IS
     BEGIN 
          IF STAT = FAIL THEN 
               FAILED ("COMPONENT ASSOCIATION EXPRESSION MUST MATCH " &
                       "RECORD COMPONENT TYPE");
          END IF;
          RETURN ARG;
     END F1;
  
     FUNCTION F IS NEW F1 (FLOAT,     2.0, PASS);
     FUNCTION F IS NEW F1 (INTEGER,     5, FAIL);
     FUNCTION F IS NEW F1 (BOOLEAN,  TRUE, FAIL);
     FUNCTION F IS NEW F1 (CHARACTER, 'E', FAIL);
 
     FUNCTION G IS NEW F1 (FLOAT,     2.0, FAIL);
     FUNCTION G IS NEW F1 (INTEGER,     5, PASS);
     FUNCTION G IS NEW F1 (BOOLEAN,  TRUE, FAIL);
     FUNCTION G IS NEW F1 (CHARACTER, 'E', FAIL);
    
BEGIN
     TEST ("C87B30A","OVERLOADED EXPRESSIONS IN RECORD AGGREGATE " &
           "COMPONENT ASSOCIATIONS");
    
     DECLARE
          R1 : REC := (F, F, G, G);
          R2 : REC := (X => F, Y => G, Z => G, W => F);
          R3 : REC := (F, F, Z => G, Y => G);
   
     BEGIN
          NULL;
     END;
 
     RESULT;
END C87B30A;
