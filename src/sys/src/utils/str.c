/*$Id: str.c,v 1.38 1999/12/26 23:41:26 bsmith Exp bsmith $*/
/*
    We define the string operations here. The reason we just do not use 
  the standard string routines in the PETSc code is that on some machines 
  they are broken or have the wrong prototypes.

*/
#include "petsc.h"                   /*I  "petsc.h"   I*/
#if defined(PETSC_HAVE_STRING_H)
#include <string.h>
#endif
#if defined(PETSC_HAVE_STRINGS_H)
#include <strings.h>
#endif
#include "petscfix.h"

#undef __FUNC__  
#define __FUNC__ "PetscStrlen"
int PetscStrlen(const char s[],int *len)
{
  PetscFunctionBegin;
  if (!s) {
    *len = 0;
  } else {
    *len = strlen(s);
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrallocpy"
int PetscStrallocpy(const char s[],char **t)
{
  int ierr,len;

  PetscFunctionBegin;
  if (s) {
    ierr  = PetscStrlen(s,&len);CHKERRQ(ierr);
    *t    = (char*)PetscMalloc((1+len)*sizeof(char));CHKPTRQ(*t);
    ierr  = PetscStrcpy(*t,s);CHKERRQ(ierr);
  } else {
    *t = 0;
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrcpy"
/*
    Handles copying null string correctly
*/
int PetscStrcpy(char s[],const char t[])
{
  PetscFunctionBegin;
  if (t && !s) {
    SETERRQ(1,1,"Trying to copy string into null pointer");
  }
  if (t) {strcpy(s,t);}
  else {s[0] = 0;}
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrncpy"
int PetscStrncpy(char s[],const char t[],int n)
{
  PetscFunctionBegin;
  strncpy(s,t,n);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrcat"
int PetscStrcat(char s[],const char t[])
{
  PetscFunctionBegin;
  strcat(s,t);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrncat"
int PetscStrncat(char s[],const char t[],int n)
{
  PetscFunctionBegin;
  strncat(s,t,n);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrcmp"
int PetscStrcmp(const char a[],const char b[],PetscTruth *flg)
{
  int c;

  PetscFunctionBegin;
  if (!a && !b) {
    *flg = PETSC_TRUE;
  } else if (!a || !b) {
    *flg = PETSC_FALSE;
  } else {
    c = strcmp(a,b);
    if (c) *flg = PETSC_FALSE;
    else   *flg = PETSC_TRUE;
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrgrt"
int PetscStrgrt(const char a[],const char b[],PetscTruth *t)
{
  int c;

  PetscFunctionBegin;
  if (!a && !b) {
    *t = PETSC_FALSE;
  } else if (a && !b) {
    *t = PETSC_TRUE; 
  } else if (!a && b) {
    *t = PETSC_FALSE; 
  } else {
    c = strcmp(a,b);
    if (c > 0) *t = PETSC_TRUE;
    else       *t = PETSC_FALSE;
  }
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrcasecmp"
/*
    Note: This is different from system strncmp() this returns PETSC_TRUE
    if the strings are the same!
*/
int PetscStrcasecmp(const char a[],const char b[],PetscTruth *t)
{
  int c;

  PetscFunctionBegin;
  if (!a && !b) c = 0;
  else if (!a || !b) c = 1;
#if defined (PARCH_win32)
  else c = stricmp(a,b);
#else
  else c = strcasecmp(a,b);
#endif
  if (!c) *t = PETSC_TRUE;
  else    *t = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrncmp"
/*
    Note: This is different from system strncmp() this returns PETSC_TRUE
    if the strings are the same!
*/
int PetscStrncmp(const char a[],const char b[],int n,PetscTruth *t)
{
  int c;

  PetscFunctionBegin;
  c = strncmp(a,b,n);
  if (!c) *t = PETSC_TRUE;
  else    *t = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrchr"
int PetscStrchr(const char a[],char b,char **c)
{
  PetscFunctionBegin;
  *c = (char *)strchr(a,b);
  PetscFunctionReturn(0);
}

/*
      This is slightly different then the system version. 
   It returns the position after the position of b and 
   if it does not find it then it returns the entire string.
*/
#undef __FUNC__  
#define __FUNC__ "PetscStrrchr"
int PetscStrrchr(const char a[],char b,char **tmp)
{
  PetscFunctionBegin;
  *tmp = (char *)strrchr(a,b);
  if (!*tmp) *tmp = (char*)a; else *tmp = *tmp + 1;
  PetscFunctionReturn(0);
}

/*
     This version is different from the system version in that
  it allows you to pass a read-only string into the function.
  A copy is made that is then passed into the system strtok() 
  routine.

    Limitation: 
  String must be less than or equal 1024 bytes in length, otherwise
  it will bleed memory.

*/
#undef __FUNC__  
#define __FUNC__ "PetscStrtok"
int PetscStrtok(const char a[],const char b[],char **result)
{
  static char init[1024];
  char        *ptr=0;
  int         ierr,len;
         

  PetscFunctionBegin;
  if (a) {
    ierr = PetscStrlen(a,&len);CHKERRQ(ierr);
    if (len > 1023) {
      ptr = (char*)PetscMalloc((len+1)*sizeof(char));
      if (!ptr) SETERRQ(1,1,"Malloc failed");
    } else {
      ptr = init;
    }
    ierr = PetscStrncpy(ptr,a,len+1);CHKERRQ(ierr);
  }
  *result = strtok(ptr,b);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrstr"
int PetscStrstr(const char a[],const char b[],char **tmp)
{
  PetscFunctionBegin;
  *tmp = (char *)strstr(a,b);
  PetscFunctionReturn(0);
}

#undef __FUNC__  
#define __FUNC__ "PetscStrreplace"
/*
      Finds any of the substrings in s and replaces with corresponding 
    one in r.

      No proper error checking yet
*/
int PetscStrreplace(const char a[],char *b,int len,char **s,char **r)
{
  int  ierr,i = 0,l,l1,l2,l3;
  char *work,*par;

  PetscFunctionBegin;
  if (len <= 0) SETERRQ(1,1,"Length of b must be greater than 0");
  if (!a || !b) SETERRQ(1,1,"a and b strings must be nonnull");
  if (!s || !r) SETERRQ(1,1,"s and r arrays must be nonull");
  work = (char*)PetscMalloc(len*sizeof(char*));CHKPTRQ(work);
  ierr = PetscStrncpy(b,a,len);CHKERRQ(ierr);  
  while (s[i]) {
    ierr = PetscStrlen(s[i],&l);CHKERRQ(ierr);
    ierr = PetscStrstr(b,s[i],&par);CHKERRQ(ierr);
    while (par) {
      *par  =  0;
      par  += l;

      ierr = PetscStrlen(b,&l1);CHKERRQ(ierr);
      ierr = PetscStrlen(r[i],&l2);CHKERRQ(ierr);
      ierr = PetscStrlen(par,&l3);CHKERRQ(ierr);
      if (l1 + l2 + l3 >= len) {
        SETERRQ(1,1,"b len is not long enough to hold new values");
      }
      ierr  = PetscStrcpy(work,b);CHKERRQ(ierr);
      ierr  = PetscStrcat(work,r[i]);CHKERRQ(ierr);
      ierr  = PetscStrcat(work,par);CHKERRQ(ierr);
      ierr  = PetscStrncpy(b,work,len);CHKERRQ(ierr);
      ierr  = PetscStrstr(b,s[i],&par);CHKERRQ(ierr);
    }
    i++;
  }
  ierr = PetscFree(work);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
