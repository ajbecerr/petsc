/*$Id: errstop.c,v 1.7 1999/11/05 14:44:06 bsmith Exp bsmith $*/

#include "petsc.h"           /*I "petsc.h" I*/

#undef __FUNC__  
#define __FUNC__ "PetscStopErrorHandler"
/*@C
   PetscStopErrorHandler - Calls MPI_abort() and exists.

   Not Collective

   Input Parameters:
+  line - the line number of the error (indicated by __LINE__)
.  fun - the function where the error occurred (indicated by __FUNC__)
.  file - the file in which the error was detected (indicated by __FILE__)
.  dir - the directory of the file (indicated by __SDIR__)
.  mess - an error text string, usually just printed to the screen
.  n - the generic error number
.  p - the specific error number
-  ctx - error handler context

   Level: developer

   Notes:
   Most users need not directly employ this routine and the other error 
   handlers, but can instead use the simplified interface SETERRQ, which has 
   the calling sequence
$     SETERRQ(n,p,mess)

   Notes for experienced users:
   Use PetscPushErrorHandler() to set the desired error handler.  The
   currently available PETSc error handlers include PetscTraceBackErrorHandler(),
   PetscStopErrorHandler(), PetscAttachDebuggerErrorHandler(), and PetscAbortErrorHandler().

.keywords: default, error, handler, traceback

.seealso:  PetscPushErrorHandler(), PetscAttachDebuggerErrorHandler(), 
           PetscAbortErrorHandler(), PetscTraceBackErrorHandler()
 @*/
int PetscStopErrorHandler(int line,char *fun,char *file,char *dir,int n,int p,char *mess,void *ctx)
{
  int        rank;
  PetscTruth flg1,flg2;
  PLogDouble mem,rss;

  PetscFunctionBegin;
  if (!mess) mess = " ";

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  if (n == PETSC_ERR_MEM) {
    (*PetscErrorPrintf)("[%d]PETSC ERROR: %s() line %d in %s%s\n",rank,fun,line,dir,file);
    (*PetscErrorPrintf)("[%d]PETSC ERROR:   Out of memory. This could be due to allocating\n",rank);
    (*PetscErrorPrintf)("[%d]PETSC ERROR:   too large an object or bleeding by not properly\n",rank);
    (*PetscErrorPrintf)("[%d]PETSC ERROR:   destroying unneeded objects.\n",rank);
    PetscTrSpace(&mem,PETSC_NULL,PETSC_NULL); PetscGetResidentSetSize(&rss);
    OptionsHasName(PETSC_NULL,"-trdump",&flg1);
    OptionsHasName(PETSC_NULL,"-trmalloc_log",&flg2);
    if (flg2) {
      PetscTrLogDump(stderr);
    } else if (flg1) {
      (*PetscErrorPrintf)("[%d]PETSC ERROR:   Memory allocated %d Memory used by process %d\n",rank,(int)mem,(int)rss);
      PetscTrDump(stderr);
    }  else {
      (*PetscErrorPrintf)("[%d]PETSC ERROR:   Memory allocated %d Memory used by process %d\n",rank,(int)mem,(int)rss);
      (*PetscErrorPrintf)("[%d]PETSC ERROR:   Try running with -trdump or -trmalloc_log for info.\n",rank);
    }
    n = 1;
  } else if (n == PETSC_ERR_SUP) {
    (*PetscErrorPrintf)("[%d]PETSC ERROR: %s() line %d in %s%s\n",rank,fun,line,dir,file);
    (*PetscErrorPrintf)("[%d]PETSC ERROR: No support for this operation for this object type!\n",rank);
    (*PetscErrorPrintf)("[%d]PETSC ERROR: %s\n",rank,mess);
    n = 1;
  } else if (n == PETSC_ERR_SIG) {
    (*PetscErrorPrintf)("[%d]PETSC ERROR: %s() line %d in %s%s %s\n",rank,fun,line,dir,file,mess);
  } else {
    (*PetscErrorPrintf)("[%d]PETSC ERROR: %s() line %d in %s%s\n    %s\n",rank,fun,line,dir,file,mess);
  }
  MPI_Abort(PETSC_COMM_WORLD,n);
  PetscFunctionReturn(0);
}

