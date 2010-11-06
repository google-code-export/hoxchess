How to compile:
---------------
- Require Qt Creator 2.0.1 with Qt 4.7.0
- Simply open the project file 'QtXiangqi.pro' using Qt Creator.

Notes to run under Mac OS X:
----------------------------
After fully built, to run the binary under Mac OS X, we need to manuall add the
following files/directories in the 'QtXiangqi' bundle:

+ QtXiangqi | Contents | Resources
 ++ books
  +++ BOOK.DAT
 ++ sounds
  +++ CAPTURE.WAV
  +++ CAPTURE2.WAV
  +++ ... the rest of the sound files

================ END OF FILE =======================
