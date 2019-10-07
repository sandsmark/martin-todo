martin-todo
===========

![screenshot](/screenshot.png)

Extremely simple todo app (168 lines of code including the .pro file). Supports
adding items, checking items, editing items (double click). Empty items are
removed.

Click the systray icon to show and hide. Ctrl+Q to quit (unless
on e. g. macos, it adapts to the platform defaults).

Stores items in ~/todo.txt, one item per line. If the line starts with `x` it
is done/checked, `-` (or any other prefix) is unchecked.

`qmake && make && ./martin-todo` to build and run.

