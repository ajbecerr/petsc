#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: ec.c,v 1.12 1998/04/27 16:09:26 bsmith Exp curfman $";
#endif

/*
   This is where the eigenvalue computation operations are defined
*/

#include "src/ec/ecimpl.h"        /*I "ec.h" I*/
#include "pinclude/pviewer.h"


#undef __FUNC__  
#define __FUNC__ "ECDestroy" 
/*@C
   ECDestroy - Destroys EC context that was created with ECCreate().

   Collective on EC

   Input Parameter:
.  ec - the eigenvalue computation context

.keywords: EC, destroy

.seealso: ECCreate(), ECSetUp()
@*/
int ECDestroy(EC ec)
{
  int ierr = 0;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);
  if (ec->ops->destroy) {ierr =  (*ec->ops->destroy)(ec);CHKERRQ(ierr);}
  else {if (ec->data) PetscFree(ec->data);}
  PLogObjectDestroy(ec);
  PetscHeaderDestroy(ec);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECView" 
/*@ 
   ECView - Prints the EC data structure.

   Collective on EC unless view is sequential

   Input Parameters:
+  EC - the eigenvalue computation context
-  viewer - visualization context

   Note:
   The available visualization contexts include
+     VIEWER_STDOUT_SELF - standard output (default)
-     VIEWER_STDOUT_WORLD - synchronized standard
         output where only the first processor opens
         the file.  All other processors send their 
         data to the first processor to print. 

   The user can open an alternative visualization context with
   ViewerFileOpenASCII() - output to a specified file.

.keywords: EC, view

.seealso: ECView(), ViewerFileOpenASCII(), ECView()
@*/
int ECView(EC ec,Viewer viewer)
{
  FILE        *fd;
  char        *method;
  int         ierr;
  ViewerType  vtype;

  PetscFunctionBegin;
  ierr = ViewerGetType(viewer,&vtype); CHKERRQ(ierr);
  if (vtype == ASCII_FILE_VIEWER || vtype == ASCII_FILES_VIEWER) {
    ierr = ViewerASCIIGetPointer(viewer,&fd); CHKERRQ(ierr);
    PetscFPrintf(ec->comm,fd,"EC Object:\n");
    ECGetType(ec,PETSC_NULL,&method);
    PetscFPrintf(ec->comm,fd,"  method: %s\n",method);
    if (ec->view) (*ec->ops->view)(ec,viewer);
  } else if (vtype == STRING_VIEWER) {
    ECType type;
    ECGetType(ec,&type,&method);
    ierr = ViewerStringSPrintf(viewer," %-7.7s",method); CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECGetEigenvalues"
/*@
   ECGetEigenvalues - Returns pointers to the real and 
   imaginary components of the eigenvalues computed 
   with an EC context.

   Not Collective

   Input Parameters:
.  ec - the eigenvalue computation context

   Output Parameters:
+  n - number of eigenvalues computed
.  rpart - array containing the real parts of the eigenvalues
-  ipart - array containing the imaginary parts of the eigenvalues

   Notes:
   ECGetEigenvalues() may be called only after ECSolve().

.keywords: EC, get, eigenvalues

.seealso: ECCreate(), ECSolve(), ECDestroy()
@*/
int ECGetEigenvalues(EC ec,int *n,double **rpart,double **ipart)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);

  *n     = ec->n;
  *rpart = ec->realpart;
  *ipart = ec->imagpart;
  PetscFunctionReturn(0);
}


#undef __FUNC__  
#define __FUNC__ "ECGetEigenvectors"
/*@
   ECGetEigenvectors - Returns pointers to the eigenvectors
   computed  with an EC context.

   Not Collective

   Input Parameters:
.  ec - the eigenvalue computation context

   Output Parameters:
+  n - number of eigenvectors computed
-  evecs - the eigenvectors

   Notes:
   ECGetEigenvectors() may be called only after ECSolveEigenvectors().

.keywords: EC, get, eigenvectors

.seealso: ECCreate(), ECSolve(), ECDestroy(), ECSolveEigenvectors()
@*/
int ECGetEigenvectors(EC ec,int *n,Vec *evecs)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);

  *n     = ec->n;
  *evecs = ec->evecs;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSetUp"
/*@
   ECSetUp - Prepares for the use of an eigenvalue solver.

   Collective on EC

   Input parameters:
.  ec - the eigenvalue computation context

.keywords: EC, setup

.seealso: ECCreate(), ECSolve(), ECDestroy()
@*/
int ECSetUp(EC ec)
{
  int ierr;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);

  if (ec->setupcalled > 0) PetscFunctionReturn(0);
  PLogEventBegin(EC_SetUp,ec,0,0,0); 
  if (!ec->A) {SETERRQ(PETSC_ERR_ARG_WRONGSTATE,0,"Matrix must be set first");}
  if (ec->setup) { ierr = (*ec->setup)(ec); CHKERRQ(ierr);}
  ec->setupcalled = 1;
  PLogEventEnd(EC_SetUp,ec,0,0,0); 
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSetEigenvectorsRequired"
/*@C
   ECSetEigenvectorsRequired - Indicates that both eigenvalues and
   eigenvectors should be computed.

   Collective on EC

   Output Parameter:
.  ec - eigenvalue computation context

.keywords: EC, set, eigenvectors, required

.seealso: ECSetUp(), ECSolve(), ECDestroy(), ECSolveEigenvectors(),
          ECGetEigenvectors()
@*/
int ECSetEigenvectorsRequired(EC ec)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);
  ec->computeeigenvectors = 1;
  PetscFunctionReturn(0);
}

#include "src/sys/nreg.h"
static NRList *__ECList = 0;
#undef __FUNC__  
#define __FUNC__ "ECCreate"
/*@C
   ECCreate - Creates the default EC context.

   Collective on MPI_Comm

   Output Parameter:
+  ec - location to put the EC context
.  pt - either EC_EIGENVALUE or EC_GENERALIZED_EIGENVALUE
-  comm - MPI communicator

.keywords: EC, create, context

.seealso: ECSetUp(), ECSolve(), ECDestroy(), ECRegister()
@*/
int ECCreate(MPI_Comm comm,ECProblemType pt,EC *ec)
{
  int ierr;
  EC  ctx;

  PetscFunctionBegin;
  *ec = 0;
  PetscHeaderCreate(ctx,_p_EC,int,EC_COOKIE,EC_LAPACK,comm,ECDestroy,ECView);
  PLogObjectCreate(ctx);
  *ec                = ctx;
  ctx->view          = 0;
  ctx->type          = (ECType) -1;
  ctx->problemtype   = pt;
  ctx->solve         = 0;
  ctx->setup         = 0;
  ctx->destroy       = 0;

  ctx->computeeigenvectors = 0;

  ctx->data          = 0;
  ctx->nwork         = 0;
  ctx->work          = 0;

  ctx->n             = 0;
  ctx->realpart      = 0;
  ctx->cnvP          = 0;

  ctx->setupcalled   = 0;
  /* this violates our rule about separating abstract from implementations */
  ierr = ECSetType(*ec,EC_LAPACK);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSetFromOptions"
/*@
   ECSetFromOptions - Set EC options from the options database.

   Collective on EC

   Input Parameter:
.  ec - the eigenvalue computation context

   Options Database Commands:
+  -ec_type  <type> - Sets EC type; use -help for a list of available methods
.  -ec_spectrum_portion <portion> - Specifies portion of spectrum of interest.
           Options include [largest_real_part,largest_magnitude, 
           smallest_real_part,smallest_magnitude, interior]
-  -ec_spectrum_number - Specifies number of eigenvalues requested

.keywords: EC, set, options

.seealso: ECCreate(), ECSetType()
@*/
int ECSetFromOptions(EC ec)
{
  int  ierr,flag;
  char spectrum[128];

  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);

  ierr = OptionsGetString(ec->prefix,"-ec_spectrum_portion",spectrum,128,&flag);CHKERRQ(ierr);
  if (flag) {
    if (!PetscStrcmp(spectrum,"largest_real_part")) {
      PLogInfo(ec,"Computing largest real part of spectrum");
      ec->spectrumportion = EC_LARGEST_REAL_PART;
    } else if (!PetscStrcmp(spectrum,"largest_magnitude")) {
      PLogInfo(ec,"Computing largest magnitude of spectrum");
      ec->spectrumportion = EC_LARGEST_MAGNITUDE;
    } else if (!PetscStrcmp(spectrum,"smallest_real_part")) {
      PLogInfo(ec,"Computing smallest real part of spectrum");
      ec->spectrumportion = EC_SMALLEST_REAL_PART;
    } else if (!PetscStrcmp(spectrum,"smallest_magnitude")) {
      PLogInfo(ec,"Computing smallest magnitude of spectrum");
      ec->spectrumportion = EC_SMALLEST_MAGNITUDE;
    } else if (!PetscStrcmp(spectrum,"interior")) {
      PLogInfo(ec,"Computing interior spectrum");
      ec->spectrumportion = EC_INTERIOR;
      ierr = OptionsGetScalar(ec->prefix,"-ec_spectrum_location",&ec->location,&flag);
             CHKERRQ(ierr);
      if (!flag) SETERRQ(PETSC_ERR_ARG_WRONGSTATE,1,"Must set interior spectrum location");   
    } else {
      SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,1,"Unknown spectrum request");
    }
  }
  ierr = OptionsGetInt(ec->prefix,"-ec_spectrum_number",&ec->n,&flag);CHKERRQ(ierr);

  if (ec->setfromoptions) {
    ierr = (*ec->setfromoptions)(ec); CHKERRQ(ierr);
  }
  ierr = OptionsHasName(0,"-help",&flag); CHKERRQ(ierr);
  if (flag) {
    ierr = ECPrintHelp(ec); CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECPrintHelp" 
/*@
   ECPrintHelp - Prints a help message about the eigenvalue computations.

   Collective on EC

   Input Parameter:
.  ec - the eigenvalue computation context

   Options Database Command:
+  -help - PrintsEC options
-  -h - Prints EC options

.keywords: EC, print, help
@*/
int ECPrintHelp(EC ec)
{
  int  ierr;
  char p[64]; 

  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);
  PetscStrcpy(p,"-");
  if (ec->prefix) PetscStrcat(p,ec->prefix);

  (*PetscHelpPrintf)(ec->comm,"EC options --------------------------------------------------\n");
  ierr = NRPrintTypes(ec->comm,stdout,ec->prefix,"ex_type",__ECList);CHKERRQ(ierr);
  (*PetscHelpPrintf)(ec->comm,"  %sec_view: print information on solvers used for eigenvalues\n",p);
  (*PetscHelpPrintf)(ec->comm,"  %sec_view_eigenvalues: print eigenvalues to screen\n",p);
  (*PetscHelpPrintf)(ec->comm,"  %sec_view_eigenvalues_draw: plot eigenvalues to screen\n",p);
  (*PetscHelpPrintf)(ec->comm,"  %sec_spectrum_number <n>: number of eigenvalues to compute\n",p);
  (*PetscHelpPrintf)(ec->comm,"  %sec_spectrum_portion <largest_real,largest_magnitude,smallest_real\n",p);
  (*PetscHelpPrintf)(ec->comm,"                smallest_magnitude,interior>: specify spectrum portion\n");
  (*PetscHelpPrintf)(ec->comm,"  %sec_spectrum_location <location>: find eigenvalues nearby.\n",p);
  (*PetscHelpPrintf)(ec->comm,"                Use with interior portion (listed above).\n");
  if (ec->printhelp) {
    ierr = (*ec->printhelp)(ec,p); CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSetOperators" 
/*@
   ECSetOperators - Sets the operators for which eigenvalues are to be computed.

   Collective on EC

   Input Parameter:
+  ec - the eigenvalue computation context
.  A  - the matrix for which eigenvalues are requested
-  B  - optional matrix for generalized eigenvalue problem

.keywords: EC, set, operators

.seealso: ECCreate()
@*/
int ECSetOperators(EC ec,Mat A,Mat B)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);
  PetscValidHeaderSpecific(A,MAT_COOKIE);
  if (B) {
    PetscValidHeaderSpecific(B,MAT_COOKIE);
    if (ec->problemtype == EC_GENERALIZED_EIGENVALUE) {
      SETERRQ(PETSC_ERR_ARG_WRONG,1,"Cannot set B operator for standard eigenvalue problem");
    }
  } else if (ec->problemtype == EC_GENERALIZED_EIGENVALUE) {
    SETERRQ(PETSC_ERR_ARG_WRONG,1,"Must set B operator for generalized eigenvalue problem");
  }
  ec->A = A;
  ec->B = B;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSetSpectrumPortion"
/*@
   ECSetSpectrumPortion - Sets the portion of the spectrum from which 
   eigenvalues will be computed.

   Collective on EC

   Input Parameters:
+  ec - the eigenvalue computation context
.  n - number of eigenvalues requested
.  portion - portion of the spectrum (see below)
- location - value near which you wish the spectrum computed

   Possible Portions of Spectrum:
+  EC_LARGEST_REAL_PART - largest real part
.  EC_LARGEST_MAGNITUDE - largest magnitude
.  EC_SMALLEST_REAL_PART - smalles real part
.  EC_SMALLEST_MAGNITUDE - smallest magnitude
-  EC_INTERIOR - interior

   Options Database Keys:
+  -ec_spectrum_portion <portion> - Specifies portion of spectrum of interest.
           Options include [largest_real_part,largest_magnitude, 
           smallest_real_part,smallest_magnitude, interior]
-  -ec_spectrum_number - Specifies number of eigenvalues requested

.keywords: EC, set, spectrum, portion

.seealso: ECCreate(), ECSetOperators()
@*/
int ECSetEigenvaluePortion(EC ec,int n,ECSpectrumPortion portion,Scalar location)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);

  ec->n               = n;
  ec->spectrumportion = portion;
  ec->location        = location;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSolve"
/*@
   ECSolve - Computes the appropriate eigenvalues.

   Collective on EC

   Input Parameter:
.  ec - the eigenvalue computation context

.keywords: EC, solve

.seealso: ECCreate(), ECSetOperators(), ECGetEigenvalues(),
          ECSolveEigenvectors()
@*/
int ECSolve(EC ec)
{
  int ierr,flag;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);

  PLogEventBegin(EC_Solve,ec,0,0,0); 
  ierr = (*ec->solve)(ec); CHKERRQ(ierr);
  PLogEventEnd(EC_Solve,ec,0,0,0); 

  ierr = OptionsHasName(ec->prefix,"-ec_view_eigenvalues",&flag);CHKERRQ(ierr);
  if (flag) {
    double *rpart,*ipart;
    int    i,n,rank;
    MPI_Comm_rank(ec->comm,&rank);
    if (!rank) {
      ierr = ECGetEigenvalues(ec,&n,&rpart,&ipart); CHKERRQ(ierr);
      printf("%d eigenvalues computed\n",n);
      for ( i=0; i<n; i++ ) {
        if (ipart[i] >= 0.0) printf("%d %g + %gi\n",i,rpart[i],ipart[i]);
        else                 printf("%d %g - %gi\n",i,rpart[i],-ipart[i]);
      }
    }
  }  
  ierr = OptionsHasName(ec->prefix,"-ec_view_eigenvalues_draw",&flag);CHKERRQ(ierr);
  if (flag) {
    double *rpart,*ipart;
    int    i,n,rank;
    MPI_Comm_rank(ec->comm,&rank);
    if (!rank) {
      Viewer    viewer;
      Draw      draw;
      DrawSP    drawsp;

      ierr = ECGetEigenvalues(ec,&n,&rpart,&ipart); CHKERRQ(ierr);
      ierr = ViewerDrawOpenX(PETSC_COMM_SELF,0,"Eigenvalues",PETSC_DECIDE,PETSC_DECIDE,
                             300,300,&viewer); CHKERRQ(ierr);
      ierr = ViewerDrawGetDraw(viewer,&draw); CHKERRQ(ierr);
      ierr = DrawSPCreate(draw,1,&drawsp); CHKERRQ(ierr);
      for ( i=0; i<n; i++ ) {
        ierr = DrawSPAddPoint(drawsp,rpart+i,ipart+i); CHKERRQ(ierr);
      }
      ierr = DrawSPDraw(drawsp); CHKERRQ(ierr);
      ierr = DrawSPDestroy(drawsp); CHKERRQ(ierr);
      ierr = ViewerDestroy(viewer); CHKERRQ(ierr);
    }
  }  
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSolveEigenvectors"
/*@
   ECSolveEigenvectors - Computes the appropriate eigenvectors

   Collective on EC

   Input Parameter:
.  ec - the eigenvalue computation context

   Notes: Must be called after ECSolve().

.keywords: EC, solve, eigenvalues

.seealso: ECCreate(), ECSetOperators(), ECSolve(), ECGetEigenVectors(),
          ECSetEigenvectorsRequired()
@*/
int ECSolveEigenvectors(EC ec)
{
  int ierr;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);

  if (!ec->computeeigenvectors) {
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE,1,"Must call ECSetEigenvectorsRequired() before this call");
  }

  /* PLogEventBegin(EC_SolveEigenvectors,ec,0,0,0);  */
  ierr = (*ec->solveeigenvectors)(ec); CHKERRQ(ierr);
  /* PLogEventEnd(EC_SolveEigenvectors,ec,0,0,0);  */
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECSetType" 
/*@
   ECSetType - Builds EC for a particular solver. 

   Collective on EC

   Input Parameter:
+  ctx      - the eigenvalue computation context
-  itmethod - a known method

   Options Database Command:
.  -ec_type <type> - Specified EC method; use -help for a list of available methods

   Notes:  
   See "petsc/include/ec.h" for available methods (for instance,
   EC_LAPACK, eventually others)

   Normally, it is best to use the ECSetFromOptions() command and
   then set the EC type from the options database rather than by using
   this routine.  Using the options database provides the user with
   maximum flexibility in evaluating the many different Krylov methods.
   The ECSetType() routine is provided for those situations where it
   is necessary to set the iterative solver independently of the command
   line or options database.  This might be the case, for example, when
   the choice of iterative solver changes during the execution of the
   program, and the user's application is taking responsibility for
   choosing the appropriate method.  In other words, this routine is
   for the advanced user.

.keywords: EC, set, type

.seealso: ECCreate(), ECSetFromOptions()
@*/
int ECSetType(EC ec,ECType itmethod)
{
  int ierr,(*r)(EC);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(ec,EC_COOKIE);
  if (ec->type == (int) itmethod) PetscFunctionReturn(0);

  if (ec->setupcalled) {
    /* destroy the old private EC context */
    ierr = (*(ec)->ops->destroy)(ec); CHKERRQ(ierr);
    ec->data = 0;
  }
  /* Get the function pointers for the approach requested */
  if (!__ECList) {ierr = ECRegisterAll(); CHKERRQ(ierr);}
  r =  (int (*)(EC))NRFindRoutine( __ECList, (int)itmethod, (char *)0 );
  if (!r) {SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,0,"Unknown method");}
  if (ec->data) PetscFree(ec->data);
  ec->data = 0;
  ierr = (*r)(ec);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECRegister" 
/*@C
   ECRegister - Adds the iterative method to the EC package,  given
   an iterative name (ECType) and a function pointer.

   Not Collective

   Input Parameters:
+  name   - for instance ECCG, ECGMRES, ...
.  sname  - corresponding string for name
-  create - routine to create method context

   Notes:
   ECRegister() has NOT yet been updated to use the new dynamic linking
   registration approach, though eventually it will be.

.keywords: EC, register

.seealso: ECRegisterAll(), ECRegisterDestroy(), ECDestroy(), ECCreate()
@*/
int  ECRegister(ECType name, char *sname, int  (*create)(EC))
{
  int ierr;
  int (*dummy)(void *) = (int (*)(void *)) create;
  PetscFunctionBegin;
  if (!__ECList) {ierr = NRCreate(&__ECList); CHKERRQ(ierr);}
  ierr = NRRegister( __ECList, (int) name, sname, dummy );CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECRegisterDestroy" 
/*@C
   ECRegisterDestroy - Frees the list of EC methods that were
   registered by ECRegister().

   Not Collective

.keywords: EC, register, destroy

.seealso: ECRegister(), ECRegisterAll()
@*/
int ECRegisterDestroy(void)
{
  PetscFunctionBegin;
  if (__ECList) {
    NRDestroy( __ECList );
    __ECList = 0;
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "ECGetType" 
/*@C
   ECGetType - Gets the EC type and method name (as a string) from 
   the method type.

   Not Collective

   Input Parameter:
.  ec - Krylov context 

   Output Parameters:
+  itmeth - EC method (or use PETSC_NULL)
-  name - name of EC method (or use PETSC_NULL)

.keywords: EC, get, type, name
@*/
int ECGetType(EC ec,ECType *type,char **name)
{
  int ierr;
  PetscFunctionBegin;
  if (!__ECList) {ierr = ECRegisterAll(); CHKERRQ(ierr);}
  if (type) *type = (ECType) ec->type;
  if (name)  *name = NRFindName( __ECList, (int) ec->type);
  PetscFunctionReturn(0);
}



