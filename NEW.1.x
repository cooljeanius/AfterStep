1.8.11
Added DefaultGeometry option to database. That should allow to place 
window with specifyed geometry when no USPosition was requested, 
instead of dragging it around. Usefull for things like Netscape.
Example: 
  Style "*escape*" DefaultGeometry 800x600+10-100


1.8.10
Xinerama support and Multiple configs on multi-head setup support.
There could be separate InitFunction, RestartFunction, Feel, Look,
and background for each screen. See man page for afterstep.

1.7.164 patch 3

$HOME/GNUstep/Library/AfterStep/start/Desktop/Theme/.include
changed from shell script call to perl script call

$HOME/GNUstep/Library/AfterStep/form
changed from shell script call to perl script call

1.7.142 patch 16

New .include option:

extension <.ext>       - strips a "ext." prefix or ".ext" suffix from 
                         included filenames

1.7.142 patch 2

New Wharf option:

Transient              - prevents a button from performing any action, 
                         being pushable, and having a balloon

1.7.126 patch 14

The ButtonPixmap, IconFont, ButtonTextureType, ButtonMaxColors, 
ButtonBgColor, and ButtonTextureColor look options are now deprecated.  
They have been replaced by several MyStyles:

ButtonPixmap           - background tile of iconified windows
ButtonTitleFocus       - title of focused iconified windows
ButtonTitleSticky      - title of sticky iconified windows
ButtonTitleUnfocus     - title of non-sticky, unfocused iconified windows

The following are automagical inheritances from old options:

FWindowStyle       => ButtonPixmap
ButtonBgColor      => ButtonPixmap.BackColor
ButtonTextureColor => ButtonPixmap.BackGradient
ButtonPixmap       => ButtonPixmap.BackPixmap
ButtonTextureType  => ButtonPixmap.texture_type
ButtonMaxColors    => ButtonPixmap.MaxColors

FWindowStyle       => ButtonTitleFocus
SWindowStyle       => ButtonTitleSticky
UWindowStyle       => ButtonTitleUnfocus

SeparateButtonTitle <flag>
                    - replaces the --enable-iconbackground configure 
                      option; specifies whether the icon title should 
                      be drawn at the top of the icon, or in a 
                      separate window at the bottom (defaults to 1)

ButtonSize <width> <height>
                    - specifies the size of iconified windows exactly; 
                      this includes titles drawn at the top, but not 
                      the separate window created by SeparateIconTitle

1.7.126 patch 11

New MyStyle option:

  BackPixmap 130 <xpm> - sets background to average of xpm and the 
                         root pixmap

  This is currently only available to AfterStep!

1.7.126 patch 09

New MyStyle option:

  BackMultiGradient <type> <color1> <offset1> ... <colorN> <offsetN>
                       - draws a multi-point gradient

  Here's an example:
  BackMultiGradient 8 black 0.0 red 0.5 green 1.0

  ... will result in a gradient like the following (b=black, r=red, g=green):

  +--------------+
  |bbbbbbbbbbbbbb|
  |bbrbbrbbrbbrbb|
  |brbrbrbrbrbrbr|
  |rrrrrrrrrrrrrr|
  |rgrgrgrgrgrgrg|
  |rrgrrgrrgrrgrr|
  |gggggggggggggg|
  +--------------+

1.7.111 patch 10
background loading moved into asetroot from Pager check man pages for more 
info.

1.7.111 patch 07
Completely revamped asetroot config. See sample config file for details.

1.7.90 patch 16

New Wharf option:
  WithdrawStyle        - control what Wharf buttons are watched for mouse 
                         button 3; this option replaces the (mostly 
                         undocumented) NoWithdraw option
                         0 == never
                         1 == any toplevel button
                         2 == the top/bottom button (or left/right, for 
                              horizontal Wharfs) 

1.7.90 patch 11

New ascommand.pl options:
  --file <file>        - read commands from file and send them to AfterStep
  --interactive        - start interactive session with AfterStep; prompts 
                         user for input, and displays AfterStep's output

1.7.90 patch 03

New feel function:
  ToggleLayer <layer1> <layer2> - toggle a window between two layers

1.7.90 patch 02

Balloons displaying the function(s) bound to titlebar buttons can now be 
displayed, with the usual keywords:
  TitleButtonBalloons                   - turn on balloons
  TitleButtonBalloonBorderColor <color> - set border color
  TitleButtonBalloonBorderWidth <width> - set border width
  TitleButtonBalloonDelay <msec>        - set delay in milliseconds

Balloon code now uses MyStyle's for text/background config.  Style names 
are:
  TitleButtonBalloon   - AfterStep window frame titlebar buttons
  *PagerBalloon        - Pager balloons
  *WharfBalloon        - Wharf balloons
  *WinListBalloon      - WinList balloons

NOTE: Texture type != 0 (ie, gradients, pixmaps, etc) does not work.

The following keywords NO LONGER EXIST:
  BalloonFore
  BalloonBack
  BalloonFont
  *WharfBalloonFore
  *WharfBalloonBack
  *WharfBalloonFont
  *WinListBalloonFore
  *WinListBalloonBack
  *WinListBalloonFont

--

1.7.75 patch 03

Changed *WinListLeftJustify to *WinListJustify <alighment>

1.7.75 patch 01 

Changed StickyPagerIcon to *PagerStickyIcons - it is now exec name specific
Changed *PagerRedrawBg <number> to *PagerDontDrawBg as making more sense.

--

1.7.49 patch 17

New database option: 
  Layer <layer>        - a replacement for StaysOnTop, StaysOnBack; see 
                         the afterstep man page for more details

1.7.49 patch 14

DeskBorderColor added to PagerDecoration option of pager config.
MyStyle support added for pager's config file and steprc in Pager.

1.7.49 patch 13

Pager now understands MyStyle options. The following styles should be 
defined :
   "*PagerUWindowStyle"(defaults to "unfocused_window_style") - unfocused 
   window representation's style.
   "*PagerFWindowStyle"(defaults to "focused_window_style") - focused 
   window representation's style.
   "*PagerSWindowStyle"(defaults to "sticky_window_style") - sticky 
   window representation's style.
   "*PagerActiveDesk" - active (selected) desk's style. BackPixmap 129 is
   supported for transparency.
   "*PagerInActiveDesk" - inactive desk's style. BackPixmap 129 is
   supported for transparency.

1.7.49 patch 06

Module communication has been rewritten, and modules can now be started 
separately from AfterStep.  There is an example of a module (written in 
perl) in tools/ascommand.pl.

1.7.47 patch 02

New Style options:
  FocusStyle [style]   - set focus style for the window
  StickyStyle [style]  - set sticky style for the window
  UnfocusStyle [style] - set unfocused style for the window

1.7.37 patch 07

New MyStyle option:
  DrawTextBackground: clear area behind text to BackColor

1.7.37 patch 02

New Wharf option:
  NoWithdraw: don't grab button 3 from the top wharf window

New configure option:
  --enable-audit: enables a memory usage audit; this is intended for 
                  developers only, but anyone can use it :)

1.7.25 patch 10

New database options:
  VerticalTitle:   draw window titlebar down the left side of the window
  HorizontalTitle: draw window titlebar across the top of the window

1.7.25 patch 04
added Pager pseudotransparency option :
*PagerTransparent [<desk#>] <color>

1.7.0 patch 25

Wharf now uses a MyStyle ("*WharfTile") for its button tile backgrounds.  
Mode 129 (transparency/tinting) now works, as well.

1.7.0 patch 22

Added new .include option:
  Name       - set the name of this menu

NOTE: root menu should now be called PopUp "0" in feel files, not 
PopUp "start".

1.7.0 patch 21

BackPixmap mode 129 can now be used with window titlebars.  It can also 
be used to tint, as well as basic transparency.

1.7.0 patch 19

added following Pager option :

*PagerSetRootOnStartup 
to make it show 
root background on startup - before notification from AfterStep
about current desk. This is needed if aterm is started from autoexec
or even before AfterStep with transparent option.

1.7.0 patch 14

New WinList option:
  AutoHide - turn on or off WinList hiding

1.7.0 patch 10

added following Pager options :
PagerDecoration option - possible values :
   DeskBorderWidth #  - border width between desks
   NoPageSeparator    - removes lines separating one page from another
   NoSelection        - removes frame surrounding selected page 
   GridColor <color>  - defines color of page separating grid and desk 
                        borders
 Usage : 
 *PagerDecoration  DeskBorderWidth 1,NoPageSeparator,NoSelection,GridColor green
 ( you can use any combination of options )

*PagerHilightBorders
Tells the pager to hilight the border of focused window as well as its
label

1.7.0 patch 9

Added balloons (with all accompanying options) to WinList.  Use:

*WinListBalloons
*WinListBalloonDelay
... etc

1.7.0 patch 7

New database option:
  AvoidCover - this window will not be covered by any other window
  AllowCover - turn off AvoidCover

1.7.0 patch 5

New config files, ".include" files, may be placed in the start/ menu tree.
.include files have the following options:
  Command    - sets an AfterStep function to apply to all items
  Include    - include another directory's contents in this menu
  KeepName   - keep the directory name so it can be referenced from a feel
  MiniPixmap - set the mini-pixmap to use for this menu
  Order      - set the order of this submenu in its parent menu

NOTE:
WARNING:
PAY ATTENTION:
The Look, Feel, and Pictures dir are no longer special, and will not 
be automagically filled in.  Take a look at the .include file for these 
dirs in the standard distro for examples.

1.7.0 patch 3

new afterstep look option:
  RubberBand - allows selection of move/resize rubberband look
    0 - normal
         ________
        |  |  |  |
        |__|__|__|
        |  |  |  |
        |__|__|__|
        |  |  |  |
        |__|__|__|

    1 - single rectangle
         ________
        |        |
        |        |
        |        |
        |        |
        |        |
        |________|

    2 - crossed-out single rectangle
         ______
        |\    /|
        | \  / |
        |  \/  |
        |  /\  |
        | /  \ |
        |/____\|

new WinList options:
  WinListPixmap - background pixmap for WinList
  TruncateLeft  - display the left part of the title
  TextStyle     - 3d text, 1 and 2 for 3d text, 0 for normal
  GradType      - gradients for WinList
    0 - normal
    1 - relief
    2 - vgradient
    3 - hgradient
  GradFrom      - starting colour for gradient
  GradTo        - ending colour for gradient

1.7.0 patch 1

StaysOnBack is no longer ignored, and now works.

-------------------------------------------------------------------------------

1.5beta5 patch21

added:
ViewportX x, ViewportY y
styles to database - allowing to do flexible StartOnPage
stuff. It will move screen to the specifyed pos on desk, before placing 
the window.

1.5beta5 patch16

configure options --with-gnustep-lib and --with-afterdir where changed
so now  you need to supply patch within gnustep dir and within gnusteplib
dir respectevely ( like --with-gnustep-lib=Library
--with-afterdir=AfterStep )

1.5beta5 patch14

Added option *PagerFastStartup to pager configuration.
This will force Pager to inform Afterstep that it is started, right 
after the first desk's background is loaded. Pager will also set the
global Atom to background Pixmap, so that Eterm can work nicely, with 
transparency emulation.
This option should speedup AS startup, when different pictures are used
for different desks.

1.5beta5 patch13 [17]

added configure option --enable-staticlibs=yes/no. Default is "yes".
This will force AS to be built with libafterstep, libasimage, libWidgets
statically linked. By default all libs are linked statically.

1.5beta5 patch10

added configure option --enable-png=yes/no. Default is "yes".
added Pager config option PagerHideInactiveLabels. See man pages.
added Pager config option PagerLabelBelowDesk. See man pages.

1.5beta4 patch07

added configure option --enable-pagerbackground=yes/no to enable/disable
all background support in Pager. Default is 'yes' (background support
enabled)

version 1.5beta4 - current
New Pager options :
defines XPM or JPEG image to be displayed in Pager window for
specified desk.
  *PagerImage  <desk#> <image_filename>

Defines image to be displayed in Root for specified desk. XPM, JPEG, or
PNG images supported internally( unless PagerXImageLoaderArgs are
specified ), other formats uses external image loader, like xli, or xv.
  *PagerDesktopImage  <desk#> <image_filename>

Defines arguments to be passed to external image loader, when showing Root
background. 
!!!!!!! This Option will disable inernal support for XPM and JPEG !!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
if desk# is ommited args will have effect for all desks, but will not
disable internal JPEG and XPM support
  *PagerXImageLoaderArgs  [<desk#>] <image loader args without prog name>

When image is dislayed on root using external app this option will let
you cache image in memory, to not invoke this app every time you switch
desks, saving lots of time. This feature will not work with some user
application, specificaly KDE desktop it will also restore all icons when
you switch desks first time. Use it with care and at your own risk
  *PagerFastBackground <desk#> <yes/no> [<tile_height> [<tile_width>]]

This option will let you reduce the number of colors used for background
image, when loading JPEG or XPM internally. Implementation of this depends
on image library, therefore you not always get what you want :)
  MaxBackgroundColors <number_of_colors>

version 1.5pre6 - current
  Swallow:    (same as 1.5pre4 MaxSwallow)
  MaxSwallow: if swallowed window is larger than button size (usually 64x64),
              the button is resized to fit the swallowed window

The Size keyword specifies the size of a wharf button
<width> and <height> are non-negative integers
  *Wharf label pixmap Size <width> <height>

On the subject of button sizes, here's a brief description of how Wharf 
chooses the size of a button, in decreasing order of precedence:
  1. if the Size keyword is specified, use that
  2. if MaxSwallow is used, use the size of the swallowed window
  3. if WharfPixmap is given, use the size of the pixmap
  4. use 64x64

If swallowed Module is larger than button size (usually 64x64),
the button is resized to fit the swallowed Module [ Currently only
Pager is a feasible Swallowable Module :)]
  MaxSwallowModule

In case you haven't noticed, Wharf now supports Folders within Folders.

-------------------------------------------------------------------------------

zharf:
next line turns on balloons
  *ZharfBalloons

color and font spec are standard
  *ZharfBalloonFore               <color>
  *ZharfBalloonBack               <color>
  *ZharfBalloonFont               <font>
  *ZharfBalloonBorderColor        <color>

width is in pixels, and is a non-negative integer
  *ZharfBalloonBorderWidth        <width>

offset is in pixels, and is an integer
(offset < 0)  => balloon will be above client window (ie, button)
(offset >= 0) => balloon will be below client window (ie, button)
  *ZharfBalloonYOffset            <offset>

delay is in milliseconds, and can be any non-negative integer
  *ZharfBalloonDelay              <milliseconds>

version 1.4.5.55N2 - version 1.5pre4
  Swallow:    (same as 1.0)
  MaxSwallow: if swallowed window is larger than button size (usually 64x64) 
              it is resized to the button size

#-------------------------------------------------------------------
fixed options
u can enable it with --enable-fixeditems
u must have @after_share@/fixed directory and it can contain:
   feels/       - merge feels in this dir with user's local ones
   looks/       - dtto.
   backgrounds/ - dtto.
   start/       - start menu entries.

 It all works all okay with one exception - when one's using
 ordered directories (ie. 2_Applications) and in /u/s/a/f/s/ is
 Applications for example (unordered directory), those two won't get
 merged. 

#-------------------------------------------------------------------
start tree (startmenu)
Directories and/or files can now be prefixed with a <number>_, which
explicitly states their order in the menu. AS an example
~/G/L/A/s/0_My-Z-Directory
~/G/L/A/s/1_My-z-File
~/G/L/A/s/2_My-A-Directory
~/G/L/A/s/3_My-R-File
etc...
order.
Entry example:
  Exec "Name to be in startmenu" exec application &
  MiniPixmap "some-mini.xpm"
a <#>_nop file:
1) with - Nop " " - will produce a space at # point in the vertical list
2) with - Nop ""  - will produce a horizontal line at # poin in list


-------------------------------------------------------------------
feel:

switching among windows
  AutoReverse 0:    default switching among the windows (circulating in
		    one direction)
  AutoReverse 1:    #1->#2->#3->#4 and then #4->#3->#2->#1
  AutoReverse 2:    #1->#2->#3->#4 and then #4->#1->#3->#2 etc. (as in
		    wind*ze 9x)
  AutoReverse		<number>

window switching through desks?
  AutoTabThroughDesks

Support Motif window manager function hints
  MWMFunctionHints

Support Motif window manager decoration hints
  MWMDecorHints

Allow overriding of Motif function hints
 MWMHintOverride

-------------------------------------------------------------------
look:
TitleButtonStyle	0	- make a space at sides of the title bar
TitleButtonStyle	1	- don't make a space at sides of the title
				  bar  (like in WindowMaker)

define the number of pixels among titlebar buttons
  TitleButtonSpacing	<number>

Do you want minipixmaps in root menu ?
[This was previously in the feel.file]
  MenuMiniPixmaps

Sort Start menu entries by ... 1=alphabetical order, 0=date (default)
[This was previously in the feel.file]
  StartMenuSortMode		<number>

0 Draws no borders;
1 Draws borders around each menu_item; and
2 Draws border around both menu_title and main menu body
  DrawMenuBorders			<number>

-------------------------------------------------------------------
pager:
next line turns on balloons
  Balloons
color and font spec are standard
  BalloonFore			<color>
  BalloonBack			<color>
  BalloonFont			<font>
  BalloonBorderColor		<color>
width is in pixels, and is a non-negative integer
  BalloonBorderWidth		<width>
offset is in pixels, and is an integer
(offset < 0)  => balloon will be above client window (ie, button)
(offset >= 0) => balloon will be below client window (ie, button)
  BalloonYOffset            	<offset>
delay is in milliseconds, and can be any non-negative integer
  BalloonDelay			<milliseconds>

-------------------------------------------------------------------
wharf:
next line turns on balloons
  *WharfBalloons
color and font spec are standard
  *WharfBalloonFore               <color>
  *WharfBalloonBack               <color>
  *WharfBalloonFont               <font>
  *WharfBalloonBorderColor        <color>
width is in pixels, and is a non-negative integer
  *WharfBalloonBorderWidth        <width>
offset is in pixels, and is an integer
(offset < 0)  => balloon will be above client window (ie, button)
(offset >= 0) => balloon will be below client window (ie, button)
  *WharfBalloonYOffset            <offset>
delay is in milliseconds, and can be any non-negative integer
  *WharfBalloonDelay              <milliseconds>

steps can be any non-negative integer
  *WharfAnimateSteps      <steps>
  *WharfAnimateStepsMain  <steps>

delay is in microseconds, and can be any non-negative integer
  *WharfAnimateDelay      <delay>

Swallow vs. MaxSwallow:
version 1.0 - version 1.4.5.55N
  Swallow:    assume swallowed window is 48x48, and place it accordingly
  MaxSwallow: if swallowed window is larger than 55x57, it is resized to
	      55x57

