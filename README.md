# The noddl Postgres Extension

Have you ever thought to yourself, "Self, I don't want users running DDL on my Postgres database, but I have no way to stop them!"

Well today is your lucky day, because now _you absolutely can_!

When enabled, the noddl extension will prevent _any_ DDL execution, even from superusers!

## Installation

Installing this extension is simple:

```bash
git clone git@github.com:bonesmoses/noddl.git
cd noddl
make
sudo make install
```

Then add `noddl` to `shared_preload_libraries` in your `postgresql.conf`, 
and choose your desired mode like so:

```ini
shared_preload_libraries = 'noddl'
noddl.enable = true
```

Then restart Postgres to load the noddl library, and you're done!

You can optionally install the "extension" portion so this module is listed in `pg_extensions` as an installed extension:

```sql
CREATE EXTENSION noddl;
```

Or can you? ;)

## Usage

This extension has approximately one parameter:

* `noddl.enable` - Boolean value to block or allow DDL execution. May only be set by superusers or as a system option. Default false.

This allows enabling the extension in `postgresql.conf` or through `ALTER SYSTEM` to act as a default DDL blocker:

```sql
ALTER SYSTEM SET noddl.enable TO true;
SELECT pg_reload_conf();
```

While enabled, superusers can temporarily disable the extension during a session:

```sql
SET noddl.enable TO false;
```

Or permanently across the instance:

```sql
SET noddl.enable TO false;
ALTER SYSTEM SET noddl.enable TO false;
SELECT pg_reload_conf();
```

Note that all DDL means **all** DDL, including creation of temp tables. This prevents unnecessary traffic on the Postgres catalog tables, as these also track temp tables.

## Discussion

This is primarily a joke extension meant to act as a learning exercise or skeleton for writing Postgres extensions. It shouldn't really be taken seriously, but is still useful on its own.

What's interesting here is that this extension uses a few undocumented pieces of the Postgres extension API to do its work.

From  [`tcop/utility.c`](https://github.com/postgres/postgres/blob/master/src/backend/tcop/utility.c)
* The `ProcessUtility_hook` callback hook
* The `standard_ProcessUtility` function
* The `GetCommandLogLevel` function to decode statement type

The extension works by replacing `standard_ProcessUtility` with its own statement processor. Each statement is checked with `GetCommandLogLevel` for any DDL, and rejected if found. If not, the Postgres `standard_ProcessUtility` statement processor continues as normal.

Who knew circumventing the Postgres statement processor was so easy?
