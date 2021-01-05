rc /Fo.\DrMem\SpaceCadetPinball.res ".\SpaceCadetPinball\SpaceCadetPinball.rc"

 cl /Zi /MT /EHsc /O  /Ob0 /cgthreads4 /Fo.\DrMem\ /Fe.\DrMem\myapp.exe ".\SpaceCadetPinball\*.cpp" Comctl32.lib Winmm.lib Htmlhelp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ".\DrMem\SpaceCadetPinball.res"