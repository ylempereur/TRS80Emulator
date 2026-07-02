# TRS-80 Emulator for macOS (v1.6)

For the documentation, go to:

<https://yves.lempereur.name/trs80.html>

Quick note about the resource file TRS80.rsrc:  
  
TRS80.rsrc is a pure resource file (it only has a resource fork, no data fork).
Git does not understand resource forks, so the file got added as an empty file.
After thinking about it for a bit, I remembered that, on volumes that do not
support resource forks, macOS splits files with a resource fork into two separate files.
So, I copied the git repo to an ExFAT partition, which allowed me to add the resource fork
to the repo as "._TRS80.rsrc". Problem solved.  
However, after thinking about it some more, I realized that my goal here is to document
the source code for posterity, not to expect anyone to try and build it.
So, I used DeRez to turn the file into a source file and added it as "TRS80.rsrc.r".
