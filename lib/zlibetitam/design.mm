.\"ident	"@(#)stitam:design.mm	1.3"
.lg 0
.ds cu curses(3X)
.nr Pt 0
.TL
TAM \(-> SVR3 Curses
.br
Conversion Library
.br
Design Document
.br
Issue 1.0
.AU "C. D. Francis" cdf SF 60545412 6106 A-118 "attunix!davef"
.AU "G. B. Smith" gbs SF 60545412 6649 G-211 "attunix!snort"
.AS 2
This document describes a method by which existing TAM programs will run on any
character terminal that is supported by \*(cu.
This method is completely SVID compatible and doesn't use the TAM library.
This document only addresses the issue of character mode applications and
will not address the problem of supporting
TAM graphics mode applications on character terminals.
.AE
.MT "INTERNAL MEMORANDUM"
.PF "''Issue 1.0''"
.H 1 "INTRODUCTION"
This document describes a method by which existing TAM programs will run on any
character terminal that is supported by \*(cu.
This method is completely SVID compatible and doesn't use the TAM library.
This document only addresses the issue of character mode applications and
will not address the problem of supporting
TAM graphics mode applications on character terminals.
.P
Compatibility will be achieved using header files and a set of library
routines that perform TAM functions with calls to \*(cu.
.P
Co-existence of TAM and \*(cu library calls within an application
program will not be achieved by
the conversion library.  TAM applications will continue to be limited
to the TAM interface defined for the UNIX PC.
.P
This effort is a part of the Extended Terminal Interface (ETI) project.
The objective of the ETI project is to provide a standard software interface
for screen management and text operations.\*F
In order to achieve compatiblity across the AT&T product line, 
ETI has choosen the \*(cu library as its base interface for character
terminals.  The primary purpose of the TAM Conversion Library is to 
extend the utility of otherwise non-portable UNIX PC applications by providing
a TAM library that is \*(cu compatible.
.FS
Extended Terminal Interface, High Level Requirements for Release 1.0.
N. M. Shah, April 18, 1986
.FE
.H 1 "MOTIVATION"
Currently, TAM programs only run on the UNIX PC and a limited number of
terminals.
TAM also uses a much older terminal information database (termcap) than the
current version of \*(cu.
This tool will provide a migration path for TAM programs to operate across
the AT&T product line.
.H 1 "CONVERSION"
Compatibility will be achieved by means of a header file, \fItam.h\fP, 
and a set of library
subroutines.
All TAM programs will have to be recompiled using the new \fItam.h\fP
header file and then
linked with the new subroutines and the \*(cu library.
The header file will cast all TAM routines directly to conversion library routines
and provide casts for TAM subroutine parameters.
For example, the TAM function \fIwcreate()\fP will be mapped to the conversion
library function \fBTAMwcreate()\fP.  \fBTAMwcreate()\fP will consist of
a series of \*(cu calls.
.H 2 "Assumptions"
The following assumptions are being made about TAM, the TAM user
community, and \*(cu, in the design of the migration
tool.
.H 3 "iswind()"
It must be assumed that the iswind() function will always return false.
This implies that the bitmap functions supported by TAM will not operate
with this conversion process.  This alleviates the need to convert hardware
dependent code.
.P
The user is responsible for the removal of the unused segments of code that
are not executed following the iswind() function call.
The expurgation is not required, but is recommended since it will
reduce the size of the code.
.P
.H 3 "Mouse Handling"
All calls to track(3T) will map to a call to wgetch().  This coincides with
the way track(3T) currently works on a character terminal.
.P
All calls to mouse management routines will translate to null operations.
This coincides with the results returned for TAM mouse management routines
on character terminals.
.H 3 "Keyboard Map Files"
Keyboard features described by TAM Keyboard Map files are fully subsumed by
new options of SVR3 \fIterminfo\fP and \fI\*(cu\fP.  Keymap files are
not necessary for a TAM conversion library based on SVR3 \fI\*(cu\fP.
.H 3 "UNIX PC Keyboard Escape Sequences"
The particular escape sequences emitted by non-ASCII UNIX PC keyboard keys
are of interest to the TAM community. Escape sequences for non-ASCII keyboard
keys will always be expressed to an applications program as their equivalent
UNIX PC keyboard escape sequences.
.H 3 "Virtual Key Codes"
The TAM community references virtual key codes by their symbolic names
defined in \fItam.h\fP rather than by their actual eight bit values.
It is acceptable to change the eight bit values of virtual key codes in \fItam.h\fP.
.H 1 "MONITIONS AND CONCERNS"
.H 2 "System Calls"
Systems calls specifically tailored to the UNIX PC would not be translated.
Calls such as ioctl(2), that have no corresponding meaning to \*(cu
will not be handled.  Such system calls are not part of the recommended
TAM interface and will fail under the TAM transition library on all machines
except the UNIX PC.
.H 2 "Curses and TAM Libraries"
Both the \*(cu and TAM libraries contain duplicate subroutine names and
it is for this reason that programs cannot contain a mixture of TAM and
\*(cu subroutines.
In other words, TAM programs that are being migrated cannot contain
calls to \*(cu subroutines other than those provided by TAM.
.H 2 "ANSI Standards - Keyboard Input"
To what extent are ANSI sequences needed in for keyboard input?
More specifically, do applications expect to see escape sequences or just
one character values for key depressions?
The TAM documentation recommends that all keyboard input be through
TAM virtualization functions.  The current thought is that the TAM
Conversion Library will allow strict ANSI or input virtualization keyboard
input.
.H 2 "ANSI Standards - Display Output"
To what extent must ANSI sequences be recognized for display output?
Recognizing a generic set of escape sequences is diametric to
\*(cu philosphy, yet the UNIX PC permits generic ANSI sequences on
a limited number of devices.  More concretely, on the UNIX PC
\fIecho "\\033[J"\fP will usually clear the screen.  The overhead
of yet another display output layer is tremendous while the gained
functionality supports a strictly non-portable coding technique.
The TAM Transition library will not recognize arbitrary ANSI escape 
for display output purposes.
.H 1 "DESIGN"
.P
The design of the TAM Conversion Library is broken into two fundamental
sections.  The first section, \fBTAM \(-> \*(cu Mapping\fP, covers
the problems of mapping TAM display functions to \*(cu display functions.
The second section, \fBKeyboard Management Subsystem\fP, covers TAM
keyboard input and other input related functions.
.H 2 "TAM \(-> \*(cu Mapping"
The following table gives a brief summary of the mapping of TAM to \*(cu
functions.  All references to \*(cu functions and data structures will be
in \fBbold type\fP and all references to TAM functions and data structures
will be in \fIitalic type\fP.
.TS
center tab(;) allbox;
lI lw(4i).
.sp
TAM Function;\fBCurses(3X) Equivalent\fP
.sp
winit();T{
Call \fBinitscr()\fP and also set up a static link list of window pointers.
Each of these pointers will maintain a forward and backward link to
other windows; information about the window's border; and the window's
prompt, user, label and slk text strings.
The global variable that indicates the current TAM window (\fIwncur\fP) is
declared and initialized in winit.  Finally, winit decrements
the value of LINES by the size of the softkey labels, prompt and
command lines at the bottom of the screen.
T}
wexit();Call \fBendwin()\fP and \fBexit().\fP
iswind();Return FALSE.
wcreate();T{
Call \fBnewwin()\fP and return a window index that looks like the TAM window
index.  The flag indicating that variable characters widths are being
used is to be ignored.  If a border around the window is indicated then
a subwindow is also created to contain the scrollable portion of the
window and a box is drawn in the parent window.
T}
wdelete();T{
Call \fBdelwin()\fP and remove the window from the list of windows.
Next, loop through all remaining windows and call \fBtouchwin()\fP and
\fBwnoutrefresh()\fP for each window.
Finally, call \fBdoupdate().\fP
T}
wselect();T{
Call \fBtouchwin()\fP and \fBwrefresh(),\fP then update the list of windows to
indicate the new ordering.
T}
wgetsel();T{
Return index to window that is on top, i.e., last window refreshed.
This window will always be the first window given in the list of
windows.
T}
wgetstat();T{
Return a copy of the \fIwstat\fP structure associated with the window.
T}
wsetstat();T{
This command can change the size and position of a window and whether
or not the window has a border. 
If the size of the window has changed then a new window must be created
(\fBnewwin()\fP), the contents of the old window must be copied to the new
one (\fBoverwrite()\fP), and the old window must be deleted (\fBdelwin()\fP).
.sp
If the border has been changed then a subwindow needs to be either
created or deleted.
.sp
Finally, this window needs to be selected and all the windows need to
be refreshed.
T}
wputc();Call \fBwaddch().\fP
wputs();Call \fBwaddstr().\fP
wprintf();Call\fBwprintw().\fP
wslk();T{
The array of character strings passed by \fIwslk()\fP are copied to the
structure associated with the window.
T}
wcmd();T{
The character string passed by \fIwcmd()\fP is copied to the
structure associated with the window.
T}
wprompt();T{
The character string passed by \fIwprompt()\fP is copied to the
structure associated with the window.
T}
wlabel();T{
The character string passed by \fIwlabel()\fP is copied to the
structure associated with the window.
T}
wrefresh();T{
Call \fBwrefresh().\fP 
If the window index is -1 then all windows are refreshed in the
appropriate order.
T}
wuser();T{
The character string passed by \fIwuser()\fP is copied to the
structure associated with the window.
T}
wgoto();Call \fBwmove().\fP
wgetpos();Call \fBgetyx().\fP
wgetc();T{
Call \fBwgetch().\fP
Character translation, from \*(cu to ANSI may be required depending
on the current keypad mode.
T}
kcodemap();T{
The ASCII representation of a virtual key is returned where the ASCII
representation is defined by the UNIX PC implementation of TAM.  This
is consistent with the current UNIX PC implementation for character
terminals.
T}
keypad();Call \fBkeypad().\fP
wsetmouse();T{
This is a null operation.
T}
wgetmouse();This is a null operation.
wreadmouse();This is a null operation.
wprexec();Call \fBerase()\fP and \fBrefresh();\fP
wpostwait();T{
Call \fBwrefresh()\fP for each window in the window list.
T}
wnl();T{
Toggle between calls to \fBnl()\fP and \fBnonl().\fP
T}
wicon();\fRThis is a null operation.
wicoff();This is a null operation.
track();Call \fBwgetch().\fP
initscr();Call \fBinitscr().\fP
nl();Call \fIwnl(wncur,1).\fP
nonl();Call \fIwnl(wncur,0).\fP
cbreak();Call \fBcbreak().\fP
nocbreak();Call \fBnocbreak().\fP
echo();Call \fBecho().\fP
noecho();Call \fBnoecho().\fP
insch();Call \fBinch().\fP
getch();Call \fBgetch().\fP
flushinp();Call \fBflushinp().\fP
attron();Call \fBattron().\fP
attroff();Call \fBattroff().\fP
savetty();Call \fBsavetty().\fP
resetty();Call \fBresetty().\fP
addch();Call \fIwputc(wncur,...).\fP
addstr();Call \fIwputs(wncur,...).\fP
beep();Call \fBbeep().\fP
clear();Call \fBclear().\fP
clearok();Call \fBclearok().\fP
clrtobot();Call \fBclrtobot().\fP
clrtoeol();Call \fBclrtoeol().\fP
delch();Call \fBdelch().\fP
deleteln();Call \fBdeleteln().\fP
erase();Call \fIclear().\fP
flash();Call \fIbeep().\fP
getyx();Call \fIwgetpos().\fP
insch();Call \fBinsch().\fP
insertln();Call \fBinsertln().\fP
leaveok();This is a null operation.
move();Call \fBwgoto().\fP
mvaddch();Call \fImove()\fP and \fIaddch().\fP
mvaddstr();Call \fImove()\fP and \fIaddstr().\fP
mvinch();Call \fImove()\fP and \fIinch().\fP
nodelay();Call \fBnodelay().\fP
refresh();Call \fIwrefresh(wncur).\fP
resetterm();Call \fBresetterm().\fP
baudrate();Call \fBbaudrate()\fP.
endwin();Call \fBendwin()\fP.
fixterm();Call \fBfixterm()\fP.
printw();Call \fIwprintw(wncur,...)\fP.
.TE
.P
For completeness, the TAM Conversion library includes the TAM message, 
menu, form, help, window, and paste buffer functions.
These "high level" functions are built from the functions in
the previous table.
.TS
center allbox tab(;);
cB s s s
lI lI lI lI.
TAM High Level Functions
message();menu();form();exhelp()
wind();pb_open();pb_check();pb_seek()
pb_empty();pb_name();pb_puts();pb_weof()
pb_gets();pb_gbuf();adf_gtwrd();adf_gtxcd()
adf_gttok()
.TE
.H 2 "Keyboard Management Subsystem"
.P
This section includes the design for the \fBKeyboard Management\fP subsystem
of the TAM Conversion library.
.H 3 "Fundamental Problems & Resolutions"
.P
This section addresses the following TAM keyboard input and input related functions.
.VL 20
.LI "wgetc()"
Read a character from the keyboard.
.LI "keypad()"
Activate or deactivate the translation of escape sequences into virtual
function key codes.
.LI "kcodemap()"
Return the UNIX PC keyboard escape sequence corresponding to a particular
virtual function key.
.LE
.P
Associated with the TAM keyboard input and input related functions are
four basic problems.  The problems and their associated resolutions are
listed below.
.BL
.LI
Keyboard Map Files.  TAM Keyboard Map files describe the mapping of function key
escape codes to UNIX PC virtual function codes.  This functionality is
fully subsumed by new options of SVR3 \fIterminfo\fP and \*(cu.
.LI
Function Key Virtualization.  TAM supports a set of virtual function keys
whose numerical values are different than the equivalent \*(cu virtual
function keys.  The conversion library will support the same virtual function
key names as UNIX PC TAM but their values will be changed to their SVR3
\*(cu equivalents.
.LI
Functionally Incompatible Keyboards.  Functionally incompatible keyboards
are keyboards that are missing keys that are present on the UNIX PC.
The functions  provided by absent keys will be supported by a series
of UNIX PC escape sequences.  This technique is similar to that provided
by TAM for character terminals on the UNIX PC.
.LI
Operationally Incompatible Keyboards.  Operationally incompatible keyboards
are keyboards that have one or more keys that emit escape sequences that are identical
to the UNIX PC keyboard sequences but do not match in terms of functionality.
The function of an operationally incompatible key will always map to its
\fIterminfo\fP specification.  The TAM specific function implied by the same
escape sequence will be accessible through the technique
for functionally incompatible keyboards.
Mechanisms in \*(cu automatically handle timing conflicts between actual
keyboard function keys and UNIX PC keyboard escape sequences.
.LE
.bp
.H 3 "Basic Model"
.P
The following diagram and subsequent notes briefly describe the 
\fBKeyboard Subsystem\fP of the TAM conversion library.
.DF
.de PS	\" start picture
.	\" $1 is height, $2 is width, both in inches
.if t .sp .3
.in (\\n(.lu-\\$2)/2u
.ne \\$1
..
.de PE	\" end of picture
.in
.if t .sp .6
..
.PS
Transition:	box height boxht*6 width boxwid*6
.ps +4
		"Keyboard Subsystem" ljust at Transition.nw + (linewid/4,-linewid/3)

.ps -4

CursesKey:	line <- left linewid*2 from 1/4 between Transition.nw and Transition.sw 

		box invis with .n at CursesKey.c "Virtual Key" "from wgetch()" 

SVR3Curses:	box with .e at CursesKey.l "SVR3" "Curses"

VtoAnsi:	box height boxht*2 with .se at Transition.se - (linewid,0) "Virtual" "To ANSI" "Mapper"
MagicEscape:	box height boxht*2 with .se at 3/4 between Transition.sw and Transition.s "Magic" "Escape" "Recognizer"

DecisionA:	circle radius circlerad/4 at CursesKey.r + (linewid/2,0)
DecisionB:	circle radius circlerad/4 at (Transition.s.x+linewid,DecisionA.r.y)

		line -> from CursesKey.r to DecisionA.w
AtoB:		line -> from DecisionA.e to DecisionB.w

AtoMagic0:	line from DecisionA.s to (DecisionA.s.x,MagicEscape.w.y+linewid/4)

AtoMagic1:	line <- from MagicEscape.w to (DecisionA.s.x+linewid/4,MagicEscape.w.y)
		arc from AtoMagic0.s to AtoMagic1.w


MagicToB0:	line from MagicEscape.e to (MagicEscape.e.x+linewid/2,MagicEscape.e.y)
MagicToB1:	line -> from MagicToB0.e + (linewid/4,linewid/4) to (MagicToB0.e.x+linewid/4,AtoB.c.y)
		arc from MagicToB0.e to MagicToB1.s

KpadOn0:	line from DecisionB.n to (DecisionB.n.x,Transition.n.y-linewid/2)
KpadOn1:	line from KpadOn0.n + (linewid/4,linewid/4) to (VtoAnsi.n.x+linewid,Transition.n.y-linewid/4)
KpadOn2:	line -> from (KpadOn1.e.x+linewid/4,KpadOn0.n.y) to (KpadOn1.e.x+linewid/4,Transition.e.y)
		arc from KpadOn1.w to KpadOn0.n
		arc from KpadOn2.n to KpadOn1.e

BtoAnsi0:	line from DecisionB.s to (DecisionB.s.x,VtoAnsi.w.y+linewid/4)
BtoAnsi1:	line -> from (DecisionB.s.x+linewid/4,VtoAnsi.w.y) to VtoAnsi.w
		arc from BtoAnsi0.s to BtoAnsi1.w

AnsiToExit0:	line from VtoAnsi.e to (KpadOn2.s.x-linewid/4,VtoAnsi.e.y)
AnsiToExit1:	line -> from (KpadOn2.s.x,VtoAnsi.e.y+linewid/4) to KpadOn2.s
		arc from AnsiToExit0.e to AnsiToExit1.s
Leave:		line -> right from KpadOn2.s

.ps -2
		"In Escape" ljust "Sequence" ljust at AtoMagic0.c + (linewid/6,0)
		box invis with .s at 1/2 between MagicToB1.n and AtoB.e "Virtual" "Key"
		"Keypad" ljust "Mode On" ljust at KpadOn0.c + (linewid/6,0)
		"Keypad" ljust "Mode Off" ljust at BtoAnsi0.c + (linewid/6,0)


		circle radius circlerad/3 at CursesKey.c + (0,circlerad/2) "A"
		circle radius circlerad/3 at AtoB.c + (0,circlerad/2) "B"
		circle radius circlerad/3 at AtoMagic1.c + (0,circlerad/2) "C"
		circle radius circlerad/3 at (1/2 between MagicToB1.n and AtoB.e) - (0,circlerad) "D"
		circle radius circlerad/3 at KpadOn1.c - (0,circlerad/2) "E"
		circle radius circlerad/3 at BtoAnsi1.c - (0,circlerad/2) "F"
		circle radius circlerad/3 at Leave.e + (circlerad/3,0) "G"

.ps +2
.PE
.DE
.sk 2
.VL 15
.LI "\D'c 1.5m'\h'-1m'\s-2A\s+2"
Input from the keyboard is always read using the \*(cu \fIwgetch()\fP
function.  Function key virtualization that is terminal dependent will be
handled by \*(cu while the
internals of the Keyboard Subsystem will manage terminal independent
function key virtualization.
.LI "\D'c 1.5m'\h'-1m'\s-2B\s+2"
At this point a keyboard character is either a terminal dependent virtual
function key or a normal ASCII character that is not part of an escape
sequence.  Most keys on a keyboard fall into this category.
.LI "\D'c 1.5m'\h'-1m'\s-2C\s+2"
Characters that reach this point are part of a UNIX PC keyboard escape sequence
or part of an escape sequence not supported by the UNIX PC.  In the first case
the UNIX PC keyboard escape sequence will be recognized and a virtual
function key code will be ejected from the \fBMagic Escape Recognizer\fP.
In the second case the escape sequence will pass through the 
\fBMagic Escape Recognizer\fP unchanged.
.LI "\D'c 1.5m'\h'-1m'\s-2D\s+2"
At this point each character is either an ASCII character, a UNIX PC keyboard
virtual function key character, or a virtual function key character not supported by
the UNIX PC.  A decision must be made to either transmit the character \fIas is\fP
to the application program or to translate the character to its corresponding
ANSI representation as defined for the UNIX PC.  Normal ASCII characters
map to themselves, UNIX PC virtual function key characters map to various escape
sequences, and other virtual function key characters map to themselves.
.LI "\D'c 1.5m'\h'-1m'\s-2E\s+2"
At this point all keystrokes are passed to the TAM application.  Note that
even virtual function keys not present on UNIX PC TAM will be given
to the application.  This is not a problem because UNIX PC TAM virtual
function keys are
superset of \*(cu virtual function keys.
.LI "\D'c 1.5m'\h'-1m'\s-2F\s+2"
At this point all UNIX PC TAM virtual function keys are mapped to their
equivalent ASCII escape sequence. All other characters are passed through
unchanged.
.LI "\D'c 1.5m'\h'-1m'\s-2G\s+2"
Finally, at this point characters are passed to the TAM application through
the \fIwgetc()\fP function call.
.LE
.P
The \fIkeypad()\fP function controls the direction of the branch after point
\u\D'c 1m'\h'-.75m'\v'.25n'\s-4G\s+4\v'-.25n'\d.  The \fIkcodemap()\fP function uses the
\fBVirtual To ANSI Mapper\fP to translate a TAM virtual function key
into its equivalent UNIX PC keyboard escape sequence.
.H 3 "Magic Escape Recognizer"
.P
The \fBMagic Escape Recognizer\fP will recognize and translate the following
UNIX PC keyboard specific escape sequences into their corresponding
virtual key values.  Escape sequences that do not have a virtual key
value are expressed in \fIitalics\fP.
.SK
.2C
.TS
tab(;);
lP-4 cP-4 s
lP-4 cP-4 s
lP-4 lP-4 lP-4.
\fBTAM Escape\fP;\fBVirtual Key\fP
\fBSequence\fP;;
.sp
;TAM;SVR3 Curses
.sp
ESC 0;End;KEY_END
ESC 9;Beg;KEY_BEG
ESC [ A;Up;KEY_UP
ESC [ B;Down;KEY_DOWN
ESC [ C;Forward;KEY_RIGHT
ESC [ D;Back;KEY_LEFT
ESC [ H;Home;KEY_HOME
ESC [ J;Clear;KEY_CLEAR
ESC [ S;RollDn;KEY_SF
ESC [ T;RollUp;KEY_SR
ESC [ U;Page;KEY_NPAGE
ESC [ V;s_Page;KEY_PPAGE
ESC N a;Rfrsh;KEY_REFRESH
ESC N B;s_Beg;KEY_SBEG
ESC N c;Move;KEY_MOVE
ESC N C;s_Move;KEY_SMOVE
ESC N d;Copy;KEY_COPY
ESC N D;s_Copy;KEY_SCOPY
ESC N e;Dlete;KEY_DL
ESC N E;s_Dlete;KEY_SDL
ESC N f;DleteChar;KEY_DC
ESC N F;s_DleteChar;KEY_SDC
ESC N g;Prev;KEY_PREVIOUS
ESC N G;s_Prev;KEY_SPREVIOUS
ESC N h;Next;KEY_NEXT
ESC N H;s_Next;KEY_SNEXT
ESC N i;Mark;KEY_MARK
ESC N I;Slect;KEY_SELECT
ESC N j;InputMode;KEY_IC
ESC N J;s_InputMode;KEY_SIC
ESC N K;s_Back;KEY_SLEFT
ESC N L;s_Forward;KEY_SRIGHT
ESC N M;s_Home;KEY_SHOME
ESC N N;s_End;KEY_SEND
ESC O a;ClearLine;KEY_EOL
ESC O A;s_ClearLine;KEY_SEOL
ESC O b;Rstrt;KEY_RESTART
ESC O B;Ref;KEY_REFRESH
ESC O c;F1;KEY_F(0)
ESC O C;s_F1;KEY_F(8)
ESC O d;F2;KEY_F(1)
ESC O D;s_F2;KEY_F(9)
ESC O e;F3;KEY_F(2)
ESC O E;s_F3;KEY_F(10)
ESC O f;F4;KEY_F(3)
ESC O F;s_F4;KEY_F(11)
ESC O g;F5;KEY_F(4)
ESC O G;s_F5;KEY_F(12)
ESC O h;F6;KEY_F(5)
ESC O H;s_F6;KEY_F(13)
ESC O i;F7;KEY_F(6)
ESC O I;s_F7;KEY_F(14)
ESC O j;F8;KEY_F(7)
ESC O J;s_F8;KEY_F(15)
ESC O k;Exit;KEY_EXIT
ESC O K;s_Exit;KEY_SEXIT
ESC O l;Msg;KEY_MESSAGE
ESC O L;s_Msg;KEY_SMESSAGE
ESC O m;Help;KEY_HELP
ESC O M;s_Help;KEY_SHELP
ESC O n;Creat;KEY_CREATE
ESC O N;s_Creat;KEY_SCREATE
ESC O o;Save;KEY_SAVE
ESC O O;s_Save;KEY_SSAVE
ESC O p;Suspd;KEY_SUSPEND
ESC O P;s_Suspd;KEY_SSUSPEND
ESC O q;Rsume;KEY_RESUME
ESC O Q;s_Rsume;KEY_SRSUME
ESC O r;Opts;KEY_OPTIONS
ESC O R;s_Opts;KEY_SOPTIONS
ESC O s;Undo;KEY_UNDO
ESC O S;s_Undo;KEY_SUNDO
ESC O t;Redo;KEY_REDO
ESC O T;s_Redo;KEY_SREDO
ESC O u;Cmd;KEY_COMMAND
ESC O U;s_Cmd;KEY_SCOMMAND
ESC O v;Open;KEY_OPEN
ESC O V;Close;KEY_CLOSE
ESC O w;Cancl;KEY_CANCEL
ESC O W;s_Cancl;KEY_SCANCEL
ESC O x;Find;KEY_FIND
ESC O X;s_Find;KEY_SFIND
ESC O y;Rplac;KEY_REPLACE
ESC O Y;s_Rplac;KEY_SREPLACE
ESC O z;Print;KEY_PRINT
ESC O Z;s_Print;KEY_SPRINT
ESC P a;PF1;KEY_F(16)
ESC P b;PF2;KEY_F(17)
ESC P c;PF3;KEY_F(18)
ESC P d;PF4;KEY_F(19)
ESC P e;PF5;KEY_F(20)
ESC P f;PF6;KEY_F(21)
ESC P g;PF7;KEY_F(22)
ESC P h;PF8;KEY_F(23)
ESC P i;PF9;KEY_F(24)
ESC P j;PF10;KEY_F(25)
none\(dg;PF11;KEY_F(26)
none\(dd;PF12;KEY_F(27)
.TE
.1C
.FS "\(dg"
The PF11 virtual key code is generated by the <control _> sequence on the
Unix PC keyboard.  There is no corresponding escape sequence for PF11.
.FE
.FS "\(dd"
The PF12 virtual key code is generated by the <control => sequence on the
Unix PC keyboard.  There is no corresponding escape sequence for PF12.
.FE
.H 3 "Virtual To ANSI Mapper"
.P
The \fBVirtual To ANSI Mapper\fP will map any UNIX PC specific virtual keyboard
character into its corresponding UNIX PC keyboard escape sequence.  The
set of virtual keyboard characters recognized is listed in the section
\fBMagic Escape Recognizer\fP.

.SG
.NS
.sp
Department 60545412
.sp
C. Arndt
O. Bloom
H. Chiesi
C. Cirillo
M. Clitherow
M. DeFazio
D. Kretsch
A. Hall
T. Hansen
D. McGovern
D. Mongeau
L. Ozimek
J. Peterson
E. Rice
H. Shottland
.sp
UNIX SE & PM at SF:
.sp
J. Baccash
A. Barrese
D. Ryan
.sp
UNIX PC SE/Dev. at FJ:
.sp
B. Courte
W. Gregory
R. Tran
B. Weiss
.sp
3B SE at IW:
.sp
J. M. Grinn
P. Walsh
.sp
Area 59:
.sp
D. Belanger
R. Reeves
.sp
OTS:
.sp
S. Chappell
R. Cohen
S. Jarowski
M. Manks
.sp
Bell Labs Training Center:
.sp
N. Ippolito
.NE
.TC
