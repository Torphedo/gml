# gml
A proof-of-concept generic mod loader for Windows. It uses PE injection to load
into a game process without a separate DLL. Function hooks are used to make a
game load files from another folder, allowing modders to edit files without
overwiting them.

Most testing was done on Notepad, and built with MinGW in CLion. The PE
injection code isn't fully correct yet, so it will probably not work right with
MSVC builds. MinGW builds have no issues, which is probably due to different
linker settings.

A lot of this code was taken from my Phantom Dust mod loading system. Most of
that codebase is unrelated to any specific game process, and I wanted to write
something that could be used on multiple games. My hope is that this project
can lower the barrier to entry for more complex mods and give early modding
efforts a head start.

## Planned Features
### Actual File Redirection
This would take maybe 50 lines of code, I just haven't gotten around to it
because of school. This would force games to load from a `mods` folder next to
their executable if possible.

### Virtual Filesystem Support
Like the Phantom Dust loader, this would allow assets (and maybe code) to be
loaded from 7zip/rar/zip files via the PhysicsFS library. I want a real
in-memory implementation this time, not one that extracts files to disk first
and returns new WinAPI handles. This is really difficult for files accessed
from multiple threads.

