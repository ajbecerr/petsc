/*$Id: ex3.c,v 1.34 1999/11/24 21:52:55 bsmith Exp bsmith $*/

static char help[] = "Plots a simple line graph\n";

#include "petsc.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  Draw           draw;
  DrawLG         lg;
  DrawAxis       axis;
  int            n = 20,i,ierr,x = 0,y = 0,width = 300,height = 300,nports = 1;
  PetscTruth     flg;
  char           *xlabel,*ylabel,*toplabel;
  double         xd,yd;
  DrawViewPorts  *ports;

  xlabel = "X-axis Label";toplabel = "Top Label";ylabel = "Y-axis Label";

  PetscInitialize(&argc,&argv,(char*)0,help);
  ierr = OptionsGetInt(PETSC_NULL,"-width",&width,PETSC_NULL);CHKERRA(ierr);
  ierr = OptionsGetInt(PETSC_NULL,"-height",&height,PETSC_NULL);CHKERRA(ierr);
  ierr = OptionsGetInt(PETSC_NULL,"-n",&n,PETSC_NULL);CHKERRA(ierr);
  ierr = OptionsHasName(PETSC_NULL,"-nolabels",&flg);CHKERRA(ierr); 
  if (flg) {
    xlabel = (char *)0; toplabel = (char *)0;
  }
  /* ierr = DrawOpenX(PETSC_COMM_SELF,0,"Title",x,y,width,height,&draw);CHKERRA(ierr);*/
  ierr = DrawCreate(PETSC_COMM_SELF,0,"Title",x,y,width,height,&draw);CHKERRA(ierr);
  ierr = DrawSetFromOptions(draw);CHKERRA(ierr);
  
  ierr = OptionsGetInt(PETSC_NULL,"-nports",&nports,PETSC_NULL);CHKERRA(ierr);
  ierr = DrawViewPortsCreate(draw,nports,&ports);CHKERRA(ierr);
  ierr = DrawViewPortsSet(ports,0);CHKERRA(ierr);

  ierr = DrawLGCreate(draw,1,&lg);CHKERRA(ierr);
  ierr = DrawLGGetAxis(lg,&axis);CHKERRA(ierr);
  ierr = DrawAxisSetColors(axis,DRAW_BLACK,DRAW_RED,DRAW_BLUE);CHKERRA(ierr);
  ierr = DrawAxisSetLabels(axis,toplabel,xlabel,ylabel);CHKERRA(ierr);

  for (i=0; i<n ; i++) {
    xd = (double)(i - 5); yd = xd*xd;
    ierr = DrawLGAddPoint(lg,&xd,&yd);CHKERRA(ierr);
  }
  ierr = DrawLGIndicateDataPoints(lg);CHKERRA(ierr);
  ierr = DrawLGDraw(lg);CHKERRA(ierr);
  ierr = DrawFlush(draw);CHKERRA(ierr);
  ierr = PetscSleep(2);CHKERRA(ierr);

  ierr = DrawViewPortsDestroy(ports);CHKERRA(ierr);
  ierr = DrawLGDestroy(lg);CHKERRA(ierr);
  ierr = DrawDestroy(draw);CHKERRA(ierr);
  PetscFinalize();
  return 0;
}
 
