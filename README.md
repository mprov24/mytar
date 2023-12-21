# Build

`make`

# Usage

`./mytar [ctxvS[f tarfile]] [file1 [ file2 [...] ] ]`

Options:
- c - Create tarfile from files
- t - List contents of tarfile
- x - Extract tarfile
- v - Verbose mode
- S - Strict adherence to header format and version

# Files
- mytar.c: Handles command line arguments, as well as shared code between list mode and extract mode.
- tarCreate.c, tarList.c, tarExtract.c: Exclusive code for each main tar mode 
- tools.c: Safe mallocs, string operations, and other useful math defs
    
