/*$Id: zao.c,v 1.13 1999/10/24 14:04:19 bsmith Exp bsmith $*/

#include "src/fortran/custom/zpetsc.h"
#include "ao.h"

#ifdef PETSC_HAVE_FORTRAN_CAPS
#define aocreatebasic_   AOCREATEBASIC
#define aocreatebasicis_ AOCREATEBASICIS
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE)
#define aocreatebasic_   aocreatebasic
#define aocreatebasicis_ aocreatebasicis
#endif

EXTERN_C_BEGIN

void PETSC_STDCALL aocreatebasic_(MPI_Comm *comm,int *napp,int *myapp,int *mypetsc,AO *aoout,int *ierr)
{
  *ierr = AOCreateBasic((MPI_Comm)PetscToPointerComm(*comm),*napp,myapp,mypetsc,aoout);
}

void PETSC_STDCALL aocreatebasicis_(IS *isapp,IS *ispetsc,AO *aoout,int *ierr)
{
  *ierr = AOCreateBasicIS(*isapp,*ispetsc,aoout);
}

EXTERN_C_END
