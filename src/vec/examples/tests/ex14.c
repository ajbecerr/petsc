/*$Id: ex14.c,v 1.40 1999/10/24 14:01:59 bsmith Exp bsmith $*/

static char help[] = "Scatters from a sequential vector to a parallel vector.\n\
This does the tricky case.\n\n";

#include "vec.h"
#include "sys.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  int           n = 5,ierr;
  int           size,rank,N;
  Scalar        value,zero = 0.0;
  Vec           x,y;
  IS            is1,is2;
  VecScatter    ctx = 0;

  PetscInitialize(&argc,&argv,(char*)0,help);
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRA(ierr);
  ierr = MPI_Comm_rank(PETSC_COMM_WORLD,&rank);CHKERRA(ierr);

  /* create two vectors */
  N = size*n;
  ierr = VecCreateMPI(PETSC_COMM_WORLD,PETSC_DECIDE,N,&y);CHKERRA(ierr);
  ierr = VecCreateSeq(PETSC_COMM_SELF,N,&x);CHKERRA(ierr);

  /* create two index sets */
  ierr = ISCreateStride(PETSC_COMM_SELF,n,0,1,&is1);CHKERRA(ierr);
  ierr = ISCreateStride(PETSC_COMM_SELF,n,rank,1,&is2);CHKERRA(ierr);

  value = rank+1; 
  ierr = VecSet(&value,x);CHKERRA(ierr);
  ierr = VecSet(&zero,y);CHKERRA(ierr);

  ierr = VecScatterCreate(x,is1,y,is2,&ctx);CHKERRA(ierr);
  ierr = VecScatterBegin(x,y,ADD_VALUES,SCATTER_FORWARD,ctx);CHKERRA(ierr);
  ierr = VecScatterEnd(x,y,ADD_VALUES,SCATTER_FORWARD,ctx);CHKERRA(ierr);
  ierr = VecScatterDestroy(ctx);CHKERRA(ierr);
  
  ierr = VecView(y,VIEWER_STDOUT_WORLD);CHKERRA(ierr);

  ierr = VecDestroy(x);CHKERRA(ierr);
  ierr = VecDestroy(y);CHKERRA(ierr);
  ierr = ISDestroy(is1);CHKERRA(ierr);
  ierr = ISDestroy(is2);CHKERRA(ierr);

  PetscFinalize(); 
  return 0;
}
 
