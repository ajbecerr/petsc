/* $Id: ilu.h,v 1.9 1999/03/11 16:20:50 bsmith Exp bsmith $ */

/* 
   Private data structure for ILU preconditioner.
*/
#if !defined(__ILU_H)
#define __ILU_H

#include "mat.h"

typedef struct {
  Mat               fact;             /* factored matrix */
  MatOrderingType   ordering;         /* matrix reordering */
  IS                row,col;         /* row and column permutations for reordering */
  void              *implctx;         /* private implementation context */
  int               inplace;          /* in-place ILU factorization */
  int               reuseordering;    /* reuses previous reordering computed */

  PetscTruth        usedt;            /* use drop tolerance form of ILU */
  PetscTruth        reusefill;        /* reuse fill from previous ILUDT */
  double            actualfill;       /* expected fill in factorization */
  MatILUInfo        info;
} PC_ILU;

#endif
