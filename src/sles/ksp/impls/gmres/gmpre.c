/*$Id: gmpre.c,v 1.22 1999/12/15 17:23:31 bsmith Exp bsmith $*/

#include "src/sles/ksp/impls/gmres/gmresp.h"       /*I  "ksp.h"  I*/

#undef __FUNC__  
#define __FUNC__ "KSPGMRESSetPreAllocateVectors" 
/*@
    KSPGMRESSetPreAllocateVectors - Causes GMRES to preallocate all its
    needed work vectors at initial setup rather than the default, which 
    is to allocate them in chunks when needed.

    Collective on KSP

    Input Parameter:
.   ksp   - iterative context obtained from KSPCreate

    Options Database Key:
.   -ksp_gmres_preallocate - Activates KSPGmresSetPreAllocateVectors()

    Level: intermediate

.keywords: GMRES, preallocate, vectors

.seealso: KSPGMRESSetRestart(), KSPGMRESSetOrthogonalization()
@*/
int KSPGMRESSetPreAllocateVectors(KSP ksp)
{
  int ierr,(*f)(KSP);

  PetscFunctionBegin;
  ierr = PetscObjectQueryFunction((PetscObject)ksp,"KSPGMRESSetPreAllocateVectors_C",(void **)&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(ksp);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

