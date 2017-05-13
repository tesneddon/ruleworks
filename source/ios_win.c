/****************************************************************************
RuleWorks - Rules based application development tool.

Copyright (C) 1999  Compaq Computer Corporation

This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2 of the License, or any later 
version. 

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
Public License for more details. 

You should have received a copy of the GNU General Public License along 
with this program; if not, write to the Free Software Foundation, 
Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Email: info@ruleworks.co.uk
****************************************************************************/



/*
**  FACILITY:
**	RULEWORKS run time system and compiler
**
**  ABSTRACT:
**	This file contains I/O Stream (IOS) support for Microsoft Windows.
**	This module includes functions from the WINIO library, found in
**	Microsoft Systems Journal 1991 #4 (July).  That code has been changed
**	to work with a 32bit environment, and names have been declared with
**	no linkage (static) to avoid polluting the user's namespace.
**
**
**  MODIFIED BY:
**	DEC	Digital Equipment Corporation
 *	CPQ	Compaq Computer Corporation
**
**  MODIFICATION HISTORY:
**
**	 9-Apr-1994	DEc	Initial version
**
**	11-Oct-1994	DEC	Correct message parameters for WIN32.  Print
**					[Application exited] message.
**
**	21-Oct-1994	DEC	Add hack to free() --fpBuffer, since
**					initialize_buffers() does ++fpBuffer.
**
**	16-Feb-1998	DEC	Window title now RuleWorks
**
**	25-Feb-1998	DEC	__WIN32__ at GetInstanceData now WIN32
**
**	01-Dec-1999	CPQ	Release with GPL
*/


#include <common.h>
#ifndef WINDOWS
#error This module should only be compiled when targetting MS Windows.
#endif
#include <ios.h>		/* This module's interface */
#include <ios_p.h>		/* For IO_Stream, IOS__E_OUT_BIZZARE */
#include <mol.h>
#include <windows.h>		/* For SW_SHOWNORMAL */

static HINSTANCE ios_S__hInstance;

/* Forward declarations */

static void TinWinGet(char *buffer, long maxChars, void *p1, void *p2);
static char *TinWinPut(char *buffer, void *p1, void *p2);

/*****************************************************************************/
/*****************************************************************************/
/*
 *
 * Code from WINIO.H
 *
 */
/*
WINIO.H
Stdio (e.g. printf) functionality for Windows - definition
Dave Maxey - 1991
*/

#ifdef __cplusplus
extern "C" {
#endif
/* ==== STDIO.H for Windows ==== */
#include <stdarg.h>
#include <stdio.h>

#if 0
#undef putchar
#define putchar fputchar
#undef getchar
#define getchar fgetchar
#endif

/* ==== Redefined STDIO functions ==== */

static BYTE *winio_gets(BYTE *pchTmp);
static int winio_printf(const BYTE *fmt, ...);
static int winio_vprintf(const BYTE *fmt, va_list marker);
static int winio_puts(const BYTE *s);
static int winio_puts_nonewline(const BYTE *s);

/* ==== Extensions ==== */

/* winio_init() must be called before any of the above listed
functions to init the i/o window. Similar arguments to WinMain(), but
we drop the cmdline pointer but add a bufsize parameter (unsigned) -
0 means default (8k). */
static int winio_init(HANDLE, HANDLE, int, unsigned);

/* Makes the window inactive, and allows the user to view and play with
it until double- clicking on the system menu to close it. NEVER RETURNS. */
static void winio_end(void);

/* closes the window immediately and frees up buffers */
static void winio_close(void);

/* to override default title of "Console I/O" */
static void winio_settitle(BYTE *);

/* May be SYSTEM_FIXED_FONT (default), ANSI_FIXED_FONT, or OEM_FIXED_FONT */
static BOOL winio_setfont(WORD);

/* To turn automatic updating of window off and on */
static BOOL winio_setpaint(BOOL);

/* clear out the contents of the buffer and start over fresh */
static void winio_clear(void);

/* should be used to release cpu in spells between I/O calls. A
WM_QUIT message received by it will exit the application. If that is
a problem, use the winio_onclose function below */
static void winio_yield(void);

/* Returns the underlying Windows window handle to WINIO window */
static HWND winio_hwnd(void);

/* ==== User definable exit routine ==== */

typedef void (* DESTROY_FUNC)(void);

/* Optional notification function; without it, there is no way for your
application to know if the user has double-clicked the system menu box */
static void winio_onclose(DESTROY_FUNC);

/* ==== Utility function built on message box ==== */

static BOOL winio_warn(BOOL, const BYTE *, ...);
#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*
 *
 * Code from WMHANDLR.H
 *
 */
/*
WMHANDLR.H
Event (WM_ message) handlers - interface
Dave Maxey and Andrew Schulman - 1991

wmhandler_init MUST be called before a window is opened.

wmhandler_get returns current handler for an event.

wmhandler_set also returns current handler, and then makes
supplied handler current for the message type.
*/

#ifdef __cplusplus
extern "C" {
#endif
typedef LONG (*WMHANDLER)(HWND, UINT, UINT, LONG);

static LONG WINAPI WndProc(HWND, UINT, UINT, LONG);
static void wmhandler_init(void);
static WMHANDLER wmhandler_get(unsigned);
static WMHANDLER wmhandler_set(unsigned, WMHANDLER);
#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/*****************************************************************************/
/*
 *
 * "New" code
 *
 */

/*
 * Windows functions like CreateWindow() and RegisterClass() need to be
 * passed an instance handle.  The first two parameters to WinMain() are
 * the handle of the current instance.  WinMain() should pass those two
 * parameters to this function.
 */
void
rul_win_init(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious)
{
    ios_S__hInstance = hinstCurrent;
}

/*
 * Initialize standard I/O window.
 */
void
rul__ios_win_init(void)
{
    IO_Stream ios;
    Molecule mol;

    /* Create window and tell the RuleWorks RTL to use the window for I/O. */

    mol = rul__mol_make_symbol("STDIN-WINDOW");
    ios = rul__ios_open_bizzare(
		mol,
		(Puts_Function) TinWinPut, NULL, NULL,
		(Gets_Function) TinWinGet, NULL, NULL);
    rul__ios_set_stdin_stream(ios);
    rul__mol_decr_uses (mol);

    mol = rul__mol_make_symbol("STDOUT-WINDOW");
    ios = rul__ios_open_bizzare(
		mol,
		(Puts_Function) TinWinPut, NULL, NULL,
		(Gets_Function) TinWinGet, NULL, NULL);
    ios->type = IOS__E_OUT_BIZZARE;
    rul__ios_set_stdout_stream(ios);
    rul__mol_decr_uses (mol);

    mol = rul__mol_make_symbol("STDERR-WINDOW");
    ios = rul__ios_open_bizzare(
		mol,
		(Puts_Function) TinWinPut, NULL, NULL,
		(Gets_Function) TinWinGet, NULL, NULL);
    ios->type = IOS__E_OUT_BIZZARE;
    rul__ios_set_stderr_stream(ios);
    rul__mol_decr_uses (mol);
}

/*
 * If this is the first time reading from or writing to std_in, std_out, or
 * std_err, and we're running under Windows, open the window.
 */
void
rul__ios_ensure_window_open(IO_Stream ios)
{
    static BOOLEAN window_open = FALSE;

    if ((ios == RUL__C_STD_IN ||
	 ios == RUL__C_STD_OUT ||
	 ios == RUL__C_STD_ERR) &&
	!window_open) {
	window_open = TRUE;
	winio_settitle("RuleWorks");
	winio_init(ios_S__hInstance,
		   0,		/* Previous instance (not used) */
		   SW_SHOWNORMAL, /* Display the window */
		   0);		/* Size of buffer (use default) */
    }
}

/*
 * Gets_Function for stdout/stderr window.
 */
static char *
TinWinPut(char *buffer, void *p1, void *p2)
{
    winio_puts_nonewline(buffer);
    return buffer;
}

/*
 * Puts_Function for stdin window.
 */
static void
TinWinGet(char *buffer, long maxChars, void *p1, void *p2)
{
    winio_gets(buffer);
}

/*****************************************************************************/
/*****************************************************************************/
/*
 *
 * Code from WINIO.C
 *
 */
/*
WINIO.C
Stdio (e.g. printf) functionality for Windows - implementation
Dave Maxey - 1991

Larry Engholm, 4/7/94
	Added winio_ in front of following names
	    gets()
	    printf()
	    vprintf()
	    puts()
	Fixed return value of winio_puts()
	Added winio_puts_nonewline()
*/

/*#include <windows.h>*/
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>
/*#include "wmhandlr.h"*/
/*#include "winio.h"*/

/* PROTOTYPES in alphabetic order */

static void            addchars(BYTE *, unsigned);
static void            adjust_caret(void);
static void            append2buffer(BYTE *, unsigned);
static int             chInput(void);
static void            compute_repaint(void);
static int             initialize_buffers(unsigned);
static int             initialize_class(HANDLE);
static void            initialize_state(void);
static int             initialize_window(HANDLE, HANDLE, int);
static void            make_avail(unsigned);
static BYTE *          nextline(BYTE *);
static BYTE *          prevline(BYTE *);
static void            set_font(void);
static LONG            winio_wmpaint(HWND, UINT, UINT, LONG);
static LONG            winio_wmsize(HWND, UINT, UINT, LONG);
static LONG            winio_wmdestroy(HWND, UINT, UINT, LONG);
static LONG            winio_wmchar(HWND, UINT, UINT, LONG);
static LONG            winio_wmkeydown(HWND, UINT, UINT, LONG);
static LONG            winio_wmhscroll(HWND, UINT, UINT, LONG);
static LONG            winio_wmvscroll(HWND, UINT, UINT, LONG);
static LONG            winio_wmsetfocus(HWND, UINT, UINT, LONG);
static LONG            winio_wmkillfocus(HWND, UINT, UINT, LONG);

// this doesn't get declared in stdio.h if _WINDOWS is defined
// although it is in the Windows libraries!
//int             vsprintf(char *, const char *, va_list);

#define         winio_caret_visible() \
                ((yCurrLine <= (yTopOfWin + yWinHeight)) && \
                (xCurrPos <= (xLeftOfWin + xWinWidth)) && \
                (xCurrPos >= xLeftOfWin))

#define         CHECK_INIT() if (! tWinioVisible) return FALSE
                 
#define         MAX_X                   127
#define         TABSIZE                 8
#define         TYPE_AHEAD              256
#define         WINIO_DEFAULT_BUFFER    8192
#define         MIN_DISCARD             256
#define         CARET_WIDTH             2

// For scrolling procedures
#define         USE_PARAM               10000
#define         DO_NOTHING              10001

static BYTE            winio_class[15] = "winio_class";
static BYTE            winio_icon[15] = "winio_icon";
static BYTE            winio_title[128] = "Console I/O";
static unsigned long   bufsize = WINIO_DEFAULT_BUFFER;
static unsigned long   kbsize = TYPE_AHEAD;
static unsigned        bufused, bufSOI;
static unsigned        curr_font = SYSTEM_FIXED_FONT;
static int             tWinioVisible = FALSE;
static int             tCaret = FALSE, tFirstTime = TRUE;
static int             cxChar, cyChar, cxScroll, cyScroll, cxWidth, cyHeight;
static int             xWinWidth, yWinHeight, xCurrPos;
static int             xLeftOfWin, yTopOfWin, yCurrLine;
static unsigned        pchKbIn, pchKbOut;
static BYTE     *fpBuffer, *fpTopOfWin, *fpCurrLine; 
static BYTE     *fpKeyboard;
//static HANDLE   hBuffer, hKeyboard;
static HWND     hwnd;
static BOOL            tTerminate = TRUE,
                       tPaint = TRUE;
static DESTROY_FUNC    destroy_func;

typedef struct {
    int hSB, vSB;
    } recVKtoSB;
                
/* This table defines, by scroll message, what increment to try */
/* and scroll horizontally. PGUP and PGDN entries are updated   */
/* in the winio_wmsize function.                                */
static int             cScrollLR[SB_ENDSCROLL + 1] =
//UP  DOWN PGUP     PGDN    POS        TRACK      TOP     BOT    ENDSCROLL
{ -1, +1,  -1,      +1,     USE_PARAM, USE_PARAM, -MAX_X, MAX_X, DO_NOTHING};
                
/* This table defines, by scroll message, what increment to try */
/* and scroll horizontally. PGUP and PGDN entries are updated   */
/* in the winio_wmsize function, and the TOP & BOTTOM entries   */
/* are updated by addchar function.                             */
static int             cScrollUD[SB_ENDSCROLL + 1] =
//UP  DOWN PGUP     PGDN    POS        TRACK      TOP     BOT    ENDSCROLL
{ -1, +1,  -1,      +1,     USE_PARAM, USE_PARAM, -1,     +1,    DO_NOTHING};
                
/* This table associates horizontal and vertical scroll         */
/* messages that should be generated by the arrow and page keys */
static recVKtoSB       VKtoSB[VK_DOWN - VK_PRIOR + 1] =
//                  VK_PRIOR                    VK_NEXT
                {   { DO_NOTHING, SB_PAGEUP },  { DO_NOTHING, SB_PAGEDOWN },
//                  VK_END                      VK_HOME
                    { SB_TOP, SB_BOTTOM },      { SB_TOP, SB_TOP },
//                  VK_LEFT                     VK_UP
                    { SB_LINEUP, DO_NOTHING },  { DO_NOTHING, SB_LINEUP },
//                  VK_RIGHT                    VK_DOWN
                    { SB_LINEDOWN, DO_NOTHING },{ DO_NOTHING, SB_LINEDOWN } };
                
/* ===================================================================  */
/* the interface functions themselves.....                              */
/* ===================================================================  */

static BYTE *winio_gets(BYTE *pchTmp)
    {
    BYTE *pch = pchTmp;
    int c;

    CHECK_INIT();
    bufSOI = bufused; /* mark beginning of line to limit backspace */
    do {
        switch (c = fgetchar())
            {
            case '\b' :     if (pch > pchTmp) pch--; break;
            case 0x1b :     pch = pchTmp; break;
            case EOF :      bufSOI = -1; return NULL;
            default :       *pch = (BYTE) c; pch++;
            }
        } while (c);
    *(pch-1) = (BYTE) '\n';	/* Hack */
    *pch = (BYTE) '\0';		/* Unnecessary hack? */
    bufSOI = -1;
    return pchTmp;
    }

static int winio_printf(const BYTE *fmt, ...)
    {
    va_list marker;
    va_start(marker, fmt);
    return winio_vprintf(fmt, marker);
    }

static int winio_vprintf(const BYTE *fmt, va_list marker)
    {
    static BYTE s[1024];
    int len;

    CHECK_INIT();
    len = vsprintf(s, fmt, marker);
    addchars(s,len);
    return len;
    }

static int fgetchar(void)
    {
    CHECK_INIT();
    return fputchar(chInput());
    }

static int kbhit(void)
    {
    CHECK_INIT();
    return (pchKbIn == pchKbOut);
    }

static int fputchar(int c)
    {
    CHECK_INIT();
    addchars((BYTE *)&c, 1);
    return c;
    }

static int winio_puts(const BYTE *s)
    {
    BYTE c = '\n';
    int len = strlen(s);
    CHECK_INIT();
    addchars((BYTE *) s, len);
    addchars(&c, 1);
    return len + 1;		/* +1 for newline */
    }

static int winio_puts_nonewline(const BYTE *s)
    {
    int len = strlen(s);
    CHECK_INIT();
    addchars((BYTE *) s, len);
    return len;
    }

/* ---------------------------------------------------------------  */
/* USED INTERNALLY - pops up an error window and returns FALSE      */
/* ---------------------------------------------------------------  */
static int fail(BYTE *s)
    {
    MessageBox(NULL,s,"ERROR",MB_OK);
    return FALSE;
    }

/* ---------------------------------------------------------------  */
/* pops up a message window                                         */
/* ---------------------------------------------------------------  */
static BOOL winio_warn(BOOL confirm, const BYTE *fmt, ...)
    {
    BYTE s[256];
    va_list marker;

    va_start(marker, fmt);
    vsprintf(s, fmt, marker);
    va_end(marker);
    
    return (MessageBox(NULL, s, winio_title, 
        confirm? MB_OKCANCEL : MB_OK) == IDOK);
    }

/* ---------------------------------------------------------------  */
/* The application must call this function before using any of the  */
/* covered stdio type calls. We need the parameter info in order    */
/* to create the window. The function allocates the buffer and      */
/* creates the window. It returns TRUE or FALSE.                    */
/* ---------------------------------------------------------------  */
static int winio_init(HANDLE hInstance, HANDLE hPrevInstance,
            int nCmdShow, unsigned wBufSize)
    {
    if (tWinioVisible)
        return FALSE;
    
    if (! initialize_buffers(wBufSize))
        return FALSE;

    initialize_state();
    
    if (! initialize_window(hInstance, hPrevInstance, nCmdShow))
        return FALSE;
    
    tWinioVisible = TRUE;
    
    atexit(winio_end);  /* hook into exit chain */

    winio_yield();
    return TRUE;
    }

/* ---------------------------------------------------------------  */
/* Clear the contents of the buffer.                                */
/* ---------------------------------------------------------------  */
static void winio_clear(void)
    {
    memset(fpBuffer,0,(int) bufsize - 1);
    fpCurrLine = fpTopOfWin = fpBuffer;
    *fpBuffer = '\0';
    xCurrPos = 0;
    yCurrLine = 0;
    yTopOfWin = 0;
    xLeftOfWin = 0;
    bufused = 0;

    if (tWinioVisible)
        {
        SetScrollRange(hwnd, SB_VERT, 1, yCurrLine + 1, FALSE);
        SetScrollPos(hwnd, SB_VERT, yTopOfWin + 1, TRUE);
        }
    }

/* ---------------------------------------------------------------  */
/* Return the window handle of the underlying Windows object.       */
/* Can be used by an application to customize the WINIO window      */
/* ---------------------------------------------------------------  */
static HWND winio_hwnd(void)
    {
    return hwnd;
    }

/* ---------------------------------------------------------------  */
/* This function is called by winio_init(). It initializes a number */
/* of global variables, including the WM_ handler table.            */
/* ---------------------------------------------------------------  */
static void initialize_state()
    {
    winio_clear();
    destroy_func = 0;
    
    /* set up our message handlers */
    wmhandler_init();
    wmhandler_set(WM_PAINT,       winio_wmpaint);
    wmhandler_set(WM_SIZE,        winio_wmsize);
    wmhandler_set(WM_DESTROY,     winio_wmdestroy);
    wmhandler_set(WM_CHAR,        winio_wmchar);
    wmhandler_set(WM_HSCROLL,     winio_wmhscroll);
    wmhandler_set(WM_VSCROLL,     winio_wmvscroll);
    wmhandler_set(WM_SETFOCUS,    winio_wmsetfocus);
    wmhandler_set(WM_KILLFOCUS,   winio_wmkillfocus);
    wmhandler_set(WM_KEYDOWN,     winio_wmkeydown);
    }

/* ---------------------------------------------------------------  */
/* This function is called by winio_init(). It initializes our      */
/* Windows class, and some global variables                         */
/* ---------------------------------------------------------------  */
static int initialize_window(HANDLE hInst, HANDLE hPrev, int nCmdShow)
    {
    static RECT start;
    int cx, cy, inc;

    cx = GetSystemMetrics(SM_CXSCREEN);
    cy = GetSystemMetrics(SM_CYSCREEN);
    inc = GetSystemMetrics(SM_CYCAPTION);
    cxScroll = GetSystemMetrics(SM_CXVSCROLL);
    cyScroll = GetSystemMetrics(SM_CYHSCROLL);

#ifndef WIN32
    if (hPrev)
        {
        // note: other WINIO apps are NOT other instances!
        GetInstanceData(hPrev, (NPSTR) &start, sizeof(RECT));
        start.top += inc;
        start.left += inc;
        if (start.top > (cy >> 2))
            start.top = cy >> 3;
        if (start.left > (cx >> 2))
            start.left = cx >> 3;
        }
    else
#endif
        {
        if (! initialize_class(hInst)) 
            return fail("Could not create class");

        start.left = cx >> 3;
        start.right = 3 * (cx >> 2);
        start.top = cy >> 3;
        start.bottom = 3 * (cy >> 2);
        }
        
    hwnd = CreateWindow(winio_class, winio_title,
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        start.left, start.top, start.right, start.bottom,
        NULL, NULL, hInst, NULL);
    if (! hwnd)
        return fail("Could not create window");

    set_font();
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return TRUE;
    }

/* -----------------------------------------------------------------------  */
/* Initializes Window Class                                                 */
/* -----------------------------------------------------------------------  */
static int initialize_class(HANDLE hInst)
    {
    WNDCLASS  wc;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(hInst, winio_icon);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName = NULL;
    wc.lpszClassName = winio_class;

    return RegisterClass(&wc);
    }
    
/* -----------------------------------------------------------------------  */
/* Uses GlobalAlloc() to allocate the display and keyboard buffers          */
/* -----------------------------------------------------------------------  */
static int initialize_buffers(unsigned wBufSize)
    {
    if (wBufSize)
        bufsize = max(wBufSize, 1024);

    if (! (/*h*/fpBuffer = /*GlobalAlloc(GMEM_MOVEABLE, bufsize)*/ malloc(bufsize)))
        return fail("Could not allocate\nconsole I/O buffer");
    
//    fpBuffer = GlobalLock(hBuffer); // keep locked; assume protected mode
    
    if (! (/*h*/fpKeyboard = /*GlobalAlloc(GMEM_MOVEABLE, kbsize)*/ malloc(kbsize)))
        return fail("Could not allocate\ntype ahead buffer");
        
//    fpKeyboard = GlobalLock(hKeyboard);

    *fpBuffer = '\0';
    fpBuffer++;

    return TRUE;
    }

/* -----------------------------------------------------------------------  */
/* Undoes the work of the above. Allows an application to close the window  */
/* Terminates the prog.                                                     */
/* -----------------------------------------------------------------------  */
static void winio_end()
    {
    winio_printf("\n[Application exited]");
    while (tWinioVisible)
        winio_yield();
    }

/* -------------------------------------------------------------------  */
/* Closes the window by sending it a WM_DESTROY message. Note that it   */
/* does not disable the _onclose defined function. So the user program  */
/* handler will be triggered. Does NOT cause the app. to terminate.     */
/* -------------------------------------------------------------------  */
static void winio_close()
    {
    tTerminate = FALSE;
    DestroyWindow(hwnd);
    tTerminate = TRUE;
    }
    
/* -------------------------------------------------------------------  */
/* processes any outstanding events waiting. These may be characters    */
/* typed at the keyboard, WM_PAINT messages, etc. It is called          */
/* internally but should also be used liberally by the application      */
/* within loops.                                                        */
/* -------------------------------------------------------------------  */
static void winio_yield()
    {
    MSG msg;
    while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    }

/* -------------------------------------------------------------------  */
/* Let the application install an exit routine, called back from        */
/* winio_wmdestroy(). Deinstall by winio_onclose(NULL)                  */
/* -------------------------------------------------------------------  */
static void winio_onclose(DESTROY_FUNC exitfunc)
    {
    destroy_func = exitfunc;
    }

/* -------------------------------------------------------------------  */
/* This function allows the font of the window to be modified, and may  */
/* be used BEFORE winio_init. Currently, only SYSTEM_, ANSI_, and       */
/* OEM_FIXED_FONTs are supported.                                       */
/* -------------------------------------------------------------------  */
static BOOL winio_setfont(WORD wFont)
    {
    if ((wFont != SYSTEM_FIXED_FONT) &&
        (wFont != ANSI_FIXED_FONT) &&
        (wFont != OEM_FIXED_FONT) )
        return FALSE;
    curr_font = wFont;
    if (tWinioVisible)
        {
        set_font();
        if (tPaint)
            InvalidateRect(hwnd, NULL, TRUE);
        }
    return TRUE;
    }

/* -------------------------------------------------------------------  */
/* This function allows the title of the window to be modified, and may */
/* be used BEFORE winio_init.                                           */
/* -------------------------------------------------------------------  */
static void winio_settitle(BYTE *pchTitle)
    {
    strncpy(winio_title, pchTitle, 127);
    winio_title[127] = '\0';
    if (tWinioVisible)
        SetWindowText(hwnd, winio_title);
    }

/* -------------------------------------------------------------------  */
/* This function allows the caller to specifiy immediate or deferred    */
/* screen updates. The call may not be issued before winio_init().      */
/* -------------------------------------------------------------------  */
static BOOL winio_setpaint(BOOL on)
    {
    BOOL ret = tPaint;
    
    CHECK_INIT();
    if (tPaint = on)
        InvalidateRect(hwnd, NULL, TRUE);
    return ret;
    }

/* ---------------------------------------------------------------  */
/* Our WM_PAINT handler. It sends the currrent 'in view' piece of   */
/* the buffer to the window. Note that an embedded NULL character   */
/* signifies an end of line, not '\n'.                              */
/* ---------------------------------------------------------------  */
static LONG winio_wmpaint(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    HDC hdc;
    PAINTSTRUCT ps;
    BYTE *pchSOL = fpTopOfWin;
    BYTE *pchEOL;
    int i, j, xStart;
    int xLeft, xRight, yTop, yBottom;

    hdc = BeginPaint(hwnd, &ps);

    xLeft = (ps.rcPaint.left / cxChar) + xLeftOfWin;
    xRight = (ps.rcPaint.right / cxChar) + xLeftOfWin;
    yTop = ps.rcPaint.top / cyChar;
    yBottom = ps.rcPaint.bottom / cyChar;
    SelectObject(hdc, GetStockObject(curr_font));

    for (i = 0; i < yTop; i++)      // lines above repaint region
        {
        while (*pchSOL)
            pchSOL++;
        pchSOL++;
        }

    if (i <= yCurrLine) // something needs repainting..
        {
        for (i = yTop; i <= yBottom; i++)   // lines in repaint region
            {
            for (j = 0; (j < xLeft) && (*pchSOL); j++, pchSOL++)
                ; // Scroll right
            pchEOL = pchSOL;
            xStart = j - xLeftOfWin;
            for (j = 0; (*pchEOL) ; j++, pchEOL++) ; // end of line
            TextOut(hdc, cxChar * xStart, cyChar * i, pchSOL,
                    min(j, xRight - xLeft + 2));
            if ((unsigned)(pchEOL - fpBuffer) >= bufused)
                break;
            pchSOL = ++pchEOL;  
            }
        }
    
    EndPaint(hwnd, &ps);
    adjust_caret();
    return 0;
    }

/* ---------------------------------------------------------------  */
/* Our WM_SIZE handler. It updates the internal record of our       */
/* window size, minus the scroll bars, and recalcs the scroll bar   */
/* ranges.                                                          */
/* ---------------------------------------------------------------  */
static LONG winio_wmsize(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    cxWidth = LOWORD(lParam);
    cyHeight = HIWORD(lParam);

    xWinWidth   = (cxWidth - cxScroll ) / cxChar;
    yWinHeight  = (cyHeight - cyScroll ) / cyChar;

    cScrollLR[SB_PAGEUP]    = -xWinWidth / 2;
    cScrollLR[SB_PAGEDOWN]  = +xWinWidth / 2;
    cScrollUD[SB_PAGEUP]    = -yWinHeight + 1;
    cScrollUD[SB_PAGEDOWN]  = +yWinHeight - 1;
    
    SetScrollRange(hwnd, SB_HORZ, 1, MAX_X, FALSE);
    SetScrollPos(hwnd, SB_HORZ, xLeftOfWin + 1, TRUE);

    SetScrollRange(hwnd, SB_VERT, 1, yCurrLine + 1, FALSE);
    SetScrollPos(hwnd, SB_VERT, yTopOfWin + 1, TRUE);
    
    return 0;
    }

/* ---------------------------------------------------------------  */
/* Our WM_DESTROY handler. It frees up storage associated with the  */
/* window, and resets its state to uninitialized.                   */
/* ---------------------------------------------------------------  */
static LONG winio_wmdestroy(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    if (destroy_func)
        (*destroy_func)();
//    GlobalUnlock(hBuffer);
//    GlobalUnlock(hKeyboard);
/*
 * Notice the strange 'fpBuffer++' in initialize_buffers().  Odd.  To account
 * for that, we need to free() --fpBuffer.  NT caught this bug.
 */
    /*GlobalF*/free(/*h*/--fpBuffer);
    /*GlobalF*/free(/*h*/fpKeyboard);
    tWinioVisible = FALSE;
    if (tTerminate) exit(0);
    return 0;
    }

/* --------------------------------------------------------------- */
/* Our WM_BYTE handler. It adds the BYTE to the internal kb buffer */
/* if there is room otherwise it queues a BEEP                     */
/* --------------------------------------------------------------- */
static LONG winio_wmchar(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    /*unsigned*/ BYTE *lpchKeybd = fpKeyboard;
    unsigned pchSave = pchKbIn;
    
    pchKbIn++;
    if (pchKbIn == TYPE_AHEAD)
        pchKbIn = 0;
    if (pchKbIn == pchKbOut)
        {
        MessageBeep(0);
        pchKbIn = pchSave;
        }
    else
        *(lpchKeybd + pchSave) = LOBYTE(wParam);

    return 0;
    }

/* ---------------------------------------------------------------  */
/* Our WM_KEYDOWN handler. This handles what would be called        */
/* function keys in the DOS world. In this case the function keys   */
/* operate as scroll bar controls, so we generate messages to the   */
/* scroll message handlers below.                                   */
/* ---------------------------------------------------------------  */
static LONG winio_wmkeydown(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    int hSB, vSB;
    
    if ((wParam < VK_PRIOR) || (wParam > VK_DOWN))
        return 0;
    
    hSB = VKtoSB[wParam - VK_PRIOR].hSB;
    vSB = VKtoSB[wParam - VK_PRIOR].vSB;
    if (hSB != DO_NOTHING)
        SendMessage(hwnd, WM_HSCROLL, hSB, 0L);
    if (vSB != DO_NOTHING)
        SendMessage(hwnd, WM_VSCROLL, vSB, 0L);
    return 0;
    }

/* --------------------------------------------------------------- */
/* Our WM_HSCROLL handler. It adjusts what part of the buffer      */
/* is visible. It operates as left/right arrow keys.               */
/* --------------------------------------------------------------- */
static LONG winio_wmhscroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    int cxSave = xLeftOfWin,
        xInc = cScrollLR[LOWORD(wParam)];
    
    if (xInc == DO_NOTHING)
        return 0;
    else if (xInc == USE_PARAM)
#ifdef WIN32
        xLeftOfWin = HIWORD(wParam) - 1;
#else
        xLeftOfWin = LOWORD(lParam) - 1;
#endif
    else
        xLeftOfWin += xInc;
    
    if ((xLeftOfWin = max(0, min(MAX_X - 1, xLeftOfWin))) == cxSave)
        return 0;

    ScrollWindow(hwnd, (cxSave - xLeftOfWin) * cxChar, 0, NULL, NULL);
    SetScrollPos(hwnd, SB_HORZ, xLeftOfWin + 1, TRUE);
    UpdateWindow(hwnd);

    return 0;
    }

/* --------------------------------------------------------------- */
/* Our WM_VSCROLL handler. It adjusts what part of the buffer      */
/* is visible. It operates as page and line up/down keys.          */
/* --------------------------------------------------------------- */
static LONG winio_wmvscroll(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    int cySave = yTopOfWin,
        yInc = cScrollUD[LOWORD(wParam)],
        i;
    
    if (yInc == DO_NOTHING)
        return 0;
    else if (yInc == USE_PARAM)
#ifdef WIN32
        yTopOfWin = HIWORD(wParam) - 1;
#else
        yTopOfWin = LOWORD(lParam) - 1;
#endif
    else
        yTopOfWin += yInc;

    if ((yTopOfWin = max(0, min(yCurrLine, yTopOfWin))) == cySave)
        return 0;

    if (yTopOfWin > cySave)
        for (i = cySave; i < yTopOfWin; i++)
            fpTopOfWin = nextline(fpTopOfWin);
    else
        for (i = cySave; i > yTopOfWin; i--)
            fpTopOfWin = prevline(fpTopOfWin);
        
    ScrollWindow(hwnd, 0, (cySave - yTopOfWin) * cyChar, NULL, NULL);
    SetScrollPos(hwnd, SB_VERT, yTopOfWin + 1, TRUE);
    UpdateWindow(hwnd);

    return 0;
    }

/* ---------------------------------------------------------------  */
/* Our WM_SETFOCUS handler. It sets up the text caret.              */
/* ---------------------------------------------------------------  */
static LONG winio_wmsetfocus(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {
    CreateCaret(hwnd, NULL, CARET_WIDTH, cyChar);
    
    if ((tCaret = winio_caret_visible()))
        {
        SetCaretPos((xCurrPos - xLeftOfWin) * cxChar,
                    (yCurrLine - yTopOfWin) * cyChar);
        ShowCaret(hwnd);
        }

    return 0;
    }

/* ---------------------------------------------------------------  */
/* Our WM_KILLFOCUS handler. It destroys the text caret.            */
/* ---------------------------------------------------------------  */
static LONG winio_wmkillfocus(HWND hwnd, UINT message, UINT wParam,
			      LONG lParam)
    {
    if (tCaret)
        {
        HideCaret(hwnd);
        tCaret = FALSE;
        }
    DestroyCaret();
    return 0;
    }

static void set_font(void)
    {
    HDC hdc;
    TEXTMETRIC tm;
        
    hdc = GetDC(hwnd);
    SelectObject(hdc, GetStockObject(curr_font));
    GetTextMetrics(hdc,&tm);
    ReleaseDC(hwnd,hdc);
    cxChar = tm.tmAveCharWidth;
    cyChar = tm.tmHeight+tm.tmExternalLeading;
    xWinWidth   = (cxWidth - cxScroll ) / cxChar;
    yWinHeight  = (cyHeight - cyScroll ) / cyChar;
    }

/* ---------------------------------------------------------------  */
/* Adjusts the position of the caret, and shows or hides it, as     */
/* appropriate.                                                     */
/* ---------------------------------------------------------------  */
static void adjust_caret()
    {
    int t = winio_caret_visible();

    if (t)
        SetCaretPos((xCurrPos - xLeftOfWin) * cxChar,
                    (yCurrLine - yTopOfWin) * cyChar);
    if (t && (! tCaret))
        ShowCaret(hwnd);
    if ((! t) && tCaret)
        HideCaret(hwnd);
    tCaret = t;
    }

/* ---------------------------------------------------------------  */
/* Computes, on the basis of what has just been updated, what area  */
/* of the window needs to be repainted.                             */
/* ---------------------------------------------------------------  */
static void compute_repaint(void)
    {
    RECT rc;
    static int xCP = 0, yCL = 0;
    int tWholeWin = FALSE;
    
    if (yCurrLine > (yTopOfWin + yWinHeight))
        {
        yTopOfWin = 0;
        fpTopOfWin = fpBuffer;
        while (yTopOfWin < (yCurrLine - ((yWinHeight + 1) / 2)))
            {
            fpTopOfWin = nextline(fpTopOfWin);
            yTopOfWin++;
            }
        tWholeWin = TRUE;
        }

    if ((xCurrPos < xLeftOfWin) || (xCurrPos > (xLeftOfWin + xWinWidth)))
        {
        xLeftOfWin = 0;
        while (xLeftOfWin < (xCurrPos - ((xWinWidth + 1) / 2)))
            xLeftOfWin++;
        tWholeWin = TRUE;
        }

    if (tWholeWin)
        InvalidateRect(hwnd, NULL, TRUE);
    else
        {
        rc.left = ((yCL == yCurrLine) ?
            (min(xCP, xCurrPos) - xLeftOfWin) * cxChar : 0);
        rc.top = (yCL - yTopOfWin) * cyChar;
        rc.right = (xWinWidth + 1) * cxChar;
        rc.bottom = (yCurrLine - yTopOfWin + 1) * cyChar;
        InvalidateRect(hwnd, &rc, TRUE);
        }
    
    yCL = yCurrLine;
    xCP = xCurrPos;
    }

/* ---------------------------------------------------------------  */
/* Adds the supplied cch-long string to the display buffer, and     */
/* ensures any changed part of the window is repainted.             */
/* ---------------------------------------------------------------  */
static void addchars(BYTE *pch, unsigned cch)
    {
    int ycSave = yCurrLine;
    int ytSave = yTopOfWin;
    int xSave = xLeftOfWin;

    make_avail(cch);

    append2buffer(pch, cch);

    if (ycSave != yCurrLine)
        SetScrollRange(hwnd, SB_VERT, 1, yCurrLine + 1, FALSE);

    if (! tPaint)
        return;
    
    compute_repaint();

    cScrollUD[SB_TOP]       = -yCurrLine;
    cScrollUD[SB_BOTTOM]    = yCurrLine;
    if (ytSave != yTopOfWin)
        SetScrollPos(hwnd, SB_VERT, yTopOfWin + 1, TRUE);       

    if (xSave != xLeftOfWin)
        SetScrollPos(hwnd, SB_HORZ, xLeftOfWin + 1, TRUE);

    winio_yield();
    }

/* ---------------------------------------------------------------  */
/* Add chars onto the display buffer, wrapping at end of line,      */
/* expanding tabs, etc.                                             */
/* ---------------------------------------------------------------  */
static void append2buffer(BYTE *pch, unsigned cch)
    {
    unsigned i;
    
    for (i = 0; i < cch; i++, pch++)
        {
        switch (*pch)
            {
            case '\n' :
                *pch = '\0';
                *(fpBuffer + bufused) = '\0';
                bufused++;
                fpCurrLine = fpBuffer + bufused;
                yCurrLine++;
                xCurrPos = 0;
                bufSOI = bufused;
                break;
            case '\t' :
                do  {
                    *(fpBuffer + bufused) = ' ';
                    bufused++;
                    xCurrPos++;
                    } while ((xCurrPos % TABSIZE) != 0);
                break;
            case EOF :
                break;
            case '\b' :
                if (bufused > bufSOI)
                    {
                    bufused--;
                    xCurrPos--;
                    }
                break;
            case 0x1b :
                while (bufused > bufSOI)
                    {
                    bufused--;
                    xCurrPos--;
                    }
                break;
            case 0x07 :
                MessageBeep(0);
                break;
            default :
                if (*pch > 0x1a)
                    {
                    if (xCurrPos >= MAX_X)
                        {
                        *(fpBuffer + bufused) = '\0';
                        bufused++;
                        xCurrPos = 0;
                        yCurrLine++;
                        fpCurrLine = fpBuffer + bufused;
                        }
                    xCurrPos++;
                    *(fpBuffer + bufused) = *pch;
                    bufused++;
                    }
            }
        }
    
    *(fpBuffer + bufused) = '\0'; // '\0' terminator after end of buffer
    }

/* ---------------------------------------------------------------  */
/* If we have run out of room in the display buffer, drop whole     */
/* lines, and move the remaining buffer up.                         */
/* ---------------------------------------------------------------  */
static void make_avail(unsigned cch)
    {
    unsigned cDiscard = 0;
    BYTE *fpTmp;
    unsigned i;

    if ((unsigned long)(bufused + cch + TABSIZE) < bufsize)
        return;

    fpTmp = fpBuffer;
    cDiscard = ((max(MIN_DISCARD, cch + 1) + MIN_DISCARD - 1)
        / MIN_DISCARD)      // this gives a whole number of
        * MIN_DISCARD;      // our allocation units.
    fpTmp += (LONG) cDiscard;
    fpTmp = nextline(fpTmp);
    cDiscard = fpTmp - fpBuffer; 
    memcpy(fpBuffer, fpTmp, bufused - cDiscard + 1);
    bufused -= cDiscard;
    if ((int) bufSOI != -1) bufSOI -= cDiscard;
    fpTmp = fpBuffer + (LONG) bufused;
    for (i = 0; i < cDiscard; i++) *fpTmp++ = '\0';
    fpCurrLine = fpBuffer;
    xCurrPos = yCurrLine = 0;
    for (i = 0; i < bufused; i++)
        {
        if (*fpCurrLine)
            xCurrPos++;
        else
            {
            xCurrPos = 0;
            yCurrLine++;
            }
        fpCurrLine++;
        }
    xLeftOfWin = yTopOfWin = -9999;
    
    InvalidateRect(hwnd, NULL, TRUE);
    }


/* -------------------------------------------------------------------  */
/* These two routines find the beginning of the next, and previous      */
/* lines relative to their input pointer                                */
/* -------------------------------------------------------------------  */

static BYTE *nextline(BYTE *p) { while (*p) p++; return ++p; }
static BYTE *prevline(BYTE *p) { p--; do p--; while (*p); return ++p; }

/* -------------------------------------------------------------------  */
/* Waits for a character to appear in the keyboard buffer, yielding     */
/* while nothing is available. Then inserts it into the buffer.         */
/* -------------------------------------------------------------------  */
static int chInput(void)
    {
    BYTE *lpchKeyBd;
    BYTE c;
    
    CHECK_INIT();
    while (pchKbIn == pchKbOut)
        winio_yield();
        
    lpchKeyBd = fpKeyboard;
    c = *(lpchKeyBd + pchKbOut);

    pchKbOut++;
    if (pchKbOut == TYPE_AHEAD)
        pchKbOut = 0;
    
    // Do CR/LF and EOF translation
    return (c == 0x1a) ? EOF : (c == '\r') ? '\n' : c;
    }

/*****************************************************************************/
/*****************************************************************************/
/*
 *
 * Code from WMHANDLR.C
 *
 */
/*
WMHANDLR.C
Event (WM_ message) handlers - implementation
Dave Maxey and Andrew Schulman - 1991
*/

/*#include <windows.h>*/
/*#include "wmhandlr.h"*/

static LONG defwmhandler(HWND hwnd, UINT message, UINT wParam, LONG lParam);

static WMHANDLER       wmhandler[WM_USER];

/* -----------------------------------------------------------------------  */
/* This is our event processor. It is the dispatcher for the handlers set   */
/* using SetHandler. An Application plugs this function into its            */
/* window, sets handlers for messages, and WndProc handles the rest.        */
/* This window procecedure should never need to be changed!                 */
/* -----------------------------------------------------------------------  */
static LONG WINAPI WndProc(HWND hWnd, UINT message, 
			   UINT wParam, LONG lParam)
    {
    if (message < WM_USER)
        return (*wmhandler[message])(hWnd, message, wParam, lParam);
    else
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

/* ---------------------------------------------------------------- */
/* Routines to get and set the message handlers. Setting to NULL    */
/* uninstalls a handler, by setting the handler to the default      */
/* which calls Windows own DefWndProc.                              */
/* ---------------------------------------------------------------- */
static WMHANDLER wmhandler_get(unsigned message)
    {
    return (message < WM_USER) ? wmhandler[message] : 0;
    }

static WMHANDLER wmhandler_set(unsigned message, WMHANDLER f)
    {
    WMHANDLER oldf;
    if (message < WM_USER)
        {
        oldf = wmhandler[message];
        wmhandler[message] = f ? f : defwmhandler;
        return (oldf ? oldf : defwmhandler);
        }
    else
        return 0;
    }

/* ----------------------------------------------------------------------- */
/* This is a default handler so that an application chain on to a previous */
/* handler from their current one without having to worry what was there   */
/* before. All this default handler does is to call DefWindowProc.         */
/* ----------------------------------------------------------------------- */
static LONG defwmhandler(HWND hwnd, UINT message, UINT wParam, LONG lParam)
    {   
    return DefWindowProc(hwnd, message, wParam, lParam);
    }

/* -------------------------------------------------------------------  */
/* MUST BE CALLED BEFORE THE APPLICATION WINDOW IS CREATED -            */
/* Just inits the array of handlers to the default..                    */
/* -------------------------------------------------------------------  */
static void wmhandler_init(void)
    {
    WMHANDLER *pwm;
    int i;
    for (i=0, pwm=wmhandler; i < WM_USER; i++, pwm++) 
        *pwm = defwmhandler;
    }
