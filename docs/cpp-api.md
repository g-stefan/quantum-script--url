# C++ API

For hosts that embed Quantum Script and for maintainers of the extension.
Read the `quantum-script` repository's `docs/embedding.md` and
`docs/writing-extensions.md` first: native functions, `Variable` and
`TPointer` work the same way here.

## Headers and namespace

```cpp
#include <XYO/QuantumScript.Extension/URL.hpp>   // Library.hpp

using namespace XYO::QuantumScript;
```

Namespace: `XYO::QuantumScript::Extension::URL`. Export macro:
`XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT`:

| Define | Effect |
|--------|--------|
| `XYO_QUANTUMSCRIPT_EXTENSION_URL_INTERNAL` (or `QUANTUM_SCRIPT__URL_INTERNAL`, set by fabricare while building the DLL) | export macro = `XYO_PLATFORM_LIBRARY_EXPORT` |
| none | export macro = `XYO_PLATFORM_LIBRARY_IMPORT` (consumers of the DLL) |
| `XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY` (set for dependents of `quantum-script--url.static`) | export macro empty, no `quantumScriptExtension` entry point |
| `XYO_PLATFORM_COMPILE_STATIC` (static platforms) | `XYO_PLATFORM_LIBRARY_EXPORT` / `IMPORT` are empty |

The `quantumScriptExtension` entry point is compiled only when
`XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY` is defined and
`XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY` is not.

## Registering the extension

```cpp
void Extension::URL::registerInternalExtension(Executive *executive);
void Extension::URL::initExecutive(Executive *executive, void *extensionId);
```

- `registerInternalExtension` registers `"URL"` as an internal extension;
  call it from the host's init callback (see
  [Getting started](getting-started.md#4-register-it-in-a-c-host)).
  `fabricare` (`XYO/Fabricare/Library.cpp`) and `quantum-script--magnet`
  (`XYO/QuantumScript.Extension/Magnet/Library.cpp`) do this.
- `initExecutive` is the extension's init function, run by the engine when a
  script first requires `URL` in a thread. It sets the extension name,
  info (`"URL"` plus the short license text), version and public flag,
  then:

  ```cpp
  executive->compileStringX("var URL={};");
  executive->setFunction2("URL.decodeComponent(str)", decodeComponent);
  executive->setFunction2("URL.encodeComponent(value)", encodeComponent);
  executive->setFunction2("URL.getSchemeName(url)", getSchemeName);
  executive->setFunction2("URL.getHostNameAndPort(url)", getHostNameAndPort);
  executive->setFunction2("URL.getUsernameAndPassword(url)", getUsernameAndPassword);
  executive->setFunction2("URL.getPathAndFileName(url)", getPathAndFileName);
  executive->setFunction2("URL.getPathAndFileNameWithQuery(url)", getPathAndFileNameWithQuery);
  executive->setFunction2("URL.getQuery(url)", getQuery);
  ```

  Do not call it directly. `URL` requires no other extension.
- The DLL build also exports
  `extern "C" void quantumScriptExtension(Executive *, void *)`, which
  forwards to `initExecutive`; it is what `Script.requireExtension` looks up
  in `quantum-script--url.dll`.

## The native functions

All eight are `static` in `URL/Library.cpp`. Each reads
`arguments->index(0)->toString()` and works on that `String` with
`indexOf`, `substring` and `split2` (which splits at the first occurrence):

| Script function | Implementation | Returns |
|-----------------|----------------|---------|
| `encodeComponent(value)` | copies `A-Z a-z . ! ~ * ' ( )`, writes other bytes with `snprintf(buf, sizeof(buf), "%02X", static_cast<unsigned char>(in[k]))` | `VariableString` |
| `decodeComponent(str)` | on `%`, reads the next two characters with `sscanf(buf, "%02X", &value)` | `VariableString` |
| `getSchemeName(url)` | `substring(0, index of "://")` | `VariableString` or `VariableNull` |
| `getHostNameAndPort(url)` | authority between `://` and the next `/`, `split2("@")` → second part; no `/` → everything after `://` | `VariableString` or `VariableNull` |
| `getUsernameAndPassword(url)` | same authority, `split2("@")` → first part | `VariableString` or `VariableNull` |
| `getPathAndFileName(url)` | from the `/`, `split2("?")` → first part; no `/` → `"/"` | `VariableString` or `VariableNull` |
| `getPathAndFileNameWithQuery(url)` | from the `/`, `split2("#")` → first part | `VariableString` or `VariableNull` |
| `getQuery(url)` | after the `/`, `split2("?")` → second part, `split2("#")` → first part | `VariableString` or `VariableNull` |

Absent parts are returned as `VariableNull::newVariable()` (script `null`),
not `undefined`. With `XYO_QUANTUMSCRIPT_DEBUG_RUNTIME` defined each
function prints a trace line (`- url-encode-component`,
`- url-get-host-name-and-port`, ...).

### Bytes above `0x7F` in `encodeComponent`

`in[k]` is a `char`, which is signed on the supported compilers. Passed to
`"%02X"` as is, bytes `0x80`-`0xFF` are promoted to a negative `int` and
print eight hex digits (`FFFFFFC4`). Older builds did exactly that, into a
`char buf[4]`: a wrong escape for the script and a buffer overflow in the
native code. The byte must be cast to `unsigned char`, and `snprintf` with
`sizeof(buf)` keeps the write inside `buf[3]`. `test/test.0002.js` guards
this with UTF-8 input (`"é"`, `"ă"`, `"€"`).

## Notes for maintainers

- New functions: register them in `initExecutive` with
  `executive->setFunction2("URL.name(args)", name)`, then update
  `README.md`, `docs/script-api.md`, `docs/reference.md`, the skill in
  `.claude/skills/quantum-script--url/` and the tests in `test/`.
- Behavior changes are visible to `quantum-script--http`,
  `quantum-script--openssl`, `quantum-script--sshremote` and fabricare's
  `gitea-release-download` script, which depend on the exact strings and
  `null` results documented in [Script API](script-api.md). Keep them
  compatible or update those callers too.
- `fabricare` links the extension statically (`quantum-script--url.static`,
  from its vendored copy in `fabricare/vendor/quantum-script--url`) and
  `quantum-script--magnet` registers it as internal: a fix reaches them only
  after they are rebuilt with the new version (and, for fabricare, the
  vendored copy is updated).
- Tests: `test/test.0001.js` encodes and decodes the URL special characters,
  `test/test.0002.js` UTF-8 text; both check the exact encoded form and the
  round trip. `fabricare/test.js` runs `test/test.000k.js` for `k` from 1
  to 2; raise the upper bound when adding `test.0003.js`. The tests use
  the `quantum-script` interpreter on `PATH`, which loads the **installed**
  DLL: `fabricare install` before `fabricare test`.
