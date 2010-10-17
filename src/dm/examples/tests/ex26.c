
static char help[] = "Tests error message in DMGetColoring() with periodic boundary conditions. \n\n";


#include "petscdm.h"
#include "petscmat.h"

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **argv)
{
  Mat            J;                   
  PetscErrorCode ierr; 
  DM             da;
  MatFDColoring  matfdcoloring = 0;
  ISColoring     iscoloring;

  PetscInitialize(&argc,&argv,(char *)0,help);
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create distributed array (DMDA) to manage parallel grid and vectors
  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = DMDACreate2d(PETSC_COMM_WORLD,DMDA_XPERIODIC,DMDA_STENCIL_BOX,-5,-5,
                    PETSC_DECIDE,PETSC_DECIDE,1,2,0,0,&da);CHKERRQ(ierr);
  ierr = DMGetMatrix(da,MATAIJ,&J);CHKERRQ(ierr);
  ierr = DMGetColoring(da,IS_COLORING_GHOSTED,MATAIJ,&iscoloring);CHKERRQ(ierr);
  ierr = MatFDColoringCreate(J,iscoloring,&matfdcoloring);CHKERRQ(ierr);
  ierr = ISColoringDestroy(iscoloring);CHKERRQ(ierr);

  /* free spaces */
  ierr = MatDestroy(J);CHKERRQ(ierr);
  ierr = MatFDColoringDestroy(matfdcoloring);CHKERRQ(ierr); 
  ierr = DMDestroy(da);CHKERRQ(ierr);
  ierr = PetscFinalize();
  PetscFunctionReturn(0);
}
