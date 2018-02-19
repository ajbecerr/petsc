
/*
   This provides a simple shell for Fortran (and C programmers) to
  create a very simple matrix class for use with KSP without coding
  much of anything.
*/

#include <petsc/private/matimpl.h>        /*I "petscmat.h" I*/

struct _MatShellOps {
  /*   0 */
  PetscErrorCode (*mult)(Mat,Vec,Vec);
  /*   5 */
  PetscErrorCode (*multtranspose)(Mat,Vec,Vec);
  /*  10 */
  /*  15 */
  PetscErrorCode (*getdiagonal)(Mat,Vec);
  /*  20 */
  /*  24 */
  /*  29 */
  /*  34 */
  /*  39 */
  PetscErrorCode (*axpy)(Mat,PetscScalar,Mat,MatStructure);
  PetscErrorCode (*copy)(Mat,Mat,MatStructure);
  /*  44 */
  PetscErrorCode (*diagonalset)(Mat,Vec,InsertMode);
  /*  49 */
  /*  54 */
  /*  59 */
  PetscErrorCode (*destroy)(Mat);
  /*  64 */
  /*  69 */
  /*  74 */
  /*  79 */
  /*  84 */
  /*  89 */
  /*  94 */
  /*  99 */
  /* 104 */
  /* 109 */
  /* 114 */
  /* 119 */
  /* 124 */
  /* 129 */
  /* 134 */
  /* 139 */
  /* 144 */
};

typedef struct {
  struct _MatShellOps ops[1];

  PetscScalar vscale,vshift;
  Vec         dshift;
  Vec         left,right;
  Vec         left_work,right_work;
  Vec         left_add_work,right_add_work;
  Mat         axpy;
  PetscScalar axpy_vscale;
  PetscBool   managescalingshifts;                   /* The user will manage the scaling and shifts for the MATSHELL, not the default */
  void        *ctx;
} Mat_Shell;

/*
      xx = diag(left)*x
*/
static PetscErrorCode MatShellPreScaleLeft(Mat A,Vec x,Vec *xx)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  *xx = NULL;
  if (!shell->left) {
    *xx = x;
  } else {
    if (!shell->left_work) {ierr = VecDuplicate(shell->left,&shell->left_work);CHKERRQ(ierr);}
    ierr = VecPointwiseMult(shell->left_work,x,shell->left);CHKERRQ(ierr);
    *xx  = shell->left_work;
  }
  PetscFunctionReturn(0);
}

/*
     xx = diag(right)*x
*/
static PetscErrorCode MatShellPreScaleRight(Mat A,Vec x,Vec *xx)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  *xx = NULL;
  if (!shell->right) {
    *xx = x;
  } else {
    if (!shell->right_work) {ierr = VecDuplicate(shell->right,&shell->right_work);CHKERRQ(ierr);}
    ierr = VecPointwiseMult(shell->right_work,x,shell->right);CHKERRQ(ierr);
    *xx  = shell->right_work;
  }
  PetscFunctionReturn(0);
}

/*
    x = diag(left)*x
*/
static PetscErrorCode MatShellPostScaleLeft(Mat A,Vec x)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (shell->left) {ierr = VecPointwiseMult(x,x,shell->left);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}

/*
    x = diag(right)*x
*/
static PetscErrorCode MatShellPostScaleRight(Mat A,Vec x)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (shell->right) {ierr = VecPointwiseMult(x,x,shell->right);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}

/*
         Y = vscale*Y + diag(dshift)*X + vshift*X

         On input Y already contains A*x
*/
static PetscErrorCode MatShellShiftAndScale(Mat A,Vec X,Vec Y)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (shell->dshift) {          /* get arrays because there is no VecPointwiseMultAdd() */
    PetscInt          i,m;
    const PetscScalar *x,*d;
    PetscScalar       *y;
    ierr = VecGetLocalSize(X,&m);CHKERRQ(ierr);
    ierr = VecGetArrayRead(shell->dshift,&d);CHKERRQ(ierr);
    ierr = VecGetArrayRead(X,&x);CHKERRQ(ierr);
    ierr = VecGetArray(Y,&y);CHKERRQ(ierr);
    for (i=0; i<m; i++) y[i] = shell->vscale*y[i] + d[i]*x[i];
    ierr = VecRestoreArrayRead(shell->dshift,&d);CHKERRQ(ierr);
    ierr = VecRestoreArrayRead(X,&x);CHKERRQ(ierr);
    ierr = VecRestoreArray(Y,&y);CHKERRQ(ierr);
  } else {
    ierr = VecScale(Y,shell->vscale);CHKERRQ(ierr);
  }
  if (shell->vshift != 0.0) {ierr = VecAXPY(Y,shell->vshift,X);CHKERRQ(ierr);} /* if test is for non-square matrices */
  PetscFunctionReturn(0);
}

/*@
    MatShellGetContext - Returns the user-provided context associated with a shell matrix.

    Not Collective

    Input Parameter:
.   mat - the matrix, should have been created with MatCreateShell()

    Output Parameter:
.   ctx - the user provided context

    Level: advanced

   Fortran Notes: To use this from Fortran you must write a Fortran interface definition for this
    function that tells Fortran the Fortran derived data type that you are passing in as the ctx argument.

.keywords: matrix, shell, get, context

.seealso: MatCreateShell(), MatShellSetOperation(), MatShellSetContext()
@*/
PetscErrorCode  MatShellGetContext(Mat mat,void *ctx)
{
  PetscErrorCode ierr;
  PetscBool      flg;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_CLASSID,1);
  PetscValidPointer(ctx,2);
  ierr = PetscObjectTypeCompare((PetscObject)mat,MATSHELL,&flg);CHKERRQ(ierr);
  if (flg) *(void**)ctx = ((Mat_Shell*)(mat->data))->ctx;
  else SETERRQ(PetscObjectComm((PetscObject)mat),PETSC_ERR_SUP,"Cannot get context from non-shell matrix");
  PetscFunctionReturn(0);
}

PetscErrorCode MatDestroy_Shell(Mat mat)
{
  PetscErrorCode ierr;
  Mat_Shell      *shell = (Mat_Shell*)mat->data;

  PetscFunctionBegin;
  if (shell->ops->destroy) {
    ierr = (*shell->ops->destroy)(mat);CHKERRQ(ierr);
  }
  ierr = VecDestroy(&shell->left);CHKERRQ(ierr);
  ierr = VecDestroy(&shell->right);CHKERRQ(ierr);
  ierr = VecDestroy(&shell->dshift);CHKERRQ(ierr);
  ierr = VecDestroy(&shell->left_work);CHKERRQ(ierr);
  ierr = VecDestroy(&shell->right_work);CHKERRQ(ierr);
  ierr = VecDestroy(&shell->left_add_work);CHKERRQ(ierr);
  ierr = VecDestroy(&shell->right_add_work);CHKERRQ(ierr);
  ierr = MatDestroy(&shell->axpy);CHKERRQ(ierr);
  ierr = PetscFree(mat->data);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode MatCopy_Shell(Mat A,Mat B,MatStructure str)
{
  Mat_Shell       *shellA = (Mat_Shell*)A->data,*shellB = (Mat_Shell*)B->data;
  PetscErrorCode  ierr;
  PetscBool       matflg;

  PetscFunctionBegin;
  ierr = PetscObjectTypeCompare((PetscObject)B,MATSHELL,&matflg);CHKERRQ(ierr);
  if (!matflg) SETERRQ(PetscObjectComm((PetscObject)A),PETSC_ERR_ARG_NOTSAMETYPE,"Output matrix must be a MATSHELL");

  ierr = PetscMemcpy(B->ops,A->ops,sizeof(struct _MatOps));CHKERRQ(ierr);
  ierr = PetscMemcpy(shellB->ops,shellA->ops,sizeof(struct _MatShellOps));CHKERRQ(ierr);

  if (shellA->ops->copy) {
    ierr = (*shellA->ops->copy)(A,B,str);CHKERRQ(ierr);
  }
  shellB->vscale = shellA->vscale;
  shellB->vshift = shellA->vshift;
  if (shellA->dshift) {
    if (!shellB->dshift) {
      ierr = VecDuplicate(shellA->dshift,&shellB->dshift);CHKERRQ(ierr);
    }
    ierr = VecCopy(shellA->dshift,shellB->dshift);CHKERRQ(ierr);
  } else {
    ierr = VecDestroy(&shellB->dshift);CHKERRQ(ierr);
  }
  if (shellA->left) {
    if (!shellB->left) {
      ierr = VecDuplicate(shellA->left,&shellB->left);CHKERRQ(ierr);
    }
    ierr = VecCopy(shellA->left,shellB->left);CHKERRQ(ierr);
  } else {
    ierr = VecDestroy(&shellB->left);CHKERRQ(ierr);
  }
  if (shellA->right) {
    if (!shellB->right) {
      ierr = VecDuplicate(shellA->right,&shellB->right);CHKERRQ(ierr);
    }
    ierr = VecCopy(shellA->right,shellB->right);CHKERRQ(ierr);
  } else {
    ierr = VecDestroy(&shellB->right);CHKERRQ(ierr);
  }
  ierr = MatDestroy(&shellB->axpy);CHKERRQ(ierr);
  if (shellA->axpy) {
    ierr                 = PetscObjectReference((PetscObject)shellA->axpy);CHKERRQ(ierr);
    shellB->axpy        = shellA->axpy;
    shellB->axpy_vscale = shellA->axpy_vscale;
  }
  PetscFunctionReturn(0);
}

PetscErrorCode MatDuplicate_Shell(Mat mat,MatDuplicateOption op,Mat *M)
{
  PetscErrorCode ierr;
  void           *ctx;

  PetscFunctionBegin;
  ierr = MatShellGetContext(mat,&ctx);CHKERRQ(ierr);
  ierr = MatCreateShell(PetscObjectComm((PetscObject)mat),mat->rmap->n,mat->cmap->n,mat->rmap->N,mat->cmap->N,ctx,M);CHKERRQ(ierr);
  ierr = MatCopy(mat,*M,SAME_NONZERO_PATTERN);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode MatMult_Shell(Mat A,Vec x,Vec y)
{
  Mat_Shell        *shell = (Mat_Shell*)A->data;
  PetscErrorCode   ierr;
  Vec              xx;
  PetscObjectState instate,outstate;

  PetscFunctionBegin;
  ierr = MatShellPreScaleRight(A,x,&xx);CHKERRQ(ierr);
  ierr = PetscObjectStateGet((PetscObject)y, &instate);CHKERRQ(ierr);
  ierr = (*shell->ops->mult)(A,xx,y);CHKERRQ(ierr);
  ierr = PetscObjectStateGet((PetscObject)y, &outstate);CHKERRQ(ierr);
  if (instate == outstate) {
    /* increase the state of the output vector since the user did not update its state themself as should have been done */
    ierr = PetscObjectStateIncrease((PetscObject)y);CHKERRQ(ierr);
  }
  ierr = MatShellShiftAndScale(A,xx,y);CHKERRQ(ierr);
  ierr = MatShellPostScaleLeft(A,y);CHKERRQ(ierr);
  
  if (shell->axpy) {
    if (!shell->left_work) {ierr = MatCreateVecs(A,&shell->left_work,NULL);CHKERRQ(ierr);}
    ierr = MatMult(shell->axpy,x,shell->left_work);CHKERRQ(ierr);
    ierr = VecAXPY(y,shell->axpy_vscale,shell->left_work);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

PetscErrorCode MatMultAdd_Shell(Mat A,Vec x,Vec y,Vec z)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (y == z) {
    if (!shell->right_add_work) {ierr = VecDuplicate(z,&shell->right_add_work);CHKERRQ(ierr);}
    ierr = MatMult(A,x,shell->right_add_work);CHKERRQ(ierr);
    ierr = VecAXPY(z,1.0,shell->right_add_work);CHKERRQ(ierr);
  } else {
    ierr = MatMult(A,x,z);CHKERRQ(ierr);
    ierr = VecAXPY(z,1.0,y);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

PetscErrorCode MatMultTranspose_Shell(Mat A,Vec x,Vec y)
{
  Mat_Shell        *shell = (Mat_Shell*)A->data;
  PetscErrorCode   ierr;
  Vec              xx;
  PetscObjectState instate,outstate;

  PetscFunctionBegin;
  ierr = MatShellPreScaleLeft(A,x,&xx);CHKERRQ(ierr);
  ierr = PetscObjectStateGet((PetscObject)y, &instate);CHKERRQ(ierr);
  if (!shell->ops->multtranspose) SETERRQ(PetscObjectComm((PetscObject)A),PETSC_ERR_ARG_WRONGSTATE,"Have not provided a MatMultTranspose() for this MATSHELL");
  ierr = (*shell->ops->multtranspose)(A,xx,y);CHKERRQ(ierr);
  ierr = PetscObjectStateGet((PetscObject)y, &outstate);CHKERRQ(ierr);
  if (instate == outstate) {
    /* increase the state of the output vector since the user did not update its state themself as should have been done */
    ierr = PetscObjectStateIncrease((PetscObject)y);CHKERRQ(ierr);
  }
  ierr = MatShellShiftAndScale(A,xx,y);CHKERRQ(ierr);
  ierr = MatShellPostScaleRight(A,y);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

PetscErrorCode MatMultTransposeAdd_Shell(Mat A,Vec x,Vec y,Vec z)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (y == z) {
    if (!shell->left_add_work) {ierr = VecDuplicate(z,&shell->left_add_work);CHKERRQ(ierr);}
    ierr = MatMultTranspose(A,x,shell->left_add_work);CHKERRQ(ierr);
    ierr = VecWAXPY(z,1.0,shell->left_add_work,y);CHKERRQ(ierr);
  } else {
    ierr = MatMultTranspose(A,x,z);CHKERRQ(ierr);
    ierr = VecAXPY(z,1.0,y);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

/*
          diag(left)(vscale*A + diag(dshift) + vshift I)diag(right)
*/
PetscErrorCode MatGetDiagonal_Shell(Mat A,Vec v)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (shell->ops->getdiagonal) {
    ierr = (*shell->ops->getdiagonal)(A,v);CHKERRQ(ierr);
  } else {
    ierr = VecSet(v,0.0);CHKERRQ(ierr);
  }
  ierr = VecScale(v,shell->vscale);CHKERRQ(ierr);
  if (shell->dshift) {
    ierr = VecAXPY(v,1.0,shell->dshift);CHKERRQ(ierr);
  }
  ierr = VecShift(v,shell->vshift);CHKERRQ(ierr);
  if (shell->left)  {ierr = VecPointwiseMult(v,v,shell->left);CHKERRQ(ierr);}
  if (shell->right) {ierr = VecPointwiseMult(v,v,shell->right);CHKERRQ(ierr);}
  if (shell->axpy) {
    if (!shell->left_work) {ierr = VecDuplicate(v,&shell->left_work);CHKERRQ(ierr);}
    ierr = MatGetDiagonal(shell->axpy,shell->left_work);CHKERRQ(ierr);
    ierr = VecAXPY(v,shell->axpy_vscale,shell->left_work);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

PetscErrorCode MatShift_Shell(Mat Y,PetscScalar a)
{
  Mat_Shell      *shell = (Mat_Shell*)Y->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (shell->left || shell->right) {
    if (!shell->dshift) {
      ierr = VecDuplicate(shell->left ? shell->left : shell->right, &shell->dshift);CHKERRQ(ierr);
      ierr = VecSet(shell->dshift,a);CHKERRQ(ierr);
    } else {
      if (shell->left)  {ierr = VecPointwiseMult(shell->dshift,shell->dshift,shell->left);CHKERRQ(ierr);}
      if (shell->right) {ierr = VecPointwiseMult(shell->dshift,shell->dshift,shell->right);CHKERRQ(ierr);}
      ierr = VecShift(shell->dshift,a);CHKERRQ(ierr);
    }
    if (shell->left)  {ierr = VecPointwiseDivide(shell->dshift,shell->dshift,shell->left);CHKERRQ(ierr);}
    if (shell->right) {ierr = VecPointwiseDivide(shell->dshift,shell->dshift,shell->right);CHKERRQ(ierr);}
  } else shell->vshift += a;
  PetscFunctionReturn(0);
}

PetscErrorCode MatDiagonalSet_Shell(Mat A,Vec D,InsertMode ins)
{
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (ins == INSERT_VALUES) SETERRQ(PetscObjectComm((PetscObject)A),PETSC_ERR_ARG_WRONGSTATE, "Operation not supported with INSERT_VALUES");
  if (!shell->dshift) {ierr = VecDuplicate(D,&shell->dshift);CHKERRQ(ierr);}
  if (shell->left || shell->right) {
    if (!shell->right_work) {ierr = VecDuplicate(shell->left ? shell->left : shell->right, &shell->right_work);CHKERRQ(ierr);}
    if (shell->left && shell->right)  {
      ierr = VecPointwiseDivide(shell->right_work,D,shell->left);CHKERRQ(ierr);
      ierr = VecPointwiseDivide(shell->right_work,shell->right_work,shell->right);CHKERRQ(ierr);
    } else if (shell->left) {
      ierr = VecPointwiseDivide(shell->right_work,D,shell->left);CHKERRQ(ierr);
    } else {
      ierr = VecPointwiseDivide(shell->right_work,D,shell->right);CHKERRQ(ierr);
    }
    if (!shell->dshift) {
      ierr = VecDuplicate(shell->left ? shell->left : shell->right, &shell->dshift);CHKERRQ(ierr);
      ierr = VecCopy(shell->dshift,shell->right_work);CHKERRQ(ierr);
    } else {
      ierr = VecAXPY(shell->dshift,1.0,shell->right_work);CHKERRQ(ierr);
    }
  } else {
    ierr = VecAXPY(shell->dshift,1.0,D);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

PetscErrorCode MatScale_Shell(Mat Y,PetscScalar a)
{
  Mat_Shell      *shell = (Mat_Shell*)Y->data;
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  shell->vscale *= a;
  shell->vshift *= a;
  if (shell->dshift) {
    ierr = VecScale(shell->dshift,a);CHKERRQ(ierr);
  }
  shell->axpy_vscale *= a;
  PetscFunctionReturn(0);
}

static PetscErrorCode MatDiagonalScale_Shell(Mat Y,Vec left,Vec right)
{
  Mat_Shell      *shell = (Mat_Shell*)Y->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (shell->axpy) SETERRQ(PetscObjectComm((PetscObject)Y),PETSC_ERR_SUP,"Cannot diagonal scale MATSHELL after MatAXPY operation");
  if (left) {
    if (!shell->left) {
      ierr = VecDuplicate(left,&shell->left);CHKERRQ(ierr);
      ierr = VecCopy(left,shell->left);CHKERRQ(ierr);
    } else {
      ierr = VecPointwiseMult(shell->left,shell->left,left);CHKERRQ(ierr);
    }
  }
  if (right) {
    if (!shell->right) {
      ierr = VecDuplicate(right,&shell->right);CHKERRQ(ierr);
      ierr = VecCopy(right,shell->right);CHKERRQ(ierr);
    } else {
      ierr = VecPointwiseMult(shell->right,shell->right,right);CHKERRQ(ierr);
    }
  }
  PetscFunctionReturn(0);
}

PetscErrorCode MatAssemblyEnd_Shell(Mat Y,MatAssemblyType t)
{
  Mat_Shell      *shell = (Mat_Shell*)Y->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (t == MAT_FINAL_ASSEMBLY) {
    shell->vshift = 0.0;
    shell->vscale = 1.0;
    ierr = VecDestroy(&shell->dshift);CHKERRQ(ierr);
    ierr = VecDestroy(&shell->left);CHKERRQ(ierr);
    ierr = VecDestroy(&shell->right);CHKERRQ(ierr);
    ierr = MatDestroy(&shell->axpy);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

PETSC_INTERN PetscErrorCode MatConvert_Shell(Mat, MatType,MatReuse,Mat*);

static PetscErrorCode MatMissingDiagonal_Shell(Mat A,PetscBool  *missing,PetscInt *d)
{
  PetscFunctionBegin;
  *missing = PETSC_FALSE;
  PetscFunctionReturn(0);
}

PetscErrorCode MatAXPY_Shell(Mat Y,PetscScalar a,Mat X,MatStructure str)
{
  Mat_Shell      *shell = (Mat_Shell*)Y->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscObjectReference((PetscObject)X);CHKERRQ(ierr);
  ierr = MatDestroy(&shell->axpy);CHKERRQ(ierr);
  shell->axpy        = X;
  shell->axpy_vscale = a;
  PetscFunctionReturn(0);
}

static struct _MatOps MatOps_Values = {0,
                                       0,
                                       0,
                                       0,
                                /* 4*/ MatMultAdd_Shell,
                                       0,
                                       MatMultTransposeAdd_Shell,
                                       0,
                                       0,
                                       0,
                                /*10*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*15*/ 0,
                                       0,
                                       MatGetDiagonal_Shell,
                                       MatDiagonalScale_Shell,
                                       0,
                                /*20*/ 0,
                                       MatAssemblyEnd_Shell,
                                       0,
                                       0,
                                /*24*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*29*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*34*/ MatDuplicate_Shell,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*39*/ MatAXPY_Shell,
                                       0,
                                       0,
                                       0,
                                       MatCopy_Shell,
                                /*44*/ 0,
                                       MatScale_Shell,
                                       MatShift_Shell,
                                       MatDiagonalSet_Shell,
                                       0,
                                /*49*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*54*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*59*/ 0,
                                       MatDestroy_Shell,
                                       0,
                                       0,
                                       0,
                                /*64*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*69*/ 0,
                                       0,
                                       MatConvert_Shell,
                                       0,
                                       0,
                                /*74*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*79*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*84*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*89*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*94*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                                /*99*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                               /*104*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                               /*109*/ 0,
                                       0,
                                       0,
                                       0,
                                       MatMissingDiagonal_Shell,
                               /*114*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                               /*119*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                               /*124*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                               /*129*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                               /*134*/ 0,
                                       0,
                                       0,
                                       0,
                                       0,
                               /*139*/ 0,
                                       0,
                                       0
};

/*MC
   MATSHELL - MATSHELL = "shell" - A matrix type to be used to define your own matrix type -- perhaps matrix free.

  Level: advanced

.seealso: MatCreateShell()
M*/

PETSC_EXTERN PetscErrorCode MatCreate_Shell(Mat A)
{
  Mat_Shell      *b;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscMemcpy(A->ops,&MatOps_Values,sizeof(struct _MatOps));CHKERRQ(ierr);

  ierr    = PetscNewLog(A,&b);CHKERRQ(ierr);
  A->data = (void*)b;

  ierr = PetscLayoutSetUp(A->rmap);CHKERRQ(ierr);
  ierr = PetscLayoutSetUp(A->cmap);CHKERRQ(ierr);

  b->ctx                 = 0;
  b->vshift              = 0.0;
  b->vscale              = 1.0;
  b->managescalingshifts = PETSC_TRUE;
  A->assembled           = PETSC_TRUE;
  A->preallocated        = PETSC_FALSE;

  ierr = PetscObjectChangeTypeName((PetscObject)A,MATSHELL);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*@C
   MatCreateShell - Creates a new matrix class for use with a user-defined
   private data storage format.

  Collective on MPI_Comm

   Input Parameters:
+  comm - MPI communicator
.  m - number of local rows (must be given)
.  n - number of local columns (must be given)
.  M - number of global rows (may be PETSC_DETERMINE)
.  N - number of global columns (may be PETSC_DETERMINE)
-  ctx - pointer to data needed by the shell matrix routines

   Output Parameter:
.  A - the matrix

   Level: advanced

  Usage:
$    extern int mult(Mat,Vec,Vec);
$    MatCreateShell(comm,m,n,M,N,ctx,&mat);
$    MatShellSetOperation(mat,MATOP_MULT,(void(*)(void))mult);
$    [ Use matrix for operations that have been set ]
$    MatDestroy(mat);

   Notes:
   The shell matrix type is intended to provide a simple class to use
   with KSP (such as, for use with matrix-free methods). You should not
   use the shell type if you plan to define a complete matrix class.

   Fortran Notes: To use this from Fortran with a ctx you must write an interface definition for this
    function and for MatShellGetContext() that tells Fortran the Fortran derived data type you are passing
    in as the ctx argument.

   PETSc requires that matrices and vectors being used for certain
   operations are partitioned accordingly.  For example, when
   creating a shell matrix, A, that supports parallel matrix-vector
   products using MatMult(A,x,y) the user should set the number
   of local matrix rows to be the number of local elements of the
   corresponding result vector, y. Note that this is information is
   required for use of the matrix interface routines, even though
   the shell matrix may not actually be physically partitioned.
   For example,

   MATSHELL handles MatShift(), MatDiagonalSet(), MatDiagonalScale(), and MatScale() internally so these 
   operations cannot be overwritten unless MatShellSetManageScalingShifts() is called.

$
$     Vec x, y
$     extern int mult(Mat,Vec,Vec);
$     Mat A
$
$     VecCreateMPI(comm,PETSC_DECIDE,M,&y);
$     VecCreateMPI(comm,PETSC_DECIDE,N,&x);
$     VecGetLocalSize(y,&m);
$     VecGetLocalSize(x,&n);
$     MatCreateShell(comm,m,n,M,N,ctx,&A);
$     MatShellSetOperation(mat,MATOP_MULT,(void(*)(void))mult);
$     MatMult(A,x,y);
$     MatDestroy(A);
$     VecDestroy(y); VecDestroy(x);
$

    For rectangular matrices do all the scalings and shifts make sense?

    Developers Notes: Regarding shifting and scaling. The general form is

          diag(left)(vscale*A + diag(dshift) + vshift I)diag(right)

      The order you apply the operations is important. For example if you have a dshift then
      apply a MatScale(s) you get s*vscale*A + s*diag(shift). But if you first scale and then shift
      you get s*vscale*A + diag(shift)

          A is the user provided function. 

.keywords: matrix, shell, create

.seealso: MatShellSetOperation(), MatHasOperation(), MatShellGetContext(), MatShellSetContext(), MATSHELL, MatShellSetManageScalingShifts()
@*/
PetscErrorCode  MatCreateShell(MPI_Comm comm,PetscInt m,PetscInt n,PetscInt M,PetscInt N,void *ctx,Mat *A)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = MatCreate(comm,A);CHKERRQ(ierr);
  ierr = MatSetSizes(*A,m,n,M,N);CHKERRQ(ierr);
  ierr = MatSetType(*A,MATSHELL);CHKERRQ(ierr);
  ierr = MatShellSetContext(*A,ctx);CHKERRQ(ierr);
  ierr = MatSetUp(*A);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*@
    MatShellSetContext - sets the context for a shell matrix

   Logically Collective on Mat

    Input Parameters:
+   mat - the shell matrix
-   ctx - the context

   Level: advanced

   Fortran Notes: To use this from Fortran you must write a Fortran interface definition for this
    function that tells Fortran the Fortran derived data type that you are passing in as the ctx argument.

.seealso: MatCreateShell(), MatShellGetContext(), MatShellGetOperation()
@*/
PetscErrorCode  MatShellSetContext(Mat mat,void *ctx)
{
  Mat_Shell      *shell = (Mat_Shell*)mat->data;
  PetscErrorCode ierr;
  PetscBool      flg;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)mat,MATSHELL,&flg);CHKERRQ(ierr);
  if (flg) {
    shell->ctx = ctx;
  } else SETERRQ(PetscObjectComm((PetscObject)mat),PETSC_ERR_SUP,"Cannot attach context to non-shell matrix");
  PetscFunctionReturn(0);
}

/*@
    MatShellSetManageScalingShifts - Allows the user to control the scaling and shift operations of the MATSHELL. Must be called immediately
          after MatCreateShell()

   Logically Collective on Mat

    Input Parameter:
.   mat - the shell matrix

  Level: advanced

.seealso: MatCreateShell(), MatShellGetContext(), MatShellGetOperation(), MatShellSetContext(), MatShellSetOperation()
@*/
PetscErrorCode MatShellSetManageScalingShifts(Mat A)
{
  PetscErrorCode ierr;
  Mat_Shell      *shell = (Mat_Shell*)A->data;
  PetscBool      flg;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(A,MAT_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)A,MATSHELL,&flg);CHKERRQ(ierr);
  if (!flg) SETERRQ(PetscObjectComm((PetscObject)A),PETSC_ERR_SUP,"Can only use with MATSHELL matrices");
  shell->managescalingshifts = PETSC_FALSE;
  A->ops->diagonalscale = 0;
  A->ops->scale         = 0;
  A->ops->shift         = 0;
  A->ops->diagonalset   = 0;
  PetscFunctionReturn(0);
}

/*@C
    MatShellTestMult - Compares the multiply routine provided to the MATSHELL with differencing on a given function.

   Logically Collective on Mat

    Input Parameters:
+   mat - the shell matrix
.   f - the function
.   base - differences are computed around this vector, see MatMFFDSetBase(), for Jacobians this is the point at which the Jacobian is being evaluated
-   ctx - an optional context for the function

   Output Parameter:
.   flg - PETSC_TRUE if the multiply is likely correct

   Options Database:
.   -mat_shell_test_mult_view - print if any differences are detected between the products and print the difference

   Level: advanced

   Fortran Notes: Not supported from Fortran

.seealso: MatCreateShell(), MatShellGetContext(), MatShellGetOperation(), MatShellTestMultTranspose()
@*/
PetscErrorCode  MatShellTestMult(Mat mat,PetscErrorCode (*f)(void*,Vec,Vec),Vec base,void *ctx,PetscBool *flg)
{
  PetscErrorCode ierr;
  PetscInt       m,n;
  Mat            mf,Dmf,Dmat,Ddiff;
  PetscReal      Diffnorm,Dmfnorm;
  PetscBool      v = PETSC_FALSE, flag = PETSC_TRUE;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_CLASSID,1);
  ierr = PetscOptionsHasName(NULL,((PetscObject)mat)->prefix,"-mat_shell_test_mult_view",&v);CHKERRQ(ierr);
  ierr = MatGetLocalSize(mat,&m,&n);CHKERRQ(ierr);
  ierr = MatCreateMFFD(PetscObjectComm((PetscObject)mat),m,n,PETSC_DECIDE,PETSC_DECIDE,&mf);CHKERRQ(ierr);
  ierr = MatMFFDSetFunction(mf,f,ctx);CHKERRQ(ierr);
  ierr = MatMFFDSetBase(mf,base,NULL);CHKERRQ(ierr);

  ierr = MatComputeExplicitOperator(mf,&Dmf);CHKERRQ(ierr);
  ierr = MatComputeExplicitOperator(mat,&Dmat);CHKERRQ(ierr);

  ierr = MatDuplicate(Dmat,MAT_COPY_VALUES,&Ddiff);CHKERRQ(ierr);
  ierr = MatAXPY(Ddiff,-1.0,Dmf,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);
  ierr = MatNorm(Ddiff,NORM_FROBENIUS,&Diffnorm);CHKERRQ(ierr);
  ierr = MatNorm(Dmf,NORM_FROBENIUS,&Dmfnorm);CHKERRQ(ierr);
  if (Diffnorm/Dmfnorm > 10*PETSC_SQRT_MACHINE_EPSILON) {
    flag = PETSC_FALSE;
    if (v) {
      ierr = PetscPrintf(PetscObjectComm((PetscObject)mat),"MATSHELL and matrix free multiple appear to produce different results.\n Norm Ratio %g Difference results followed by finite difference one\n",(double)(Diffnorm/Dmfnorm));
      ierr = MatViewFromOptions(Ddiff,(PetscObject)mat,"-mat_shell_test_mult_view");CHKERRQ(ierr);
      ierr = MatViewFromOptions(Dmf,(PetscObject)mat,"-mat_shell_test_mult_view");CHKERRQ(ierr);
      ierr = MatViewFromOptions(Dmat,(PetscObject)mat,"-mat_shell_test_mult_view");CHKERRQ(ierr);
    }
  } else if (v) {
    ierr = PetscPrintf(PetscObjectComm((PetscObject)mat),"MATSHELL and matrix free multiple appear to produce the same results\n");
  }
  if (flg) *flg = flag;
  ierr = MatDestroy(&Ddiff);CHKERRQ(ierr);
  ierr = MatDestroy(&mf);CHKERRQ(ierr);
  ierr = MatDestroy(&Dmf);CHKERRQ(ierr);
  ierr = MatDestroy(&Dmat);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*@C
    MatShellTestMultTranpose - Compares the multiply transpose routine provided to the MATSHELL with differencing on a given function.

   Logically Collective on Mat

    Input Parameters:
+   mat - the shell matrix
.   f - the function
.   base - differences are computed around this vector, see MatMFFDSetBase(), for Jacobians this is the point at which the Jacobian is being evaluated
-   ctx - an optional context for the function

   Output Parameter:
.   flg - PETSC_TRUE if the multiply is likely correct

   Options Database:
.   -mat_shell_test_mult_view - print if any differences are detected between the products and print the difference

   Level: advanced

   Fortran Notes: Not supported from Fortran

.seealso: MatCreateShell(), MatShellGetContext(), MatShellGetOperation(), MatShellTestMult()
@*/
PetscErrorCode  MatShellTestMultTranspose(Mat mat,PetscErrorCode (*f)(void*,Vec,Vec),Vec base,void *ctx,PetscBool *flg)
{
  PetscErrorCode ierr;
  Vec            x,y,z;
  PetscInt       m,n,M,N;
  Mat            mf,Dmf,Dmat,Ddiff;
  PetscReal      Diffnorm,Dmfnorm;
  PetscBool      v = PETSC_FALSE, flag = PETSC_TRUE;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_CLASSID,1);
  ierr = PetscOptionsHasName(NULL,((PetscObject)mat)->prefix,"-mat_shell_test_mult_transpose_view",&v);CHKERRQ(ierr);
  ierr = MatCreateVecs(mat,&x,&y);CHKERRQ(ierr);
  ierr = VecDuplicate(y,&z);CHKERRQ(ierr);
  ierr = MatGetLocalSize(mat,&m,&n);CHKERRQ(ierr);
  ierr = MatGetSize(mat,&M,&N);CHKERRQ(ierr);
  ierr = MatCreateMFFD(PetscObjectComm((PetscObject)mat),m,n,M,N,&mf);CHKERRQ(ierr);
  ierr = MatMFFDSetFunction(mf,f,ctx);CHKERRQ(ierr);
  ierr = MatMFFDSetBase(mf,base,NULL);CHKERRQ(ierr);
  ierr = MatComputeExplicitOperator(mf,&Dmf);CHKERRQ(ierr);
  ierr = MatTranspose(Dmf,MAT_INPLACE_MATRIX,&Dmf);CHKERRQ(ierr);
  ierr = MatComputeExplicitOperatorTranspose(mat,&Dmat);CHKERRQ(ierr);

  ierr = MatDuplicate(Dmat,MAT_COPY_VALUES,&Ddiff);CHKERRQ(ierr);
  ierr = MatAXPY(Ddiff,-1.0,Dmf,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);
  ierr = MatNorm(Ddiff,NORM_FROBENIUS,&Diffnorm);CHKERRQ(ierr);
  ierr = MatNorm(Dmf,NORM_FROBENIUS,&Dmfnorm);CHKERRQ(ierr);
  if (Diffnorm/Dmfnorm > 10*PETSC_SQRT_MACHINE_EPSILON) {
    flag = PETSC_FALSE;
    if (v) {
      ierr = PetscPrintf(PetscObjectComm((PetscObject)mat),"MATSHELL and matrix free multiple appear to produce different results.\n Norm Ratio %g Difference results followed by finite difference one\n",(double)(Diffnorm/Dmfnorm));
      ierr = MatViewFromOptions(Ddiff,(PetscObject)mat,"-mat_shell_test_mult_transpose_view");CHKERRQ(ierr);
      ierr = MatViewFromOptions(Dmf,(PetscObject)mat,"-mat_shell_test_mult_transpose_view");CHKERRQ(ierr);
      ierr = MatViewFromOptions(Dmat,(PetscObject)mat,"-mat_shell_test_mult_transpose_view");CHKERRQ(ierr);
    }
  } else if (v) {
    ierr = PetscPrintf(PetscObjectComm((PetscObject)mat),"MATSHELL transpose and matrix free multiple appear to produce the same results\n");
  }
  if (flg) *flg = flag;
  ierr = MatDestroy(&mf);CHKERRQ(ierr);
  ierr = MatDestroy(&Dmat);CHKERRQ(ierr);
  ierr = MatDestroy(&Ddiff);CHKERRQ(ierr);
  ierr = MatDestroy(&Dmf);CHKERRQ(ierr);
  ierr = VecDestroy(&x);CHKERRQ(ierr);
  ierr = VecDestroy(&y);CHKERRQ(ierr);
  ierr = VecDestroy(&z);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*@C
    MatShellSetOperation - Allows user to set a matrix operation for
                           a shell matrix.
    MatShellSetOperation - Allows user to set a matrix operation for a shell matrix.

   Logically Collective on Mat

    Input Parameters:
+   mat - the shell matrix
.   op - the name of the operation
-   f - the function that provides the operation.

   Level: advanced

    Usage:
$      extern PetscErrorCode usermult(Mat,Vec,Vec);
$      ierr = MatCreateShell(comm,m,n,M,N,ctx,&A);
$      ierr = MatShellSetOperation(A,MATOP_MULT,(void(*)(void))usermult);

    Notes:
    See the file include/petscmat.h for a complete list of matrix
    operations, which all have the form MATOP_<OPERATION>, where
    <OPERATION> is the name (in all capital letters) of the
    user interface routine (e.g., MatMult() -> MATOP_MULT).

    All user-provided functions (except for MATOP_DESTROY) should have the same calling
    sequence as the usual matrix interface routines, since they
    are intended to be accessed via the usual matrix interface
    routines, e.g.,
$       MatMult(Mat,Vec,Vec) -> usermult(Mat,Vec,Vec)

    In particular each function MUST return an error code of 0 on success and
    nonzero on failure.

    Within each user-defined routine, the user should call
    MatShellGetContext() to obtain the user-defined context that was
    set by MatCreateShell().

    Fortran Notes: For MatCreateVecs() the user code should check if the input left or right matrix is -1 and in that case not
       generate a matrix. See src/mat/examples/tests/ex120f.F

    Use MatSetOperation() to set an operation for any matrix type

.keywords: matrix, shell, set, operation

.seealso: MatCreateShell(), MatShellGetContext(), MatShellGetOperation(), MatShellSetContext(), MatSetOperation(), MatShellSetManageScalingShifts()
@*/
PetscErrorCode  MatShellSetOperation(Mat mat,MatOperation op,void (*f)(void))
{
  PetscErrorCode ierr;
  PetscBool      flg;
  Mat_Shell      *shell = (Mat_Shell*)mat->data;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_CLASSID,1);
  ierr = PetscObjectTypeCompare((PetscObject)mat,MATSHELL,&flg);CHKERRQ(ierr);
  if (!flg) SETERRQ(PetscObjectComm((PetscObject)mat),PETSC_ERR_SUP,"Can only use with MATSHELL matrices");
  switch (op) {
  case MATOP_DESTROY:
    shell->ops->destroy = (PetscErrorCode (*)(Mat))f;
    break;
  case MATOP_DIAGONAL_SET:
  case MATOP_DIAGONAL_SCALE:
  case MATOP_SHIFT:
  case MATOP_SCALE:
  case MATOP_AXPY:
    if (shell->managescalingshifts) SETERRQ(PetscObjectComm((PetscObject)mat),PETSC_ERR_ARG_WRONGSTATE,"MATSHELL is managing scalings and shifts, see MatShellSetManageScalingShifts()");
    (((void(**)(void))mat->ops)[op]) = f;
    break;
  case MATOP_GET_DIAGONAL:
    if (shell->managescalingshifts) shell->ops->getdiagonal = (PetscErrorCode (*)(Mat,Vec))f;
    else mat->ops->getdiagonal = (PetscErrorCode (*)(Mat,Vec))f;
  case MATOP_VIEW:
    if (!mat->ops->viewnative) {
      mat->ops->viewnative = mat->ops->view;
    }
    mat->ops->view = (PetscErrorCode (*)(Mat,PetscViewer))f;
    break;
  case MATOP_MULT:
    if (shell->managescalingshifts){
      shell->ops->mult = (PetscErrorCode (*)(Mat,Vec,Vec))f;
      mat->ops->mult   = MatMult_Shell;
    } else mat->ops->mult = (PetscErrorCode (*)(Mat,Vec,Vec))f;
    break;
  case MATOP_MULT_TRANSPOSE:
    if (shell->managescalingshifts) {
      shell->ops->multtranspose = (PetscErrorCode (*)(Mat,Vec,Vec))f;
      mat->ops->multtranspose   = MatMultTranspose_Shell;
    } else mat->ops->multtranspose = (PetscErrorCode (*)(Mat,Vec,Vec))f;
    break;
  default:
    (((void(**)(void))mat->ops)[op]) = f;
  }
  PetscFunctionReturn(0);
}

/*@C
    MatShellGetOperation - Gets a matrix function for a shell matrix.

    Not Collective

    Input Parameters:
+   mat - the shell matrix
-   op - the name of the operation

    Output Parameter:
.   f - the function that provides the operation.

    Level: advanced

    Notes:
    See the file include/petscmat.h for a complete list of matrix
    operations, which all have the form MATOP_<OPERATION>, where
    <OPERATION> is the name (in all capital letters) of the
    user interface routine (e.g., MatMult() -> MATOP_MULT).

    All user-provided functions have the same calling
    sequence as the usual matrix interface routines, since they
    are intended to be accessed via the usual matrix interface
    routines, e.g.,
$       MatMult(Mat,Vec,Vec) -> usermult(Mat,Vec,Vec)

    Within each user-defined routine, the user should call
    MatShellGetContext() to obtain the user-defined context that was
    set by MatCreateShell().

.keywords: matrix, shell, set, operation

.seealso: MatCreateShell(), MatShellGetContext(), MatShellSetOperation(), MatShellSetContext()
@*/
PetscErrorCode  MatShellGetOperation(Mat mat,MatOperation op,void(**f)(void))
{
  PetscErrorCode ierr;
  PetscBool      flg;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_CLASSID,1);
  switch (op) {
  case MATOP_DESTROY:
    ierr = PetscObjectTypeCompare((PetscObject)mat,MATSHELL,&flg);CHKERRQ(ierr);
    if (flg) {
      Mat_Shell *shell = (Mat_Shell*)mat->data;
      *f = (void (*)(void))shell->ops->destroy;
    } else {
      *f = (void (*)(void))mat->ops->destroy;
    }
    break;
  case MATOP_DIAGONAL_SET:
    ierr = PetscObjectTypeCompare((PetscObject)mat,MATSHELL,&flg);CHKERRQ(ierr);
    if (flg) {
      Mat_Shell *shell = (Mat_Shell*)mat->data;
      *f = (void (*)(void))shell->ops->diagonalset;
    } else {
      *f = (void (*)(void))mat->ops->diagonalset;
    }
    break;
  case MATOP_VIEW:
    *f = (void (*)(void))mat->ops->view;
    break;
  default:
    *f = (((void (**)(void))mat->ops)[op]);
  }
  PetscFunctionReturn(0);
}

/*@C
    MatSetOperation - Allows user to set a matrix operation for any matrix type

   Logically Collective on Mat

    Input Parameters:
+   mat - the shell matrix
.   op - the name of the operation
-   f - the function that provides the operation.

   Level: developer

    Usage:
$      extern PetscErrorCode usermult(Mat,Vec,Vec);
$      ierr = MatCreateXXX(comm,...&A);
$      ierr = MatShellSetOperation(A,MATOP_MULT,(void(*)(void))usermult);

    Notes:
    See the file include/petscmat.h for a complete list of matrix
    operations, which all have the form MATOP_<OPERATION>, where
    <OPERATION> is the name (in all capital letters) of the
    user interface routine (e.g., MatMult() -> MATOP_MULT).

    All user-provided functions (except for MATOP_DESTROY) should have the same calling
    sequence as the usual matrix interface routines, since they
    are intended to be accessed via the usual matrix interface
    routines, e.g.,
$       MatMult(Mat,Vec,Vec) -> usermult(Mat,Vec,Vec)

    In particular each function MUST return an error code of 0 on success and
    nonzero on failure.

    This routine is distinct from MatShellSetOperation() in that it can be called on any matrix type.

.keywords: matrix, shell, set, operation

.seealso: MatCreateShell(), MatShellGetContext(), MatShellGetOperation(), MatShellSetContext(), MatShellSetOperation()
@*/
PetscErrorCode  MatSetOperation(Mat mat,MatOperation op,void (*f)(void))
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_CLASSID,1);
  (((void(**)(void))mat->ops)[op]) = f;
  PetscFunctionReturn(0);
}
