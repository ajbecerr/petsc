/* $Id: petscvec.h,v 1.127 2001/09/11 16:31:30 bsmith Exp $ */
/* 
    Defines the vector component of PETSc. Vectors generally represent 
  degrees of freedom for finite element/finite difference functions
  on a grid. They have more mathematical structure then simple arrays.
*/

#ifndef __PETSCVEC_H 
#define __PETSCVEC_H
#include "petscis.h"
#include "petscsys.h"

#define VEC_COOKIE         PETSC_COOKIE+3
#define MAP_COOKIE         PETSC_COOKIE+22
#define VEC_SCATTER_COOKIE PETSC_COOKIE+4

/*S
     PetscMap - Abstract PETSc object that defines the layout of vector and
  matrices across processors

   Level: advanced

   Notes:
    Does not play a role in the PETSc design, can be ignored

  Concepts: parallel decomposition

.seealso:  PetscMapCreateMPI()
S*/
typedef struct _p_PetscMap*         PetscMap;

/*S
     Vec - Abstract PETSc vector object

   Level: beginner

  Concepts: field variables, unknowns, arrays

.seealso:  VecCreate(), VecType, VecSetType()
S*/
typedef struct _p_Vec*         Vec;

/*S
     VecScatter - Object used to manage communication of data
       between vectors in parallel. Manages both scatters and gathers

   Level: beginner

  Concepts: scatter

.seealso:  VecScatterCreate(), VecScatterBegin(), VecScatterEnd()
S*/
typedef struct _p_VecScatter*  VecScatter;

/*E
    VecType - String with the name of a PETSc vector or the creation function
       with an optional dynamic library name, for example
       http://www.mcs.anl.gov/petsc/lib.a:myveccreate()

   Level: beginner

.seealso: VecSetType(), Vec
E*/
#define VEC_SEQ         "seq"
#define VEC_MPI         "mpi"
#define VEC_FETI        "feti"
#define VEC_SHARED      "shared"
#define VEC_ESI         "esi"
#define VEC_PETSCESI    "petscesi"
typedef char*  VecType;

#define VEC_SER_SEQ_BINARY "seq_binary"
#define VEC_SER_MPI_BINARY "mpi_binary"
typedef char *VecSerializeType;

#ifdef PETSC_USE_NEW_LOGGING
/* Logging support */
extern int VEC_COOKIE;
extern int MAP_COOKIE;
extern int VEC_SCATTER_COOKIE;
enum {VEC_ScatterBarrier, VEC_Dot, VEC_Norm, VEC_Max, VEC_Min, VEC_TDot, VEC_Scale, VEC_Copy, VEC_Set, VEC_AXPY, VEC_AYPX,
      VEC_Swap, VEC_WAXPY, VEC_AssemblyBegin, VEC_AssemblyEnd, VEC_MTDot, VEC_MDot, VEC_MAXPY, VEC_PMult, VEC_SetValues,
      VEC_Load, VEC_View, VEC_ScatterBarrierBegin, VEC_ScatterBegin, VEC_ScatterEnd, VEC_SetRandom,
      VEC_NormBarrier, VEC_NormComm, VEC_DotBarrier, VEC_DotComm, VEC_MDotBarrier, VEC_MDotComm, VEC_ReduceArithmetic,
      VEC_ReduceCommunication, VEC_ReduceBarrier, VEC_ReduceCommOnly, VEC_MAX_EVENTS};
extern int VecEvents[VEC_MAX_EVENTS];
#define VecLogEventBegin(e,o1,o2,o3,o4) PetscLogEventBegin(VecEvents[e],o1,o2,o3,o4)
#define VecLogEventEnd(e,o1,o2,o3,o4)   PetscLogEventEnd(VecEvents[e],o1,o2,o3,o4)
#endif

EXTERN int VecCreateSeq(MPI_Comm,int,Vec*);
EXTERN int PetscMapCreateMPI(MPI_Comm,int,int,PetscMap*);  
EXTERN int VecCreateMPI(MPI_Comm,int,int,Vec*);  
EXTERN int VecCreateSeqWithArray(MPI_Comm,int,const PetscScalar[],Vec*);  
EXTERN int VecCreateMPIWithArray(MPI_Comm,int,int,const PetscScalar[],Vec*);  
EXTERN int VecCreateShared(MPI_Comm,int,int,Vec*);  
EXTERN int VecCreate(MPI_Comm, Vec *); 
EXTERN int VecSetType(Vec,VecType); 
EXTERN int VecSetFromOptions(Vec);
EXTERN int VecPrintHelp(Vec);
EXTERN int VecDestroy(Vec);        

EXTERN int VecSetSize(Vec, int, int);

EXTERN int PetscMapDestroy(PetscMap);
EXTERN int PetscMapGetLocalSize(PetscMap,int *);
EXTERN int PetscMapGetSize(PetscMap,int *);
EXTERN int PetscMapGetLocalRange(PetscMap,int *,int *);
EXTERN int PetscMapGetGlobalRange(PetscMap,int *[]);

EXTERN int VecDot(Vec,Vec,PetscScalar*);
EXTERN int VecTDot(Vec,Vec,PetscScalar*);  
EXTERN int VecMDot(int,Vec,const Vec[],PetscScalar*);
EXTERN int VecMTDot(int,Vec,const Vec[],PetscScalar*); 

/*E
    NormType - determines what type of norm to compute

    Level: beginner

.seealso: VecNorm(), VecNormBegin(), VecNormEnd(), MatNorm()
E*/
typedef enum {NORM_1=1,NORM_2=2,NORM_FROBENIUS=3,NORM_INFINITY=4,NORM_1_AND_2=5} NormType;
#define NORM_MAX NORM_INFINITY

EXTERN int VecNorm(Vec,NormType,PetscReal *);
EXTERN int VecSum(Vec,PetscScalar*);
EXTERN int VecMax(Vec,int*,PetscReal *);
EXTERN int VecMin(Vec,int*,PetscReal *);
EXTERN int VecScale(const PetscScalar *a,Vec v);
EXTERN int VecCopy(Vec,Vec);        
EXTERN int VecSetRandom(PetscRandom,Vec);
EXTERN int VecSet(const PetscScalar*,Vec);
EXTERN int VecSwap(Vec,Vec);
EXTERN int VecAXPY(const PetscScalar*,Vec,Vec);  
EXTERN int VecAXPBY(const PetscScalar*,const PetscScalar *,Vec,Vec);  
EXTERN int VecMAXPY(int,const PetscScalar*,Vec,Vec*);
EXTERN int VecAYPX(const PetscScalar*,Vec,Vec);
EXTERN int VecWAXPY(const PetscScalar*,Vec,Vec,Vec);
EXTERN int VecPointwiseMult(Vec,Vec,Vec);    
EXTERN int VecPointwiseDivide(Vec,Vec,Vec);    
EXTERN int VecShift(const PetscScalar*,Vec);
EXTERN int VecReciprocal(Vec);
EXTERN int VecPermute(Vec, IS, PetscTruth);
EXTERN int VecSqrt(Vec);
EXTERN int VecAbs(Vec);
EXTERN int VecDuplicate(Vec,Vec*);          
EXTERN int VecDuplicateVecs(Vec,int,Vec*[]);         
EXTERN int VecDestroyVecs(const Vec[],int); 
EXTERN int VecGetPetscMap(Vec,PetscMap*);

EXTERN int VecStrideNorm(Vec,int,NormType,PetscReal*);
EXTERN int VecStrideGather(Vec,int,Vec,InsertMode);
EXTERN int VecStrideScatter(Vec,int,Vec,InsertMode);
EXTERN int VecStrideMax(Vec,int,int *,PetscReal *);
EXTERN int VecStrideMin(Vec,int,int *,PetscReal *);
EXTERN int VecStrideGatherAll(Vec,Vec*,InsertMode);
EXTERN int VecStrideScatterAll(Vec*,Vec,InsertMode);

EXTERN int VecSetValues(Vec,int,const int[],const PetscScalar[],InsertMode);
EXTERN int VecAssemblyBegin(Vec);
EXTERN int VecAssemblyEnd(Vec);
EXTERN int VecSetStashInitialSize(Vec,int,int);
EXTERN int VecStashView(Vec,PetscViewer);

#define VecSetValue(v,i,va,mode) 0;\
{int _ierr,_row = i; PetscScalar _va = va; \
  _ierr = VecSetValues(v,1,&_row,&_va,mode);CHKERRQ(_ierr); \
}
EXTERN int VecSetBlockSize(Vec,int);
EXTERN int VecGetBlockSize(Vec,int*);
EXTERN int VecSetValuesBlocked(Vec,int,const int[],const PetscScalar[],InsertMode);

/* Dynamic creation and loading functions */
extern PetscFList VecList;
extern int VecRegisterAllCalled;
extern int VecSetType(Vec, VecType);
extern int VecGetType(Vec, VecType *);
extern int VecRegister(const char[],const char[],const char[],int(*)(Vec, ParameterDict));
extern int VecRegisterAll(const char []);
extern int VecRegisterDestroy(void);
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define VecRegisterDynamic(a,b,c,d) VecRegister(a,b,c,0)
#else
#define VecRegisterDynamic(a,b,c,d) VecRegister(a,b,c,d)
#endif

extern PetscFList VecSerializeList;
extern int VecSerializeRegisterAllCalled;
extern int VecSetSerializeType(Vec, VecSerializeType);
extern int VecGetSerializeType(Vec, VecSerializeType *);
extern int VecSerializeRegister(const char [], const char [], const char [], int (*)(MPI_Comm, Vec *, PetscViewer, PetscTruth));
extern int VecSerializeRegisterAll(const char []);
extern int VecSerializeRegisterDestroy(void);
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define VecSerializeRegisterDynamic(a,b,c,d) VecSerializeRegister(a,b,c,0)
#else
#define VecSerializeRegisterDynamic(a,b,c,d) VecSerializeRegister(a,b,c,d)
#endif

EXTERN int VecScatterCreate(Vec,IS,Vec,IS,VecScatter *);
EXTERN int VecScatterPostRecvs(Vec,Vec,InsertMode,ScatterMode,VecScatter);
EXTERN int VecScatterBegin(Vec,Vec,InsertMode,ScatterMode,VecScatter);
EXTERN int VecScatterEnd(Vec,Vec,InsertMode,ScatterMode,VecScatter); 
EXTERN int VecScatterDestroy(VecScatter);
EXTERN int VecScatterCopy(VecScatter,VecScatter *);
EXTERN int VecScatterView(VecScatter,PetscViewer);
EXTERN int VecScatterRemap(VecScatter,int *,int*);

typedef enum {PIPELINE_DOWN=0,PIPELINE_UP=1} PipelineDirection;
typedef enum {PIPELINE_NONE=1,PIPELINE_SEQUENTIAL=2,
	      PIPELINE_REDBLACK=3,PIPELINE_MULTICOLOR=4} PipelineType;

typedef struct _p_VecPipeline*  VecPipeline;

EXTERN int VecPipelineCreate(MPI_Comm,Vec,IS,Vec,IS,VecPipeline *);
EXTERN int VecPipelineSetType(VecPipeline,PipelineType,PetscObject);
EXTERN int VecPipelineSetup(VecPipeline);
EXTERN int VecPipelineBegin(Vec,Vec,InsertMode,ScatterMode,PipelineDirection,VecPipeline);
EXTERN int VecPipelineEnd(Vec,Vec,InsertMode,ScatterMode,PipelineDirection,VecPipeline); 
EXTERN int VecPipelineView(VecPipeline,PetscViewer);
EXTERN int VecPipelineDestroy(VecPipeline);

EXTERN int VecGetArray(Vec,PetscScalar*[]);
EXTERN int VecRestoreArray(Vec,PetscScalar*[]);
EXTERN int VecGetArray4d(Vec,int,int,int,int,int,int,int,int,PetscScalar****[]);
EXTERN int VecRestoreArray4d(Vec,int,int,int,int,int,int,int,int,PetscScalar****[]);
EXTERN int VecGetArray3d(Vec,int,int,int,int,int,int,PetscScalar***[]);
EXTERN int VecRestoreArray3d(Vec,int,int,int,int,int,int,PetscScalar***[]);
EXTERN int VecGetArray2d(Vec,int,int,int,int,PetscScalar**[]);
EXTERN int VecRestoreArray2d(Vec,int,int,int,int,PetscScalar**[]);
EXTERN int VecGetArray1d(Vec,int,int,PetscScalar *[]);
EXTERN int VecRestoreArray1d(Vec,int,int,PetscScalar *[]);

EXTERN int VecPlaceArray(Vec,const PetscScalar[]);
EXTERN int VecResetArray(Vec);
EXTERN int VecReplaceArray(Vec,const PetscScalar[]);
EXTERN int VecGetArrays(const Vec[],int,PetscScalar**[]);
EXTERN int VecRestoreArrays(const Vec[],int,PetscScalar**[]);

EXTERN int VecValid(Vec,PetscTruth*);
EXTERN int VecView(Vec,PetscViewer);
EXTERN int VecEqual(Vec,Vec,PetscTruth*);
EXTERN int VecLoad(PetscViewer,Vec*);
EXTERN int VecLoadIntoVector(PetscViewer,Vec);

EXTERN int VecGetSize(Vec,int*);
EXTERN int VecGetLocalSize(Vec,int*);
EXTERN int VecGetType(Vec,VecType*);
EXTERN int VecGetOwnershipRange(Vec,int*,int*);

EXTERN int VecSetLocalToGlobalMapping(Vec,ISLocalToGlobalMapping);
EXTERN int VecSetValuesLocal(Vec,int,const int[],const PetscScalar[],InsertMode);
EXTERN int VecSetLocalToGlobalMappingBlock(Vec,ISLocalToGlobalMapping);
EXTERN int VecSetValuesBlockedLocal(Vec,int,const int[],const PetscScalar[],InsertMode);

EXTERN int VecDotBegin(Vec,Vec,PetscScalar *);
EXTERN int VecDotEnd(Vec,Vec,PetscScalar *);
EXTERN int VecTDotBegin(Vec,Vec,PetscScalar *);
EXTERN int VecTDotEnd(Vec,Vec,PetscScalar *);
EXTERN int VecNormBegin(Vec,NormType,PetscReal *);
EXTERN int VecNormEnd(Vec,NormType,PetscReal *);

typedef enum {VEC_IGNORE_OFF_PROC_ENTRIES,VEC_TREAT_OFF_PROC_ENTRIES} VecOption;
EXTERN int VecSetOption(Vec,VecOption);

EXTERN int VecContourScale(Vec,PetscReal,PetscReal);

/*
    These numbers need to match the entries in 
  the function table in src/vec/vecimpl.h
*/
typedef enum { VECOP_VIEW = 32,
               VECOP_LOADINTOVECTOR = 38
             } VecOperation;
EXTERN int VecSetOperation(Vec,VecOperation,void(*)());

/*
     Routines for dealing with ghosted vectors:
  vectors with ghost elements at the end of the array.
*/
EXTERN int VecCreateGhost(MPI_Comm,int,int,int,const int[],Vec*);  
EXTERN int VecCreateGhostWithArray(MPI_Comm,int,int,int,const int[],const PetscScalar[],Vec*);  
EXTERN int VecCreateGhostBlock(MPI_Comm,int,int,int,int,const int[],Vec*);  
EXTERN int VecCreateGhostBlockWithArray(MPI_Comm,int,int,int,int,const int[],const PetscScalar[],Vec*);  
EXTERN int VecGhostGetLocalForm(Vec,Vec*);
EXTERN int VecGhostRestoreLocalForm(Vec,Vec*);
EXTERN int VecGhostUpdateBegin(Vec,InsertMode,ScatterMode);
EXTERN int VecGhostUpdateEnd(Vec,InsertMode,ScatterMode);

EXTERN int VecConjugate(Vec);

EXTERN int VecConvertMPIToSeqAll(Vec vin,Vec *vout);
EXTERN int VecConvertMPIToMPIZero(Vec vin,Vec *vout);


EXTERN int VecESISetType(Vec,char*);
EXTERN int VecESISetFromOptions(Vec);

#endif









