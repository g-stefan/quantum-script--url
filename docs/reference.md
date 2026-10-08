# API reference

## Script

Available after `Script.requireExtension("URL")`. Built into `fabricare`
and `magnet`.

| Symbol | Returns | Notes |
|--------|---------|-------|
| `URL.encodeComponent(value)` | String | keeps `A-Z a-z . ! ~ * ' ( )`, every other byte → `%XX` uppercase (digits, `-`, `_`, space, UTF-8 bytes included) |
| `URL.decodeComponent(str)` | String | `%XX` (any case) → byte; `+` unchanged; malformed escapes silently dropped |
| `URL.getSchemeName(url)` | String or `null` | text before the first `://`, case kept |
| `URL.getHostNameAndPort(url)` | String or `null` | authority after `@`; no `/` after it → everything after `://` |
| `URL.getUsernameAndPassword(url)` | String or `null` | authority before `@`, still encoded; `null` without `@` or without `/` after the authority |
| `URL.getPathAndFileName(url)` | String or `null` | from the first `/` to `?`; `"/"` without path; keeps `#...` when there is no query |
| `URL.getPathAndFileNameWithQuery(url)` | String or `null` | from the first `/` to `#`; `null` without path |
| `URL.getQuery(url)` | String or `null` | after `?`, before `#`, not decoded; `""` for a bare `?`; `null` without `?` or without path |

Every `get*` function returns `null` when the argument has no `://`.

### Edge cases

| Expression | Result |
|------------|--------|
| `URL.encodeComponent("a b")` | `"a%20b"` |
| `URL.encodeComponent("2026")` | `"%32%30%32%36"` |
| `URL.encodeComponent(255)` | `"%32%35%35"` (the text `"255"`) |
| `URL.encodeComponent()` | `"undefined"` |
| `URL.encodeComponent("ă")` | `"%C4%83"` (older builds: `"%FFFFFFC4%FFFFFF83"`) |
| `URL.decodeComponent("a+b")` | `"a+b"` |
| `URL.decodeComponent("%2f")` | `"/"` |
| `URL.decodeComponent("ab%")` | `"ab"` |
| `URL.decodeComponent("a%zzb")` | `"ab"` |
| `URL.decodeComponent("a%00b").length` | `3` |
| `URL.getSchemeName("HTTP://x/")` | `"HTTP"` |
| `URL.getSchemeName("mailto:a@b")` | `null` |
| `URL.getHostNameAndPort("http://u:p@h")` | `"u:p@h"` (no `/` after the authority) |
| `URL.getHostNameAndPort("http://h?x=1")` | `"h?x=1"` |
| `URL.getHostNameAndPort("file:///C:/f")` | `""` |
| `URL.getUsernameAndPassword("http://u:p@h")` | `null` |
| `URL.getPathAndFileName("http://h")` | `"/"` |
| `URL.getPathAndFileName("http://h/a#f")` | `"/a#f"` |
| `URL.getPathAndFileNameWithQuery("http://h")` | `null` |
| `URL.getQuery("http://h/a?")` | `""` |
| `URL.getQuery("http://h/a?x=1?y=2#f")` | `"x=1?y=2"` |
| `URL.getQuery("http://h?x=1")` | `null` |

### Errors

| Message | Cause |
|---------|-------|
| `Unable to open "URL"` | the extension library was not found and no internal one is registered |

The eight functions themselves never throw. Calling a string method on a
`null` result (`URL.getSchemeName("x").toLowerCaseASCII()`) does: check with
`Script.isNull` first.

## C++

Namespace `XYO::QuantumScript::Extension::URL`, umbrella header
`<XYO/QuantumScript.Extension/URL.hpp>`.

### Library (`URL/Library.hpp`)

| Symbol | Notes |
|--------|-------|
| `void registerInternalExtension(Executive *executive)` | register `"URL"` as an internal extension |
| `void initExecutive(Executive *executive, void *extensionId)` | extension init, run by the engine |
| `extern "C" void quantumScriptExtension(Executive *, void *)` | DLL entry point (not in static builds) |

### Metadata

| Symbol | Notes |
|--------|-------|
| `Version::version()`, `Version::build()`, `Version::versionWithBuild()`, `Version::datetime()` | from `version.json` |
| `Copyright::copyright()`, `Copyright::publisher()`, `Copyright::company()`, `Copyright::contact()` | |
| `License::license()`, `License::shortLicense()` | MIT text |

`Version`, `Copyright` and `License` exist in every XYO library: qualify them
(`Extension::URL::Version::versionWithBuild()`).

### Build configuration

| Name | Meaning |
|------|---------|
| `quantum-script--url` | fabricare project, `dll-or-lib` |
| `quantum-script--url.static` | fabricare project, static `lib` with static CRT, for hosts built with the static CRT (`fabricare`) |
| `XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT` | export / import macro |
| `XYO_QUANTUMSCRIPT_EXTENSION_URL_INTERNAL` | defined while building the DLL (from `QUANTUM_SCRIPT__URL_INTERNAL`) |
| `XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY` | static library: empty export macro, no DLL entry point |
| `XYO_QUANTUMSCRIPT_DEBUG_RUNTIME` | print a trace line on every call |
