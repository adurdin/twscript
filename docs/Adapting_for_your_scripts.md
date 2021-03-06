Important Notice
================

This document will only be of interest to people who want to recompile twscript,
or use this package as the base from which they can develop their own scripts.

If all you want to do is *use* twscript, please see the TWScript.md document,
you can safely ignore this one!


Compiling or using this package as the base for your scripts
============================================================

If you are interested in using this package as the base from which you can
develop your own scripts, or just want to bugfix and recompile twscript,
you will need to do rather more work than a normal Dromed user. You will
need to have at least the following installed:

1. a working compiler setup (and editor to code in)
2. git (the distributed version control system) to fetch the library code.
2. the supporting libraries
3. the twscript code.

what follows is a walkthrough of how to get the above set up, and how to get
to the point where you can actually compile scripts.

1. Setting up the compiler
--------------------------

Before you can do any compiling, you need a compiler. This is actually not as
straightforward as you might think: as even if you already have a compiler
available, there's a good chance it may not actually work with this code, or it
may produce an .osm that Thief can't actually load (or it may work fine, in
which case, great! But don't blame me if you end up up to your armpits in
soul-devouring, be-tentacled horrors from beyond time and space when trying it).
Hence, you are strongly advised to follow these steps, even if you already
have a working compiler setup, unless you're feeling adventurous:

0. Download the TDM-GCC compiler suite for Windows from the [project site](http://tdm-gcc.tdragon.net/).
   You want to get the `tdm-gcc-x.x.x` bundle installer (MinGW/sjlj), currently
   at version 4.6.1 available from [this link](http://sourceforge.net/projects/tdm-gcc/files/TDM-GCC%20Installer/tdm-gcc-4.6.1.exe/download)
   (note that I have not tried using the tdm64-gcc installer, and I doubt the
   generated .osm would work. Feel free to experiment!)
1. Run the `tdm-gcc-x.x.x` installer:
    - Click "Create"
    - Select "MinGW/TDM (32-bit), click "Next"
    - Set the installation directory, click "Next"
    - Unless you have some preferred sourceforge mirror, click "Next" again.
    - Review the selected components, you shouldn't need to change any. Click
      "Install".
    - Go look at lolcats or something for a bit.
2. From the Start Menu, select MinGW32 -> MinGW Command Prompt
3. When the command prompt appears, type the following: `gcc -v`. If all goes
   well, you should see several lines of information output, ending in
   `gcc version X.X.X (tdm-1)`.

This is enough to get you a working compiler, but the toolset included is very
limited, I generally suggest installing MSYS as well:

0. Download the latest version of the `mingw-get-inst` installer from
   [sourceforge](http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst/)
1. Run the installer
2. Click next until the installer reaches the "Select Destination Location"
   step, set the location to the path you installed tdm-gcc to (for example,
   `C:\MinGW-TDM`)
3. On the Select Components step **YOU MUST UNTICK "C Compiler"**. *Do not*
   attempt to install the standard MinGW compiler here, or you will encounter
   all sorts of problems (remember those be-tentacled horrors? Yes, those sorts
   of problems). Scroll down the list of components, find "MSYS Basic System"
   and tick it. You must ensure that "MSYS Basic System" **is the only enabled
   component**, then click "Next".
4. Go look at more lolcats while it installs.
5. From the start menu, select MinGW -> MinGW Shell (yes, this is different
   from above).
6. At the command prompt, type `gcc -v` again. You should get the same output
   as before.

At this point, you should have a working compiler, and a decent set of tools
installed.

2. Setting up an editor
-----------------------

I'm not going to tell you to use one editor or another here - editor choice has
been the subject of Holy Wars for decades, and almost everyone has their
favorite. If you don't have a decent programmer's editor installed, you can
go far worse than to start off with [Notepad++](http://notepad-plus-plus.org/):
it's fast, light, does syntax highlighting and block folding. Apparently
TDM-GCC plays nice with [Code::Blocks](http://www.codeblocks.org/),
so you might want to look at that. I tend to use Emacs via
[Cygwin](http://www.cygwin.com/), but I'm also reliably informed that I'm
insane, so YMMV with that.

3. Setting up git
-----------------

To actually get the code, you need to install git. If you already have git
(or github's native tool) installed, you can skip this step.

0. Close any MinGW windows you may have opened when setting up the compiler.
1. Download the latest installer from http://git-scm.com/downloads
2. Run the installer, clicking "Next" through it until you come to the
   "Adjusting your PATH environment" page. Select the "Run Git from the
   Windows Command Prompt" option, and click "Next"
3. Leave the "Choosing the SSH executable" option on "Use OpenSSH".
   **Do not use PuTTY**, even if you have it installed and working - it
   can introduce problems.
4. Keep clicking "Next" until it starts installing.
5. lolcats time again.
7. Once the install has finished, run Git -> Git Bash from the Start Menu.
   If all goes well, the Git Bash window should open, and you can use
   `git --version` to confirm that git is working.

Before continuing, you may want to become familiar with the basics of using
Git. A good place to start is the online [Pro Git Book](http://git-scm.com/book).

4. Getting the code
-------------------

Now you should be in the position where you can actually start to fetch code
to work on. All the code is on github, so you need to clone copies of it
with git (you may wish to fork the twscript project before doing so, if you
want to push your changes back to github, but that's up to you).

0. Create a directory somewhere on your system, something like `D:\thiefscripts`
   It doesn't matter what you call it, provided that you can remember it, and it
   **does not** contain spaces. Spaces in filenames are another way to summon
   those soul-devouring, be-tentacled horrors, so avoid them.
1. From the start menu, select Git -> Git Bash
2. Change into the directory you created in step 0, using unix-like syntax, eg:
   `cd /d/thiefscripts`
3. Enter the following commands to clone Telliamed's script support libraries:

        git clone https://github.com/whoopdedo/lg.git
        git clone https://github.com/whoopdedo/scriptlib.git

4. And this one to clone this project (if you have forked it, clone your
   fork instead!):

        git clone https://github.com/TheWatcher/twscript.git

5. You'll probably also want to grab Telliamed's Public Scripts package itself,
   so that you can use the various script implementations it contains as a
   reference. You can either look at it [on the web](https://github.com/whoopdedo/publicscripts)
   at github, or clone a local copy:

        git clone https://github.com/whoopdedo/publicscripts.git

You should now have four or five directories inside the one you created in
step 0 above: lg, dh2, scriptlib, twscript, and possibly publicscripts. Now
to compile them...

5. Compiling the libraries
--------------------------

Before you can actually create any scripts, you need to have compiled the
support libraries your scripts will need to use in order to communicate
with the Dark engine. This step should be pretty straightforward at this
point. Henceforth, for simplicity, I am going to assume that all the projects
you cloned in section 4 are inside the `D:\thiefscripts` directory as
discussed there - if you cloned them elsewhere, you must update the paths
accordingly.

First you need to fix up a few issues with the standard libraries as cloned:

0. Optional step: open `D:/thiefscripts/lg/lg/config.h` in your text editor,
   go to line 24 and add `#undef __thiscall` to that line, eg:

        #else // !_MSC_VER
        #undef __thiscall
        #define __thiscall

   and save the changes. If you don't do this, you'll get a redefinition
   warning from almost every compile, and it gets irritating quickly.
1. Open `D:/thiefscripts/lg/lg/types.h` in your text editor, go to line
   195 and change `static const float epsilon = 0.00001f` to read
   `static constexpr float epsilon = 0.00001f`  Save the changes.
2. Open `D:/thiefscripts/lg/Makefile` in your text editor, go to line 24,
   and change

        `CXXFLAGS = -W -Wall -Wno-unused-parameter -masm=intel $(INCLUDEDIRS)`

   to be

        `CXXFLAGS = -std=gnu++0x -W -Wall -Wno-unused-parameter -masm=intel $(INCLUDEDIRS)`

   Save the changes.
3. Open `D:/thiefscripts/scriptlib/Makefile` in your text editor, go to
   line 45, and change

        `CXXFLAGS = -W -Wall -masm=intel`

   to read

        `CXXFLAGS = -std=gnu++0x -W -Wall -masm=intel`

   Save the changes.

Now you can start compiling:

0. Start a MinGW Shell window
1. Change into the lg directory, eg: `cd /d/thiefscripts/lg`
2. Run `make`
3. Change into the scriptlib directory, eg: `cd ../scriptlib`
4. Run `make`

6. Test compiling TWScript
--------------------------

At this point, all the libraries required to compile scripts should be in place
and compiled, so it is time to actually give it a go. Before you start working
on any scripts of your own, you should make sure your environment is sane by
compiling the TWScript package:

0. If a MinGW Shell window isn't current open, start one.
1. Change into the twscript directory: `cd /d/thiefscripts/twscript`
2. Run `make`
3. Once make has finished, copy twscript.osm into your Thief 2 installation.
4. Start Dromed, add an object, set it up to use TWTweqSmooth, go into game
   mode, and see if it works. If it does, you have successfully compiled the
   osm!

7. Writing your own scripts
---------------------------

If you get here without problems, you are ready to start making your own
scripts (and your problems are literally just beginning...).

Updated instructions here soon.

8. Important note about member variables
----------------------------------------

When writing scripts for the dark engine you should be very careful when using
'non persistent' member variables to maintain cross-message state (that is,
storing state that persists between messages in normal member variables rather
than using the persistent script data store via `set_script_data()`,
`get_script_data()`, etc). Every time a metaproperty is added to an object, the
list of scripts attached to the object is replaced: already attached script
instances are detached and destroyed, and new copies are added in their place.
This means that, every time a metaprop is added, you will lose any state held
in member variables.

Either avoid using member variables to maintain state and use the persistent
script data store to do it, or ensure that your scripts are able to restore
their state through other means.
