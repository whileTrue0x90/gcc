-- CD5012B.ADA

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
-- OBJECTIVE:
--     CHECK THAT AN ADDRESS CLAUSE CAN BE GIVEN FOR A VARIABLE OF AN
--     INTEGER TYPE IN THE DECLARATIVE PART OF A GENERIC PACKAGE BODY.

-- HISTORY:
--     DHH 09/16/87  CREATED ORIGINAL TEST.
--     PWB 05/11/89  CHANGED EXTENSION FROM '.DEP' TO '.ADA'.

WITH SYSTEM; USE SYSTEM;
WITH REPORT; USE REPORT;
WITH SPPRT13;
PROCEDURE CD5012B IS

BEGIN

     TEST ("CD5012B", "AN ADDRESS CLAUSE CAN BE " &
                      "GIVEN FOR A VARIABLE OF AN INTEGER " &
                      "TYPE IN THE DECLARATIVE PART OF A " &
                      "GENERIC PACKAGE BODY");

     DECLARE

          GENERIC
          PACKAGE GENPACK IS
          END GENPACK;

          PACKAGE BODY GENPACK IS

               INT2 : INTEGER :=2;

               FOR INT2 USE AT
                      SPPRT13.VARIABLE_ADDRESS;

          BEGIN
               IF EQUAL (3, 3) THEN
                    INT2 := 1;
               END IF;
               IF INT2 /= 1 THEN
                    FAILED ("WRONG VALUE FOR VARIABLE IN " &
                            "A GENERIC PACKAGE BODY");
               END IF;
               IF INT2'ADDRESS /= SPPRT13.VARIABLE_ADDRESS THEN
                    FAILED ("WRONG ADDRESS FOR VARIABLE " &
                            "IN A GENERIC PACKAGE BODY");
               END IF;
          END GENPACK;

          PACKAGE PACK IS NEW GENPACK;
     BEGIN
          NULL;
     END;
     RESULT;
END CD5012B;
