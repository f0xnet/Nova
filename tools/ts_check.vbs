' Nova incremental build — timestamp checker
' Usage: cscript //nologo ts_check.vbs <source.cpp> <dest.o>
' Exit 1 = source newer than dest, recompile needed
' Exit 0 = dest up to date

Dim fso, src, obj
Set fso = CreateObject("Scripting.FileSystemObject")

src = WScript.Arguments(0)
obj  = WScript.Arguments(1)

If Not fso.FileExists(obj) Then WScript.Quit 1
If Not fso.FileExists(src) Then WScript.Quit 0

If fso.GetFile(src).DateLastModified > fso.GetFile(obj).DateLastModified Then
    WScript.Quit 1
End If

WScript.Quit 0
