# Time-Travelling File System 

## Overview
An in-memory version-control system inspired by Git, built with custom implementations of:
- **Tree** — each file's version history (TreeNode with parent/children pointers)
- **HashMap** — O(1) average lookup of versions by ID (separate chaining)
- **Heap** — system-wide analytics (max-heap with custom comparator)

No C++ STL containers (`map`, `set`, `priority_queue`) are used for these data structures.
`std::unordered_map` is used **only** as a name→File* registry at the FileSystem level (not a graded data structure).

---

## How to Compile

```bash
chmod +x compile.sh
./compile.sh
```

Or manually:
```bash
g++ -std=c++17 -O2 -Wall main.cpp FileSystem.cpp -o vfs
```

## How to Run

```bash
./vfs
```

The program reads commands interactively from **stdin** (`vfs> ` prompt).  
You can also pipe a script file:
```bash
./vfs < commands.txt
```

---

## Command Reference

| Command | Syntax | Description |
|---|---|---|
| CREATE | `CREATE <filename>` | Creates a file with root version 0 (auto-snapshot). |
| READ | `READ <filename>` | Prints the active version's content. |
| INSERT | `INSERT <filename> <content>` | Appends content. Creates new version if active is a snapshot; edits in-place otherwise. |
| UPDATE | `UPDATE <filename> <content>` | Replaces content. Same branching logic as INSERT. |
| SNAPSHOT | `SNAPSHOT <filename> <message>` | Freezes the active version, marking it immutable. |
| ROLLBACK | `ROLLBACK <filename> [versionID]` | Moves active pointer to the given version (or parent if no ID given). |
| HISTORY | `HISTORY <filename>` | Lists all snapshots chronologically (ID, timestamp, message). |
| RECENT_FILES | `RECENT_FILES` | Files sorted by last modification time (descending). |
| BIGGEST_TREES | `BIGGEST_TREES` | Files sorted by total version count (descending). |
| HELP | `HELP` | Prints command list. |
| EXIT | `EXIT` | Quits the program. |

> **Tip:** Content or messages containing spaces must be wrapped in double quotes.  
> Example: `INSERT notes.txt "Hello World"`

---

## Key Semantics & Error Handling

| Situation | Behaviour |
|---|---|
| CREATE on existing file | Prints error, no change. |
| READ/INSERT/UPDATE/SNAPSHOT/ROLLBACK on missing file | Prints error, no change. |
| SNAPSHOT on already-snapshotted version | Prints error, no change. |
| INSERT/UPDATE on snapshot version | Creates a new child version (branching). |
| INSERT/UPDATE on non-snapshot version | Edits content in-place (no new node). |
| ROLLBACK without ID | Rolls back to parent; error if already at root. |
| ROLLBACK to non-existent version ID | Prints error, no change. |
| Invalid version ID (non-integer) | Prints error, no change. |

---

## Architecture

```
main.cpp          — CLI parser, command dispatch
FileSystem.hpp/cpp — orchestrates files, implements all commands
File.hpp          — File struct (owns the version tree + HashMap)
TreeNode (File.hpp)— one version node (id, content, message, timestamps, parent, children)
HashMap.hpp       — custom hash map (separate chaining, template)
Heap.hpp          — custom max-heap (comparator-based, template)
```

---

## Sample Session

```
vfs> CREATE notes.txt
Created file 'notes.txt' (version 0, initial snapshot).

vfs> INSERT notes.txt "Hello, world!"
Inserted into new version v1 of 'notes.txt'.

vfs> SNAPSHOT notes.txt "First draft"
Snapshot taken at version v1 of 'notes.txt': "First draft".

vfs> INSERT notes.txt " More text."
Inserted into new version v2 of 'notes.txt'.

vfs> READ notes.txt
[notes.txt | v2]
Hello, world! More text.

vfs> ROLLBACK notes.txt 1
Rolled back 'notes.txt' to version v1.

vfs> READ notes.txt
[notes.txt | v1 (snapshot)]
Hello, world!

vfs> HISTORY notes.txt
Snapshot history of 'notes.txt':
ID    Timestamp             Message
------------------------------------------------------------
v0    2025-09-01 10:00:00   Initial version
v1    2025-09-01 10:00:05   First draft
```
