/*$Id: petscfix.h,v 1.99 1999/11/23 18:12:07 bsmith Exp bsmith $*/

/*
    This fixes various things in system files that are incomplete, for 
  instance many systems don't properly prototype all system functions.
  It is not intended to DUPLICATE anything in the system include files;
  if the compiler reports a conflict between a prototye in a system file
  and this file then the prototype in this file should be removed.

    This is included by files in src/sys/src
*/

#if !defined(_PETSCFIX_H)
#define _PETSCFIX_H

#include "petsc.h"

/*
  This prototype lets us resolve the datastructure 'rusage' only in
  the source files using getrusage, and not in other source files.
*/
typedef struct rusage* s_rusage;

/* ------------------ ASCI Red --------------------------------*/
#if defined(__cplusplus)
extern "C" {
extern char *getwd(char *_name);
}
#else
extern char *getwd(char *_name);
#endif
#endif

