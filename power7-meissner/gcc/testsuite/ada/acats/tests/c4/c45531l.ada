-- C45531L.ADA

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
--     CHECK THAT THE OPERATOR "/" PRODUCES CORRECT RESULTS
--     FOR MIXED FIXED POINT AND INTEGER TYPES USING 3 SUBTESTS.
--       THIS TEST REQUIRES MIN_WORD_LENGTH = 32.
--       THIS TEST USES VALUES OF DELTA WHICH ARE GREATER THAN OR
--       EQUAL TO 0.5.
--
--     TEST CASES ARE:
--       A) FIXED / INTEGER WHEN ALL VALUES ARE MODEL NUMBERS.
--       B) FIXED / INTEGER WITH NUMERATOR MODEL NUMBER AND RESULT NOT.
--       C) FIXED / INTEGER FOR NON-MODEL NUMBERS.
--
--     REPEAT FOR MINIMUM REQUIRED WORD LENGTHS OF 12, 16, 32 AND 48,
--     WITH RANGE <, =, AND > THAN 1.0 AND
--     WITH DELTA <, =, AND > THAN 1.0.

-- HISTORY:
--     NTW 09/08/86 CREATED ORIGINAL TEST.
--     RJW 11/05/86 REVISED COMMENTS.
--     DHH 01/13/88 ADDED APPLICABILITY CRITERIA AND STANDARD HEADER.
--     BCB 04/27/90 REVISED APPLICABILITY CRITERIA.
--     BCB 10/03/90 REMOVED APPLICABILITY CRITERIA AND N/A => ERROR
--                  LINE.  CHANGED EXTENSION FROM '.DEP' TO '.ADA'.


WITH REPORT;
PROCEDURE C45531L IS

     USE REPORT;

     MIN_WORD_LENGTH : CONSTANT := 32;
     FULL_SCALE      : CONSTANT := 2 ** (MIN_WORD_LENGTH - 1);
     FORTH           : CONSTANT := FULL_SCALE / 4;
     RNG1            : CONSTANT := FULL_SCALE * 0.5;
     TYPE FX_0P5  IS DELTA 0.5 RANGE -RNG1 * 1 .. RNG1 * 1 - 0.5;
     TYPE FX_1    IS DELTA 1.0 RANGE -RNG1 * 2 .. RNG1 * 2 - 1.0;
     TYPE FX_RNG1 IS DELTA RNG1
                    RANGE -RNG1 * FULL_SCALE .. RNG1 * (FULL_SCALE - 1);

BEGIN TEST ("C45531L", "MIXED FIXED POINT AND INTEGER ""/"" "
                     & "FOR DELTA <, =, > 1.0");

     --------------------------------------------------

     -- CASE A) FIXED / INTEGER WHEN ALL VALUES ARE MODEL NUMBERS.

A:   DECLARE
          A              : FX_0P5  := 0.0;
          B              : INTEGER := 0;
          RESULT_VALUE   : FX_0P5  := 0.0;
          LOWEST_ACCEPTABLE_VALUE  : FX_0P5 := FX_0P5 (1.5);
          HIGHEST_ACCEPTABLE_VALUE : FX_0P5 := FX_0P5 (1.5);
     BEGIN
          IF EQUAL (3, 3) THEN
               A := FX_0P5 (7.5);                 -- A MODEL NUMBER
               B := 5;
          END IF;

          RESULT_VALUE := A / B;

          IF    (RESULT_VALUE < LOWEST_ACCEPTABLE_VALUE)
             OR (RESULT_VALUE > HIGHEST_ACCEPTABLE_VALUE) THEN
               FAILED ("RESULT OF ""/"" OUTSIDE RESULT MODEL INTERVAL "
                    &  "FOR FIXED / INTEGER "
                    &  "WHEN ALL VALUES ARE MODEL NUMBERS");
          END IF;
     END A;

     --------------------------------------------------

     -- CASE B) FIXED / INTEGER WITH NUMERATOR MODEL NUMBER, RESULT NOT

B:   DECLARE
          A              : FX_1    := 0.0;
          B              : INTEGER := 0;
          RESULT_VALUE   : FX_1    := 0.0;
          LOWEST_ACCEPTABLE_VALUE  :  FX_1
                                   := FX_1 (FORTH );
          HIGHEST_ACCEPTABLE_VALUE :  FX_1
                                   := FX_1 (FORTH + 1);
     BEGIN
          IF EQUAL (3, 3) THEN
               A := FX_1 (3 * FORTH + 1);         -- A MODEL NUMBER
               B := 3;
          END IF;

          RESULT_VALUE := A / B;

          IF    (RESULT_VALUE < LOWEST_ACCEPTABLE_VALUE)
             OR (RESULT_VALUE > HIGHEST_ACCEPTABLE_VALUE) THEN
               FAILED ("RESULT OF ""/"" OUTSIDE RESULT MODEL INTERVAL "
                    &  "FOR FIXED / INTEGER WITH NUMERATOR MODEL "
                    &  "NUMBER, RESULT NOT");

          END IF;
     END B;

     --------------------------------------------------

     -- CASE C) FIXED / INTEGER FOR NON-MODEL NUMBERS

C:   DECLARE
          A              : FX_RNG1 := 0.0;
          B              : INTEGER := 0;
          RESULT_VALUE   : FX_RNG1 := 0.0;
          LOWEST_ACCEPTABLE_VALUE  :  FX_RNG1
                                   := FX_RNG1 (RNG1 * FORTH );
          HIGHEST_ACCEPTABLE_VALUE :  FX_RNG1
                                   := FX_RNG1 (RNG1 * (FORTH + 1) );
     BEGIN
          IF EQUAL (3, 3) THEN                    -- A NOT MODEL NUMBER
               A := FX_RNG1 (3 * (RNG1 * FORTH + 0.5) );
               B := 3;
          END IF;

          RESULT_VALUE := A / B;

          IF    (RESULT_VALUE < LOWEST_ACCEPTABLE_VALUE)
             OR (RESULT_VALUE > HIGHEST_ACCEPTABLE_VALUE) THEN
               FAILED ("RESULT OF ""/"" OUTSIDE RESULT MODEL INTERVAL "
                    &  "FOR FIXED / INTEGER FOR NON-MODEL NUMBERS");
          END IF;
     END C;

     --------------------------------------------------


     RESULT;

END C45531L;
