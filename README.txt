====[ README.OS2 ]======================================================

Written by Tommi Nieminen 26-Aug-1994
Internet address: Tommi.Nieminen@uta.fi  or  sktoni@kielo.uta.fi
(at least until spring 1995)

This package, `ed.zip', is a combination of `ed-x.zoo' and `ed-s.lzh'
that I found at the anonymous FTP place ftp.funet.fi in the directory
/pub/os2/all/editors.  The only thing I did to the sources was the
changing of `#include <regexp.h>' to `#include "regexp.h"' in all the
`*.c' files where that line was found.

The executable file was recompiled using emx 0.8h with GNU gcc 2.5.7.
The trouble with the old executable was that it couldn't read long HPFS
file names.  Recompiling with emx was enough to solve this problem.

For detailed documentation, history & such, see `ed.man'.  I haven't
touched it :-)

====[ END OF README.OS2 ]===============================================
