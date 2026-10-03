# Exact source-archive qualification

`verify.py` audits a `swift package archive-source` ZIP, extracts it into a
fresh temporary directory, makes the extracted package read-only, creates an
independent Swift package consumer whose only CNA dependency is that extracted
artifact, builds debug and release, and runs native 60/600-frame 2D canaries.
It rejects `.git`, `.build`, native libraries, static libraries, and developer
path leaks in the source archive.
