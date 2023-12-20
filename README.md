Usage:

`./mytar [ctxvS[f tarfile]] [file1 [ file2 [...] ] ]`

Files:
- mytar.c: Handles command line arguments, as well as shared code between list mode and extract mode.
- tarCreate.c, tarList.c, tarExtract.c: Exclusive code for each main tar mode 
- tools.c: Safe mallocs, string operations, and other useful math defs
    
