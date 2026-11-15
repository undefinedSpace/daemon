# undefinedSpace (C++ daemon)

undefinedSpace - It's a system for changes monitoring of files on the hard drive, helpful if you do not know which file was changed a couple of days ago.

undefinedSpace - complex of programs that consists of 3 main blocks: [Daemon](https://github.com/undefinedSpace/daemon), API ([PHP](https://github.com/undefinedSpace/api-php), [JS](https://github.com/undefinedSpace/nodejs-api)), Web UI (soon).

## How to install

First you need the make, gcc and some other tools/libs, on Debian or Ubuntu you can install all what you need via APT:

    apt-get install gcc cmake make zlib1g-dev

Next we need build this program, for this from folder with source code you just need execute:

    cmake .
    make

As a result, you should see the **undefined** binary file in the project directory.

## How to use

To use this program you need to give 2 or more arguments:

    ./undefined <path 1> ... [path n] <api-address>

* path 1, ..., path n - absolute paths in your filesystem
* api-server - ip or dns address of your api

The daemon uses Linux fanotify with directory file handles (`FAN_REPORT_DFID_NAME`).
It needs a kernel and filesystem that support this mode and privileges to create
the fanotify group (normally `CAP_SYS_ADMIN`). If a directory cannot be marked,
the daemon prints an error; changes in that directory may then be missed.
On fanotify queue overflow it rescans the monitored directories.
At startup it raises its soft open-file limit to the process's hard limit
(capped by `/proc/sys/fs/nr_open` if unlimited), before opening monitored
directories. Raising a hard limit imposed by the launching service or shell
requires additional privileges; configure that limit in the service if needed.

File identity is a UUID stored in the `user.undefinedSpace.id` extended attribute;
the inode is kept only as a search hint. The companion xattr
`user.undefinedSpace.identity` records the originating filesystem object. If a
copy preserves both xattrs (for example `cp -a`), the daemon detects that the
object is different and assigns the copy a new UUID. The daemon must be able to
write `user.*` xattrs on every monitored directory and regular file. Files that
cannot hold a UUID (including symlinks on ext4) are skipped with an error. A
rename keeps the UUID; replacing a file with a new filesystem object gives it
a new UUID. Hard links share one UUID, but this database stores only one name
per UUID. Files with UUIDs from older daemon versions receive the new identity
xattr when next scanned. If only the UUID xattr is copied without its companion
identity xattr, the copy cannot be distinguished from an older unmarked file.

For a fresh MariaDB installation, import `extra/undefinedSpace.sql`. It creates
the schema and seeds the `events` table; the daemon adds the local server record
on its first synchronization. The old inode-based database schema is not
compatible with this version.

## History viewer

`web/` contains a read-only HTML history page backed by MariaDB. Install
`python3-mysqldb` and run it locally with a database account that has **SELECT**
access to `undefinedSpace`:

    UNDEFINED_DB_USER=your_user UNDEFINED_DB_PASSWORD=your_password python3 web/server.py

Open http://127.0.0.1:8080/. Set `UNDEFINED_DB_HOST`, `UNDEFINED_DB_PORT`,
`UNDEFINED_DB_NAME` or `UNDEFINED_WEB_PORT` if needed. The page shows projects,
event dates, current paths, search and filters; historical paths cannot be
reconstructed exactly from the current database schema. The web server binds to
localhost by default; do not expose it publicly without authentication.

When a regular file is atomically replaced at the same path, the monitor waits
up to 200 ms before reporting a deletion. If the replacement appears, it sends
one `IS_REPLACED` event instead of `IS_DELETED` and `IS_CREATED`. The new file
gets its own UUID; the old item and its history remain in `items` and `changes`
with `deleted=1`. The replacement change belongs to the new item and stores
both UUIDs (`old_uuid`, `new_uuid`) in its JSON description. Existing databases
do not need a schema migration: the handler adds event 16 on first replacement.
