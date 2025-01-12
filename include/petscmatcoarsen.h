#pragma once

#include <petscmat.h>

/* SUBMANSEC = Mat */

PETSC_EXTERN PetscFunctionList MatCoarsenList;

/*S
     MatCoarsen - Object for managing the coarsening of a graph (symmetric matrix)

   Level: advanced

  Note:
    This is used by the `PCGAMG` to generate coarser representations of an algebraic problem

.seealso: [](ch_matrices), [](sec_graph), `Mat`, `MatCoarsenCreate()`, `MatCoarsenType`, `MatColoringType`, `MatPartitioningType`, `MatOrderingType`
          `MatColoring`, `MatPartitioning`
S*/
typedef struct _p_MatCoarsen *MatCoarsen;

/*J
    MatCoarsenType - String with the name of a PETSc matrix coarsening algorithm

   Level: beginner

.seealso: [](ch_matrices), [](sec_graph), `Mat`, `MatCoarsenCreate()`, `MatCoarsen`, `MatColoringType`, `MatPartitioningType`, `MatOrderingType`
J*/
typedef const char *MatCoarsenType;
#define MATCOARSENMIS  "mis"
#define MATCOARSENHEM  "hem"
#define MATCOARSENMISK "misk"

/* linked list for aggregates */
typedef struct _PetscCDIntNd {
  struct _PetscCDIntNd *next;
  PetscInt              gid;
} PetscCDIntNd;

/* only used by node pool */
typedef struct _PetscCDArrNd {
  struct _PetscCDArrNd *next;
  struct _PetscCDIntNd *array;
} PetscCDArrNd;

/* linked list data structure that encodes aggregates and C-F points with array[idx] == NULL for F point and array of indices in an aggregate or C point (first index is always global index my0 + idx */
typedef struct _PetscCoarsenData {
  PetscCDArrNd   pool_list; /* node pool */
  PetscCDIntNd  *new_node;
  PetscInt       new_left;
  PetscInt       chk_sz; /* chunk size */
  PetscCDIntNd  *extra_nodes;
  PetscCDIntNd **array; /* Array of lists */
  PetscInt       size;  /* size of 'array' */
  Mat            mat;   /* cache a Mat for communication data */
} PetscCoarsenData;

PETSC_EXTERN PetscErrorCode MatCoarsenCreate(MPI_Comm, MatCoarsen *);
PETSC_EXTERN PetscErrorCode MatCoarsenSetType(MatCoarsen, MatCoarsenType);
PETSC_EXTERN PetscErrorCode MatCoarsenSetAdjacency(MatCoarsen, Mat);
PETSC_EXTERN PetscErrorCode MatCoarsenSetGreedyOrdering(MatCoarsen, const IS);
PETSC_EXTERN PetscErrorCode MatCoarsenSetStrictAggs(MatCoarsen, PetscBool);
PETSC_EXTERN PetscErrorCode MatCoarsenGetData(MatCoarsen, PetscCoarsenData **);
PETSC_EXTERN PetscErrorCode MatCoarsenApply(MatCoarsen);
PETSC_EXTERN PetscErrorCode MatCoarsenDestroy(MatCoarsen *);
PETSC_EXTERN PetscErrorCode MatCoarsenRegister(const char[], PetscErrorCode (*)(MatCoarsen));
PETSC_EXTERN PetscErrorCode MatCoarsenView(MatCoarsen, PetscViewer);
PETSC_EXTERN PetscErrorCode MatCoarsenSetFromOptions(MatCoarsen);
PETSC_EXTERN PetscErrorCode MatCoarsenGetType(MatCoarsen, MatCoarsenType *);
PETSC_EXTERN PetscErrorCode MatCoarsenViewFromOptions(MatCoarsen, PetscObject, const char[]);

PETSC_EXTERN PetscErrorCode PetscCDCreate(PetscInt, PetscCoarsenData **);
PETSC_EXTERN PetscErrorCode PetscCDDestroy(PetscCoarsenData *);
PETSC_EXTERN PetscErrorCode PetscCDIntNdSetID(PetscCDIntNd *, PetscInt);
PETSC_EXTERN PetscErrorCode PetscCDIntNdGetID(const PetscCDIntNd *, PetscInt *);
PETSC_EXTERN PetscErrorCode PetscCDAppendID(PetscCoarsenData *, PetscInt, PetscInt);
PETSC_EXTERN PetscErrorCode PetscCDAppendRemove(PetscCoarsenData *, PetscInt, PetscInt);
PETSC_EXTERN PetscErrorCode PetscCDAppendNode(PetscCoarsenData *, PetscInt, PetscCDIntNd *);
PETSC_EXTERN PetscErrorCode PetscCDRemoveNextNode(PetscCoarsenData *, PetscInt, PetscCDIntNd *);
PETSC_EXTERN PetscErrorCode PetscCDSizeAt(const PetscCoarsenData *, PetscInt, PetscInt *);
PETSC_EXTERN PetscErrorCode PetscCDEmptyAt(const PetscCoarsenData *, PetscInt, PetscBool *);
PETSC_EXTERN PetscErrorCode PetscCDSetChuckSize(PetscCoarsenData *, PetscInt);
PETSC_EXTERN PetscErrorCode PetscCDPrint(const PetscCoarsenData *, MPI_Comm);
PETSC_EXTERN PetscErrorCode PetscCDGetMIS(PetscCoarsenData *, IS *);
PETSC_EXTERN PetscErrorCode PetscCDGetMat(PetscCoarsenData *, Mat *);
PETSC_EXTERN PetscErrorCode PetscCDSetMat(PetscCoarsenData *, Mat);
PETSC_EXTERN PetscErrorCode PetscCDRemoveAll(PetscCoarsenData *, PetscInt);

PETSC_EXTERN PetscErrorCode PetscCDGetHeadPos(const PetscCoarsenData *, PetscInt, PetscCDIntNd **);
PETSC_EXTERN PetscErrorCode PetscCDGetNextPos(const PetscCoarsenData *, PetscInt, PetscCDIntNd **);
PETSC_EXTERN PetscErrorCode PetscCDGetASMBlocks(const PetscCoarsenData *, const PetscInt, Mat, PetscInt *, IS **);

PETSC_EXTERN PetscErrorCode MatCoarsenMISKSetDistance(MatCoarsen, PetscInt);
PETSC_EXTERN PetscErrorCode MatCoarsenMISKGetDistance(MatCoarsen, PetscInt *);
