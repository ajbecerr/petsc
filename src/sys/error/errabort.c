/*
       The default error handlers and code that allows one to change
   error handlers.
*/
#include <petscsys.h> /*I "petscsys.h" I*/

/*@C
  PetscAbortErrorHandler - Error handler that calls abort on error.
  This routine is very useful when running in the debugger, because the
  user can look directly at the stack frames and the variables.

  Not Collective

  Input Parameters:
+ comm - communicator over which error occurred
. line - the line number of the error (indicated by __LINE__)
. fun  - the function name
. file - the file in which the error was detected (indicated by __FILE__)
. mess - an error text string, usually just printed to the screen
. n    - the generic error number
. p    - specific error number
- ctx  - error handler context

  Options Database Keys:
+ -on_error_abort                                         - Activates aborting when an error is encountered
- -start_in_debugger [noxterm,dbx,xxgdb]  [-display name] - Starts all
    processes in the debugger and uses PetscAbortErrorHandler().  By default the
    debugger is gdb; alternatives are dbx and xxgdb.

  Level: developer

  Notes:
  Ussers do not directly employ this routine

  Use `PetscPushErrorHandler()` to set the desired error handler.  The
  currently available PETSc error handlers include `PetscTraceBackErrorHandler()`,
  `PetscAttachDebuggerErrorHandler()`, and `PetscAbortErrorHandler()`.

.seealso: `PetscError()`, `PetscPushErrorHandler()`, `PetscPopErrorHander()`, `PetscTraceBackErrorHandler()`,
          `PetscAttachDebuggerErrorHandler()`, `PetscMPIAbortErrorHandler()`, `PetscReturnErrorHandler()`, `PetscEmacsClientErrorHandler()`
@*/
PetscErrorCode PetscAbortErrorHandler(MPI_Comm comm, int line, const char *fun, const char *file, PetscErrorCode n, PetscErrorType p, const char *mess, void *ctx)
{
  PetscFunctionBegin;
  (void)comm;
  (void)p;
  (void)ctx;
  n = (*PetscErrorPrintf)("PetscAbortErrorHandler: %s() at %s:%d %s\n  To prevent termination, change the error handler using PetscPushErrorHandler()\n", fun, file, line, mess);
  abort();
  (void)n;
  PetscFunctionReturn(PETSC_SUCCESS);
}
