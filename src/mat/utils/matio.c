/* 
   This file contains simple binary read/write routines for matrices.
 */

#include "src/mat/matimpl.h"             /*I  "petscmat.h"  I*/
#include "petscsys.h"

#undef __FUNCT__  
#define __FUNCT__ "MatLoadPrintHelp_Private"
static PetscErrorCode MatLoadPrintHelp_Private(Mat A)
{
  static PetscTruth called = PETSC_FALSE; 
  MPI_Comm          comm = A->comm;
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  if (called) {PetscFunctionReturn(0);} else called = PETSC_TRUE;
  ierr = (*PetscHelpPrintf)(comm," Options for MatLoad:\n");CHKERRQ(ierr);
  ierr = (*PetscHelpPrintf)(comm,"  -mat_type <type>\n");CHKERRQ(ierr);
  ierr = (*PetscHelpPrintf)(comm,"  -matload_type <type>\n");CHKERRQ(ierr);
  ierr = (*PetscHelpPrintf)(comm,"  -matload_block_size <block_size> :Used for MATBAIJ, MATBDIAG\n");CHKERRQ(ierr);
  ierr = (*PetscHelpPrintf)(comm,"  -matload_bdiag_diags <s1,s2,s3,...> : Used for MATBDIAG\n");CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatLoad"
/*@C
   MatLoad - Loads a matrix that has been stored in binary format
   with MatView().  The matrix format is determined from the options database.
   Generates a parallel MPI matrix if the communicator has more than one
   processor.  The default matrix type is AIJ.

   Collective on PetscViewer

   Input Parameters:
+  viewer - binary file viewer, created with PetscViewerBinaryOpen()
-  outtype - type of matrix desired, for example MATSEQAIJ,
             MATMPIROWBS, etc.  See types in petsc/include/petscmat.h.

   Output Parameters:
.  newmat - new matrix

   Basic Options Database Keys:
+    -matload_type seqaij   - AIJ type
.    -matload_type mpiaij   - parallel AIJ type
.    -matload_type seqbaij  - block AIJ type
.    -matload_type mpibaij  - parallel block AIJ type
.    -matload_type seqsbaij - block symmetric AIJ type
.    -matload_type mpisbaij - parallel block symmetric AIJ type
.    -matload_type seqbdiag - block diagonal type
.    -matload_type mpibdiag - parallel block diagonal type
.    -matload_type mpirowbs - parallel rowbs type
.    -matload_type seqdense - dense type
.    -matload_type mpidense - parallel dense type
-    -matload_symmetric - matrix in file is symmetric

   More Options Database Keys:
   Used with block matrix formats (MATSEQBAIJ, MATMPIBDIAG, ...) to specify
   block size
.    -matload_block_size <bs>

   Used to specify block diagonal numbers for MATSEQBDIAG and MATMPIBDIAG formats
.    -matload_bdiag_diags <s1,s2,s3,...>

   Level: beginner

   Notes:
   MatLoad() automatically loads into the options database any options
   given in the file filename.info where filename is the name of the file
   that was passed to the PetscViewerBinaryOpen(). The options in the info
   file will be ignored if you use the -matload_ignore_info option.

   In parallel, each processor can load a subset of rows (or the
   entire matrix).  This routine is especially useful when a large
   matrix is stored on disk and only part of it existsis desired on each
   processor.  For example, a parallel solver may access only some of
   the rows from each processor.  The algorithm used here reads
   relatively small blocks of data rather than reading the entire
   matrix and then subsetting it.

   Notes for advanced users:
   Most users should not need to know the details of the binary storage
   format, since MatLoad() and MatView() completely hide these details.
   But for anyone who's interested, the standard binary matrix storage
   format is

$    int    MAT_FILE_COOKIE
$    int    number of rows
$    int    number of columns
$    int    total number of nonzeros
$    int    *number nonzeros in each row
$    int    *column indices of all nonzeros (starting index is zero)
$    PetscScalar *values of all nonzeros

   Note for Cray users, the int's stored in the binary file are 32 bit
integers; not 64 as they are represented in the memory, so if you
write your own routines to read/write these binary files from the Cray
you need to adjust the integer sizes that you read in, see
PetscReadBinary() and PetscWriteBinary() to see how this may be
done.

   In addition, PETSc automatically does the byte swapping for
machines that store the bytes reversed, e.g.  DEC alpha, freebsd,
linux, Windows and the paragon; thus if you write your own binary
read/write routines you have to swap the bytes; see PetscReadBinary()
and PetscWriteBinary() to see how this may be done.

.keywords: matrix, load, binary, input

.seealso: PetscViewerBinaryOpen(), MatView(), VecLoad()

 @*/  
PetscErrorCode MatLoad(PetscViewer viewer,const MatType outtype,Mat *newmat)
{
  Mat         factory;
  PetscErrorCode ierr;
  PetscTruth  isbinary,flg;
  MPI_Comm    comm;
  PetscErrorCode (*r)(PetscViewer,const MatType,Mat*);
  char        mtype[256],*prefix;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(viewer,PETSC_VIEWER_COOKIE,1);
  PetscValidPointer(newmat,3);
  *newmat = 0;

  ierr = PetscObjectGetOptionsPrefix((PetscObject)viewer,&prefix);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_BINARY,&isbinary);CHKERRQ(ierr);
  if (!isbinary) {
    SETERRQ(PETSC_ERR_ARG_WRONG,"Invalid viewer; open viewer with PetscViewerBinaryOpen()");
  }

  ierr = PetscOptionsGetString(prefix,"-mat_type",mtype,256,&flg);CHKERRQ(ierr);
  if (flg) {
    outtype = mtype;
  }
  ierr = PetscOptionsGetString(prefix,"-matload_type",mtype,256,&flg);CHKERRQ(ierr);
  if (flg) {
    outtype = mtype;
  }
  if (!outtype) outtype = MATAIJ;

  ierr = PetscObjectGetComm((PetscObject)viewer,&comm);CHKERRQ(ierr);  
  ierr = MatCreate(comm,0,0,0,0,&factory);CHKERRQ(ierr);
  ierr = MatSetType(factory,outtype);CHKERRQ(ierr);
  r = factory->ops->load;
  ierr = MatDestroy(factory);
  if (!r) SETERRQ1(PETSC_ERR_SUP,"MatLoad is not supported for type: %s",outtype);

  ierr = PetscLogEventBegin(MAT_Load,viewer,0,0,0);CHKERRQ(ierr);
  ierr = (*r)(viewer,outtype,newmat);CHKERRQ(ierr);
  ierr = PetscLogEventEnd(MAT_Load,viewer,0,0,0);CHKERRQ(ierr);

  ierr = PetscOptionsHasName(prefix,"-matload_symmetric",&flg);CHKERRQ(ierr);
  if (flg) {
    ierr = MatSetOption(*newmat,MAT_SYMMETRIC);CHKERRQ(ierr);
    ierr = MatSetOption(*newmat,MAT_SYMMETRY_ETERNAL);CHKERRQ(ierr);
  }
  ierr = PetscOptionsHasName(PETSC_NULL,"-help",&flg);CHKERRQ(ierr);
  if (flg) {ierr = MatLoadPrintHelp_Private(*newmat);CHKERRQ(ierr); }
  PetscFunctionReturn(0);
}

