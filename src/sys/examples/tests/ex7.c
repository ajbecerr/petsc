/*$Id: ex7.c,v 1.9 1999/12/20 20:16:53 bsmith Exp bsmith $*/

/*
     Formatted test for PetscSetCommWorld()
*/

static char help[] = "Tests PetscSetCommWorld()\n\n";

#include "petsc.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  int size,ierr;

  MPI_Init(&argc,&argv);

  /*
    Create a seperate PETSc universe for each processor
  */
  PetscSetCommWorld(MPI_COMM_SELF);
  PetscInitialize(&argc,&argv,PETSC_NULL,help);
   
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRA(ierr);
  if (size != 1) SETERRA(1,0,"main:Error from PetscSetCommWorld()");

  PetscFinalize();
  MPI_Finalize();
  return 0;
}
