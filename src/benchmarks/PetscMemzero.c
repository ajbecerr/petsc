/*$Id: PetscMemzero.c,v 1.13 1999/10/24 14:04:16 bsmith Exp bsmith $*/

#include "petsc.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  PLogDouble x,y,z;
  Scalar     A[10000];
  int        ierr;

  PetscInitialize(&argc,&argv,0,0);
  /* To take care of paging effects */
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscGetTime(&x);CHKERRA(ierr);

  ierr = PetscGetTime(&x);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*10000);CHKERRA(ierr);,
  ierr = PetscGetTime(&y);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscMemzero(A,sizeof(Scalar)*0);CHKERRA(ierr);
  ierr = PetscGetTime(&z);CHKERRA(ierr);

  fprintf(stderr,"%s : \n","PetscMemzero");
  fprintf(stderr,"    %-11s : %e sec\n","Latency",(z-y)/10.0);
  fprintf(stderr,"    %-11s : %e sec\n","Per Scalar",(2*y-x-z)/100000.0);

  PetscFinalize();
  PetscFunctionReturn(0);
}
