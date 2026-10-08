# Quantum Script Extension URL — Documentation

`quantum-script--url` is the **URL extension of Quantum Script**. Loaded with
`Script.requireExtension("URL")`, it adds a `URL` object to scripts with
percent-encoding of URL components and simple splitting of a URL into its
parts:

```javascript
URL.encodeComponent("a b&c");          // "a%20b%26c"       percent-encode a component
URL.decodeComponent("a%20b%26c");      // "a b&c"           decode %XX escapes

var url = "https://user:pass@example.com:8443/dir/file.html?q=1#top";
URL.getSchemeName(url);                // "https"
URL.getHostNameAndPort(url);           // "example.com:8443"
URL.getUsernameAndPassword(url);       // "user:pass"
URL.getPathAndFileName(url);           // "/dir/file.html"
URL.getPathAndFileNameWithQuery(url);  // "/dir/file.html?q=1"
URL.getQuery(url);                     // "q=1"
```

It is the URL layer used by the other scripted network extensions: `HTTP`,
`OpenSSL` (HTTPS) and `SSHRemote` call `URL.getSchemeName`,
`getHostNameAndPort`, `getPathAndFileName` and `getQuery` to turn a URL into
a socket connection and a request line. It is also built into `fabricare`
and `magnet`, so build and automation scripts can use it without any DLL.

- **Plain string slicing, not a full RFC 3986 parser.** Every `get*`
  function looks for the first `://`, then for the first `/` after it, and
  cuts the string around `@`, `?` and `#`. There is no validation,
  normalization, resolution of relative URLs or decoding of the parts.
- **`null` when the part is absent.** A URL without `://` gives `null` from
  every `get*` function; a missing user info or query gives `null` too.
  Test with `Script.isNull(r)` before calling string methods on the result.
- **Works best with a path.** The authority (`user:pass@host:port`) is only
  split cleanly when a `/` follows it: `http://example.com?x=1` gives the
  host `"example.com?x=1"`. Make sure there is a `/` after the host (see
  [Script API](script-api.md#normalize-a-url-before-splitting-it)).
- **Percent-encoding is byte based.** `encodeComponent` keeps only
  `A-Z a-z . ! ~ * ' ( )` and writes every other byte as `%XX` (uppercase
  hex), **digits included**. `decodeComponent` turns `%XX` back into bytes
  and leaves everything else, including `+`, as it is.
- **UTF-8 is encoded byte by byte**: `"ă"` → `"%C4%83"`. Builds made
  before this was fixed (and `fabricare` until it is rebuilt) wrote
  `%FFFFFFXX` instead; see
  [Script API](script-api.md#non-ascii-text).

```
scripts: quantum-script .js, fabricare build scripts, magnet, embedding hosts, ...
quantum-script--http / --openssl / --sshremote   (use URL.get* to open connections)
quantum-script--url       <-- this extension: URL.encodeComponent / decodeComponent / get*
quantum-script            (Executive, Variable, Context)
xyo-system, xyo-encoding, xyo-multithreading, xyo-data-structures, xyo-managed-memory, xyo-platform
```

## Why it exists

| Need | What `URL` gives |
|------|------------------|
| Put arbitrary text into a URL query or path segment | `URL.encodeComponent(value)` — `&`, `=`, `?`, `/`, `#`, space, ... become `%XX` |
| Read values received in a URL (CGI `QUERY_STRING`, links, redirects) | `URL.decodeComponent(text)` |
| Open a connection from a URL | `getSchemeName` (protocol), `getHostNameAndPort` (where to connect) |
| Build an HTTP request line | `getPathAndFileNameWithQuery` (or `getPathAndFileName` + `getQuery`) |
| Get the file name of a download | `Shell.getFileName(URL.getPathAndFileName(url))` |
| Read credentials embedded in a URL | `getUsernameAndPassword` (`"user:pass"`, still encoded) |
| Same API in every host | available in `quantum-script`, `fabricare`, `magnet` and hosts that register it |

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Load the extension | `Script.requireExtension("URL");` | no other extension required |
| Encode a query value | `URL.encodeComponent("a b")` | `"a%20b"`; digits become `%3X` too |
| Decode a query value | `URL.decodeComponent("a%20b")` | `"a b"`; `+` is **not** a space |
| Decode an HTML form value | `URL.decodeComponent(v.replace("+", " "))` | `replace` replaces all |
| Protocol | `URL.getSchemeName(url)` | as written (`"HTTP"` stays uppercase) |
| Host and port | `URL.getHostNameAndPort(url)` | `"host:port"` or `"host"`; split the port yourself |
| User and password | `URL.getUsernameAndPassword(url)` | `"user:pass"`, or `null` |
| Path | `URL.getPathAndFileName(url)` | `"/"` when there is no path |
| Path with query (request target) | `URL.getPathAndFileNameWithQuery(url)` | `null` when there is no path |
| Query | `URL.getQuery(url)` | without `?` and `#...`; `null` when absent, `""` for a bare `?` |
| Check a missing part | `Script.isNull(r)` | not `!r`: an empty query is `""` |
| Parse a query into an object | split on `&` and `=`, decode each part | see [recipes](script-api.md#recipes) |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build and install, load the extension from a script, fabricare and magnet scripts, register it in a C++ host |
| [Script API](script-api.md) | Every function: exact splitting and encoding rules, edge cases, recipes |
| [C++ API](cpp-api.md) | `registerInternalExtension`, `initExecutive`, the DLL entry point, the static project, notes for maintainers |
| [API reference](reference.md) | Every script and C++ symbol on one page |

Quantum Script itself (the language, `Script.requireExtension`, embedding,
writing extensions) is documented in the `quantum-script` repository,
`docs/`. The extensions that build on `URL` (`HTTP`, `OpenSSL`,
`SSHRemote`) are documented in their own repositories.

## Source map

```
source/XYO/QuantumScript.Extension/URL.hpp            umbrella header, include this from C++
source/XYO/QuantumScript.Extension/URL.Amalgam.cpp    the whole extension in one translation unit
source/XYO/QuantumScript.Extension/URL/
    Dependency.hpp                                    <XYO/QuantumScript.hpp>, export macro
    Library[.hpp/.cpp]                                initExecutive, registerInternalExtension,
                                                      encodeComponent / decodeComponent / get*
    Copyright / License / Version                     library metadata
fabricare.json                                        quantum-script--url (dll-or-lib), quantum-script--url.static
fabricare/test.js                                     "fabricare test": runs test/test.0001.js
test/test.0001.js                                     encode / decode round trip of URL special characters
```

## AI assistant skill

A Claude Code skill describing how to use this extension lives in
[`.claude/skills/quantum-script--url/`](../.claude/skills/quantum-script--url/SKILL.md).
It is picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that use `URL`
(fabricare scripts, magnet scripts, `HTTP` / `OpenSSL` / `SSHRemote`,
other Quantum Script tools).
