
/*
    Code for managing color the X implementation of the PetscDraw routines.

    Currently we default to using cmapping[0 to PETSC_DRAW_BASIC_COLORS-1] for the basic colors and
    cmapping[DRAW_BASIC_COLORS to 255] for contour plots.

*/
#include <../src/sys/classes/draw/impls/x/ximpl.h>
#include <X11/Xatom.h>

static const char *colornames[PETSC_DRAW_BASIC_COLORS] = {"white",   "black",     "red",      "green",      "cyan",      "blue",       "magenta",   "aquamarine",      "forestgreen", "orange",        "violet",
                                                          "brown",   "pink",      "coral",    "gray",       "yellow",    "gold",       "lightpink", "mediumturquoise", "khaki",       "dimgray",       "yellowgreen",
                                                          "skyblue", "darkgreen", "navyblue", "sandybrown", "cadetblue", "powderblue", "deeppink",  "thistle",         "limegreen",   "lavenderblush", "plum"};

/*
   Sets up a color map for a display. This is shared by all the windows
  opened on that display; this is to save time when windows are open so
  each one does not have to create its own color map which can take 15 to 20 seconds

     This is new code written 2/26/1999 Barry Smith,I hope it can replace
  some older,rather confusing code.

     The calls to XAllocNamedColor() and XAllocColor() are very slow
     because we have to request from the X server for each
     color. Could not figure out a way to request a large number at the
     same time.

   IMPORTANT: this code will fail if user opens windows on two different
  displays: should add error checking to detect this. This is because all windows
  share the same gColormap and gCmapping.

*/
static Colormap          gColormap = 0;
static PetscDrawXiPixVal gCmapping[PETSC_DRAW_MAXCOLOR];
static unsigned char     gCpalette[PETSC_DRAW_MAXCOLOR][3];

static PetscErrorCode PetscDrawSetUpColormap_Shared(Display *display, int screen, Visual *visual, Colormap colormap)
{
  int           i, k, ncolors = PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS;
  unsigned char R[PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS];
  unsigned char G[PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS];
  unsigned char B[PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS];
  XColor        colordef, ecolordef;
  PetscBool     fast = PETSC_FALSE;

  PetscFunctionBegin;
  if (colormap) gColormap = colormap;
  else gColormap = DefaultColormap(display, screen);

  /* set the basic colors into the color map */
  for (i = 0; i < PETSC_DRAW_BASIC_COLORS; i++) {
    XAllocNamedColor(display, gColormap, colornames[i], &colordef, &ecolordef);
    gCmapping[i]    = colordef.pixel;
    gCpalette[i][0] = (unsigned char)(colordef.red >> 8);
    gCpalette[i][1] = (unsigned char)(colordef.green >> 8);
    gCpalette[i][2] = (unsigned char)(colordef.blue >> 8);
  }

  /* set the contour colors into the colormap */
  PetscCall(PetscOptionsGetBool(NULL, NULL, "-draw_fast", &fast, NULL));
  PetscCall(PetscDrawUtilitySetCmap(NULL, ncolors, R, G, B));
  for (i = 0, k = PETSC_DRAW_BASIC_COLORS; i < ncolors; i++, k++) {
    colordef.red   = (unsigned short)(R[i] << 8);
    colordef.green = (unsigned short)(G[i] << 8);
    colordef.blue  = (unsigned short)(B[i] << 8);
    colordef.flags = DoRed | DoGreen | DoBlue;
    colordef.pixel = gCmapping[PETSC_DRAW_BLACK];
    if (!fast) XAllocColor(display, gColormap, &colordef);
    gCmapping[k]    = colordef.pixel;
    gCpalette[k][0] = R[i];
    gCpalette[k][1] = G[i];
    gCpalette[k][2] = B[i];
  }

  PetscCall(PetscInfo(NULL, "Successfully allocated colors\n"));
  PetscFunctionReturn(PETSC_SUCCESS);
}

/*
    Keep a record of which pixel numbers in the cmap have been
  used so far; this is to allow us to try to reuse as much of the current
  colormap as possible.
*/
static PetscBool cmap_pixvalues_used[PETSC_DRAW_MAXCOLOR];
static int       cmap_base = 0;

static PetscErrorCode PetscDrawSetUpColormap_Private(Display *display, int screen, Visual *visual, Colormap colormap)
{
  int           found, i, k, ncolors = PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS;
  unsigned char R[PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS];
  unsigned char G[PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS];
  unsigned char B[PETSC_DRAW_MAXCOLOR - PETSC_DRAW_BASIC_COLORS];
  Colormap      defaultmap = DefaultColormap(display, screen);
  XColor        colordef;
  PetscBool     fast = PETSC_FALSE;

  PetscFunctionBegin;
  if (colormap) gColormap = colormap;
  else gColormap = XCreateColormap(display, RootWindow(display, screen), visual, AllocAll);

  cmap_base = 0;

  PetscCall(PetscMemzero(cmap_pixvalues_used, sizeof(cmap_pixvalues_used)));

  /* set the basic colors into the color map */
  for (i = 0; i < PETSC_DRAW_BASIC_COLORS; i++) {
    XParseColor(display, gColormap, colornames[i], &colordef);
    /* try to allocate the color in the default-map */
    found = XAllocColor(display, defaultmap, &colordef);
    /* use it, if it it exists and is not already used in the new colormap */
    if (found && colordef.pixel < PETSC_DRAW_MAXCOLOR && !cmap_pixvalues_used[colordef.pixel]) {
      cmap_pixvalues_used[colordef.pixel] = PETSC_TRUE;
      /* otherwise search for the next available slot */
    } else {
      while (cmap_pixvalues_used[cmap_base]) cmap_base++;
      colordef.pixel                   = cmap_base;
      cmap_pixvalues_used[cmap_base++] = PETSC_TRUE;
    }
    XStoreColor(display, gColormap, &colordef);
    gCmapping[i]    = colordef.pixel;
    gCpalette[i][0] = (unsigned char)(colordef.red >> 8);
    gCpalette[i][1] = (unsigned char)(colordef.green >> 8);
    gCpalette[i][2] = (unsigned char)(colordef.blue >> 8);
  }

  /* set the contour colors into the colormap */
  PetscCall(PetscOptionsGetBool(NULL, NULL, "-draw_fast", &fast, NULL));
  PetscCall(PetscDrawUtilitySetCmap(NULL, ncolors, R, G, B));
  for (i = 0, k = PETSC_DRAW_BASIC_COLORS; i < ncolors; i++, k++) {
    colordef.red   = (unsigned short)(R[i] << 8);
    colordef.green = (unsigned short)(G[i] << 8);
    colordef.blue  = (unsigned short)(B[i] << 8);
    colordef.flags = DoRed | DoGreen | DoBlue;
    colordef.pixel = gCmapping[PETSC_DRAW_BLACK];
    if (!fast) {
      /* try to allocate the color in the default-map */
      found = XAllocColor(display, defaultmap, &colordef);
      /* use it, if it it exists and is not already used in the new colormap */
      if (found && colordef.pixel < PETSC_DRAW_MAXCOLOR && !cmap_pixvalues_used[colordef.pixel]) {
        cmap_pixvalues_used[colordef.pixel] = PETSC_TRUE;
        /* otherwise search for the next available slot */
      } else {
        while (cmap_pixvalues_used[cmap_base]) cmap_base++;
        colordef.pixel                   = cmap_base;
        cmap_pixvalues_used[cmap_base++] = PETSC_TRUE;
      }
      XStoreColor(display, gColormap, &colordef);
    }
    gCmapping[k]    = colordef.pixel;
    gCpalette[k][0] = R[i];
    gCpalette[k][1] = G[i];
    gCpalette[k][2] = B[i];
  }

  PetscCall(PetscInfo(NULL, "Successfully allocated colors\n"));
  PetscFunctionReturn(PETSC_SUCCESS);
}

static PetscErrorCode PetscDrawSetUpColormap_X(Display *display, int screen, Visual *visual, Colormap colormap)
{
  PetscBool   sharedcolormap = PETSC_FALSE;
  XVisualInfo vinfo;

  PetscFunctionBegin;
  PetscCall(PetscOptionsGetBool(NULL, NULL, "-draw_x_shared_colormap", &sharedcolormap, NULL));
  /*
     Need to determine if window supports allocating a private colormap,
  */
  if (XMatchVisualInfo(display, screen, 24, StaticColor, &vinfo) || XMatchVisualInfo(display, screen, 24, TrueColor, &vinfo) || XMatchVisualInfo(display, screen, 16, StaticColor, &vinfo) || XMatchVisualInfo(display, screen, 16, TrueColor, &vinfo) || XMatchVisualInfo(display, screen, 15, StaticColor, &vinfo) || XMatchVisualInfo(display, screen, 15, TrueColor, &vinfo))
    sharedcolormap = PETSC_TRUE;
  /*
     Generate the X colormap object
  */
  if (sharedcolormap) {
    PetscCall(PetscDrawSetUpColormap_Shared(display, screen, visual, colormap));
  } else {
    PetscCall(PetscDrawSetUpColormap_Private(display, screen, visual, colormap));
  }
  PetscFunctionReturn(PETSC_SUCCESS);
}

PETSC_INTERN PetscErrorCode PetscDrawSetColormap_X(PetscDraw_X *, Colormap);

PetscErrorCode PetscDrawSetColormap_X(PetscDraw_X *XiWin, Colormap colormap)
{
  PetscBool fast = PETSC_FALSE;

  PetscFunctionBegin;
  PetscCall(PetscOptionsGetBool(NULL, NULL, "-draw_fast", &fast, NULL));
  PetscCheck(XiWin->depth >= 8, PETSC_COMM_SELF, PETSC_ERR_SUP_SYS, "PETSc Graphics require monitors with at least 8 bit color (256 colors)");
  if (!gColormap) PetscCall(PetscDrawSetUpColormap_X(XiWin->disp, XiWin->screen, XiWin->vis, colormap));
  XiWin->cmap     = gColormap;
  XiWin->cmapsize = fast ? PETSC_DRAW_BASIC_COLORS : PETSC_DRAW_MAXCOLOR;
  PetscCall(PetscMemcpy(XiWin->cmapping, gCmapping, sizeof(XiWin->cmapping)));
  PetscCall(PetscMemcpy(XiWin->cpalette, gCpalette, sizeof(XiWin->cpalette)));
  XiWin->background = XiWin->cmapping[PETSC_DRAW_WHITE];
  XiWin->foreground = XiWin->cmapping[PETSC_DRAW_BLACK];
  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode PetscDrawXiColormap(PetscDraw_X *XiWin)
{
  return PetscDrawSetColormap_X(XiWin, (Colormap)0);
}
