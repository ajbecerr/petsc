/*$Id: dpoints.c,v 1.27 1999/10/24 14:01:10 bsmith Exp bsmith $*/
/*
       Provides the calling sequences for all the basic Draw routines.
*/
#include "src/sys/src/draw/drawimpl.h"  /*I "draw.h" I*/

#undef __FUNC__  
#define __FUNC__ "DrawPointSetSize" 
/*@
   DrawPointSetSize - Sets the point size for future draws.  The size is
   relative to the user coordinates of the window; 0.0 denotes the natural
   width, 1.0 denotes the entire viewport. 

   Not collective

   Input Parameters:
+  draw - the drawing context
-  width - the width in user coordinates

   Level: advanced

   Note: 
   Even a size of zero insures that a single pixel is colored.

.keywords: draw, point, set, size

.seealso: DrawPoint()
@*/
int DrawPointSetSize(Draw draw,PetscReal width)
{
  int        ierr;
  PetscTruth isnull;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,DRAW_COOKIE);
  ierr = PetscTypeCompare((PetscObject)draw,DRAW_NULL,&isnull);CHKERRQ(ierr);
  if (isnull) PetscFunctionReturn(0);
  if (width < 0.0 || width > 1.0) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,0,"Bad size %g, should be between 0 and 1",width);
  ierr = (*draw->ops->pointsetsize)(draw,width);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

