---
name: quantum-script--url
description: >-
  How to use the Quantum Script URL extension (quantum-script--url), loaded
  with Script.requireExtension("URL") and built into fabricare and magnet:
  URL.encodeComponent(value) (keeps only A-Z a-z . ! ~ * ' ( ), every other
  byte -> %XX uppercase, digits and - _ included, space is %20, UTF-8 byte
  by byte; older builds gave %FFFFFFXX for non-ASCII), URL.decodeComponent(str)
  (%XX any case -> byte, "+" is not a space, malformed escapes silently
  dropped), and the string-slicing URL splitters URL.getSchemeName,
  getHostNameAndPort, getUsernameAndPassword, getPathAndFileName,
  getPathAndFileNameWithQuery, getQuery (first "://", first "/" after it,
  split at "@" "?" "#", nothing decoded, null when absent or no "://", odd
  results when no "/" follows the host); recipes for building / parsing
  query strings, form "+" decoding, normalizing a URL, host / port split,
  request line, download file name, credentials; how HTTP / OpenSSL HTTPS /
  SSHRemote use it; the C++ side (registerInternalExtension, initExecutive,
  quantumScriptExtension entry point, quantum-script--url.static). Use when
  writing or reviewing Quantum Script or fabricare .js code that builds,
  splits or percent-encodes URLs, C++ code that includes
  <XYO/QuantumScript.Extension/URL.hpp>, a fabricare.json depending on
  "quantum-script--url", or when working inside the quantum-script--url
  repository.
---

# quantum-script--url

URL extension of Quantum Script (see the `quantum-script` skill for the
language and its differences from JavaScript; its rules apply). Purpose:
**percent-encode URL components and cut a URL into scheme, host:port, user
info, path and query** — the layer `HTTP`, `OpenSSL` (`HTTPS`) and
`SSHRemote` use to turn a URL into a connection and a request line. It is a
string slicer, not an RFC 3986 parser: no validation, no normalization, no
relative URL resolution, nothing decoded.

Full documentation: `docs/` in the quantum-script--url repository
(`X:\Storage\XYO\Gitea\CPP\quantum-script--url\docs` on this machine):
README (purpose), getting-started (build, load, hosts, C++ registration),
**script-api** (exact rules, result table, recipes), cpp-api,
reference. When in doubt read
`source/XYO/QuantumScript.Extension/URL/Library.cpp` (~220 lines).

## Script API

```javascript
Script.requireExtension("URL");     // already loaded in fabricare scripts

URL.encodeComponent("a b&c");       // "a%20b%26c"
URL.decodeComponent("a%20b%26c");   // "a b&c"

var url = "https://user:pass@example.com:8443/dir/file.html?q=1#top";
URL.getSchemeName(url);                // "https"
URL.getHostNameAndPort(url);           // "example.com:8443"
URL.getUsernameAndPassword(url);       // "user:pass"        (still encoded)
URL.getPathAndFileName(url);           // "/dir/file.html"
URL.getPathAndFileNameWithQuery(url);  // "/dir/file.html?q=1"
URL.getQuery(url);                     // "q=1"              (no "?", no "#top", not decoded)
```

## Hard rules

1. **Absent parts are `null`**, never `undefined`. No `://` in the argument
   → every `get*` returns `null` (`mailto:`, `example.com/a`, relative
   paths). No `@` → `getUsernameAndPassword` is `null`; no `?` →
   `getQuery` is `null`. Check with `Script.isNull(r)` before calling a
   method (`URL.getSchemeName(u).toLowerCaseASCII()` throws on `null`).
2. **`getQuery` can be `""`** (bare `?`): test absence with `Script.isNull`,
   not `!r`, when the difference matters.
3. **A `/` must follow the host.** Without it: `getHostNameAndPort` returns
   everything after `://` (`"user:pass@h"`, `"h?x=1"`),
   `getUsernameAndPassword` and `getQuery` are `null`,
   `getPathAndFileName` is `"/"`, `getPathAndFileNameWithQuery` is `null`.
   Normalize first (recipe below) when input may lack the path.
4. **`getPathAndFileName` keeps `#frag` when there is no query**
   (`/a#frag`); `getPathAndFileNameWithQuery` cuts at `#`.
5. **Nothing is decoded or lower-cased**: scheme case is kept (`"HTTP"`),
   compare with `toLowerCaseASCII()` / `toUpperCaseASCII()`; decode path,
   query values and user info yourself with `decodeComponent`.
6. **Port is not split or defaulted**: `"example.com:8443"` or
   `"example.com"`; split at `lastIndexOf(":")` unless a `]` follows
   (IPv6 `[::1]`), default 80 / 443 yourself.
7. **`encodeComponent` keeps only `A-Z a-z . ! ~ * ' ( )`**: digits,
   `-`, `_` are encoded too (`"2026"` → `"%32%30%32%36"`), space is `%20`
   (never `+`). Valid, but differs from JavaScript's
   `encodeURIComponent`; do not compare its output with other tools.
   Encode one component (name, value, path segment) at a time, never a
   whole URL (`:` `/` `?` `&` `=` `#` get encoded).
8. **UTF-8 is encoded byte by byte**: `encodeComponent("ă")` is
   `"%C4%83"`. Builds before the fix gave `"%FFFFFFC4%FFFFFF83"` (signed
   `char` through `sprintf("%02X")`); fabricare keeps a vendored copy, so
   its scripts behave that way until fabricare is rebuilt. If output must
   match `encodeURIComponent` (digits, `-`, `_` kept), use the `Buffer`
   based encoder below.
9. **`decodeComponent` does not map `+` to space** — for form data use
   `URL.decodeComponent(v.replace("+", " "))` (`replace` replaces all). It
   never fails: `%` at the end, a lone digit after `%`, or a non-hex pair
   are dropped; `%4g` gives byte `0x04`. Validate yourself if needed.
10. The argument is converted with `toString`: `encodeComponent()` is
    `"undefined"`, numbers are encoded as their decimal text.
11. Available in the `quantum-script` interpreter (DLL), `fabricare` (static,
    preloaded), `magnet`, and hosts that register it. One engine per
    thread: each thread requires `URL` itself; functions are stateless.

## Recipes

```javascript
// query string from an object
function buildQuery(params) {
	var query = "";
	for (var name in params) {
		if (query != "") { query += "&"; };
		query += URL.encodeComponent(name) + "=" + URL.encodeComponent(params[name]);
	};
	return query;
};

// query string to an object (CGI: Shell.getenv("QUERY_STRING"))
function parseQuery(query) {
	var result = {};
	if (Script.isNull(query) || query == "") { return result; };
	var pairs = query.split("&");
	for (var i = 0; i < pairs.length; ++i) {
		if (pairs[i] == "") { continue; };
		var index = pairs[i].indexOf("=");
		var name = (index < 0) ? pairs[i] : pairs[i].substring(0, index);
		var value = (index < 0) ? "" : pairs[i].substring(index + 1);
		result[URL.decodeComponent(name.replace("+", " "))] = URL.decodeComponent(value.replace("+", " "));
	};
	return result;
};

// ensure a "/" after the host: "http://u:p@h?x=1" -> "http://u:p@h/?x=1"
function normalizeURL(url) {
	var scheme = URL.getSchemeName(url);
	if (Script.isNull(scheme)) { return url; };
	var start = scheme.length + 3;
	var stop = url.length;
	var marks = ["/", "?", "#"];
	for (var i = 0; i < marks.length; ++i) {
		var index = url.indexOf(marks[i], start);
		if (index >= 0 && index < stop) { stop = index; };
	};
	if (url.substring(stop, 1) == "/") { return url; };
	return url.substring(0, stop) + "/" + url.substring(stop);
};

// percent-encode like encodeURIComponent / RFC 3986 (digits, - _ kept)
function encodeComponentRFC3986(text) {          // needs Script.requireExtension("Buffer")
	var bytes = Buffer.fromString(text);
	var hex = bytes.toHex().toUpperCaseASCII();
	var out = "";
	for (var i = 0; i < bytes.length; ++i) {
		var c = bytes.getU8(i);
		if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 48 && c <= 57) || c == 45 || c == 46 || c == 95 || c == 126) {
			out += text.substring(i, 1);
		} else {
			out += "%" + hex.substring(i * 2, 2);
		};
	};
	return out;                                  // "café 2026" -> "caf%C3%A9%202026"
};

var server = URL.getSchemeName(url) + "://" + URL.getHostNameAndPort(url);   // base URL
var target = URL.getPathAndFileNameWithQuery(url);                          // request target
if (Script.isNull(target)) { target = "/"; };
var fileName = URL.decodeComponent(Shell.getFileName(URL.getPathAndFileName(url)));
var info = URL.getUsernameAndPassword(url);                                 // "user:p%40ss" or null
```

`substring(start, length)` takes a length; `indexOf` returns `-1` when not
found.

## C++

```cpp
#include <XYO/QuantumScript.Extension/URL.hpp>
using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {                 // host init callback
	Extension::URL::registerInternalExtension(executive);  // scripts still requireExtension("URL")
};
```

- fabricare.json dependency: `"quantum-script--url"` (`dll-or-lib`: DLL on
  dynamic platforms, static lib on `*.static` platforms), or
  `"quantum-script--url.static"` for a host with the static CRT (as
  fabricare does). Pulls in `quantum-script`, `quantum-script--console`.
- DLL entry point `extern "C" quantumScriptExtension(Executive *, void *)`
  only with `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY` and without
  `XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY`. Static hosts must register the
  extension as internal.

## Working in this repository

- Build: `fabricare make`, `fabricare install`, `fabricare test` (see the
  `fabricare` skill). `quantum-script` and `quantum-script--console` must be
  installed first. `fabricare test` runs
  `quantum-script --execution-time test/test.000k.js` (k = 1..2: special
  characters, UTF-8) with the interpreter on PATH, which loads the
  **installed** DLL: install before testing a change. Raise the loop bound
  in `fabricare/test.js` when adding `test/test.0003.js`.
- Native functions live in `URL/Library.cpp` as
  `static TPointer<Variable> name(VariableFunction *, Variable *this_, VariableArray *arguments)`
  and are registered in `initExecutive` with
  `executive->setFunction2("URL.name(args)", name)`; absent results are
  `VariableNull::newVariable()`.
- Callers depend on the exact strings and `null` results:
  quantum-script--http, quantum-script--openssl, quantum-script--sshremote,
  fabricare's `Internal/gitea-release-download.js`. fabricare uses a
  vendored copy (`fabricare/vendor/quantum-script--url`).
- `encodeComponent` formats bytes with
  `snprintf(buf, sizeof(buf), "%02X", static_cast<unsigned char>(in[k]))`
  into `char buf[3]`; keep the cast (signed `char` gives `FFFFFFXX` and
  overflows the buffer). The vendored copy in fabricare still has the old
  `sprintf` until it is updated.
- New functions or behavior changes: update `README.md`,
  `docs/script-api.md`, `docs/reference.md`, the tests in `test/` and this
  skill.
- Code style: tabs (width 8), `.clang-format`, CRLF, statements and blocks
  end with `};`, camelCase. SPDX: MIT for `source/` and `docs/`, Unlicense
  for `test/`, `fabricare/` and `.claude/` (see `.reuse/dep5`).
