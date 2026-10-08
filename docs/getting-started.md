# Getting started

## 1. Build and install

The extension is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. `quantum-script` (and everything
below it: `xyo-system`, `xyo-encoding`, ...) and `quantum-script--console`
must be installed to the SDK first. From the repository root:

```bash
fabricare make       # build into output/
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare test       # run test/test.0001.js with the quantum-script interpreter
fabricare clean      # remove output/ and temp/
```

`fabricare.json` declares two projects:

| Project | Kind | Purpose |
|---------|------|---------|
| `quantum-script--url` | `dll-or-lib`: shared library in a dynamic build, static library in a static build | the extension; depends on `quantum-script`, `quantum-script--console` |
| `quantum-script--url.static` | `lib`, static CRT | the extension as a static library for hosts built with the static CRT (`fabricare`); dependents get `XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY` defined |

After `fabricare install`, `quantum-script--url.dll` (Windows) /
`libquantum-script--url.so` (Linux) sits in the SDK `bin` folder next to
`quantum-script.exe`, which is where `Script.requireExtension("URL")` finds
it.

`fabricare test` (`fabricare/test.js`) runs
`quantum-script --execution-time test/test.0001.js` and stops on failure.
The interpreter is the one on `PATH` and it loads the extension from its own
folder, so install a changed build before testing it.

## 2. Use it from a script

```javascript
Script.requireExtension("Console");
Script.requireExtension("URL");

var url = "http://example.com:8080/docs/index.html?lang=ro&q=a%20b#top";

Console.writeLn(URL.getSchemeName(url));                // http
Console.writeLn(URL.getHostNameAndPort(url));           // example.com:8080
Console.writeLn(URL.getPathAndFileName(url));           // /docs/index.html
Console.writeLn(URL.getQuery(url));                     // lang=ro&q=a%20b
Console.writeLn(URL.getPathAndFileNameWithQuery(url));  // /docs/index.html?lang=ro&q=a%20b

var search = "rock & roll";
var link = "http://example.com/search?q=" + URL.encodeComponent(search);
Console.writeLn(link);                                  // http://example.com/search?q=rock%20%26%20roll
Console.writeLn(URL.decodeComponent("rock%20%26%20roll"));   // rock & roll

if (Script.isNull(URL.getUsernameAndPassword(url))) {
	Console.writeLn("no credentials in the URL");
};
```

Run it with:

```bash
quantum-script hello-url.js
```

`Script.requireExtension("URL")` looks for an external
`quantum-script--url` library first (the file as named, then every include
path folder: next to the interpreter, next to the script), then for an
internal extension registered by the host. Loading twice does nothing. A
missing extension throws `Unable to open "URL"`.

## 3. fabricare and magnet scripts

Both `fabricare` and `magnet` register `URL` as an internal extension, so
their scripts can use it without any DLL. `fabricare` itself loads it at
startup, so in a fabricare build script `URL` is already defined:

```javascript
// in a fabricare script: the server and file name of a release archive
var archive = "https://gitea.example.com/owner/project/releases/download/v1.0.0/project-1.0.0.7z";
var server = URL.getSchemeName(archive) + "://" + URL.getHostNameAndPort(archive);
// "https://gitea.example.com", used to call the server's API
var fileName = Shell.getFileName(URL.getPathAndFileName(archive));
// "project-1.0.0.7z"
```

This is how fabricare's own `gitea-release-download` script finds the API
of the server that hosts a release.

In other hosts call `Script.requireExtension("URL")` first; it is harmless
when the extension is already loaded.

## 4. Register it in a C++ host

A host that embeds Quantum Script makes `URL` available as an internal
extension by registering it in the init callback:

```cpp
#include <XYO/QuantumScript.hpp>
#include <XYO/QuantumScript.Extension/Console.hpp>
#include <XYO/QuantumScript.Extension/URL.hpp>

using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {
	Extension::Console::registerInternalExtension(executive);
	Extension::URL::registerInternalExtension(executive);
};

int main(int cmdN, char *cmdS[]) {
	if (ExecutiveX::initExecutive(cmdN, cmdS, initExecutive)) {
		if (!ExecutiveX::executeString(
		        "Script.requireExtension(\"Console\");"
		        "Script.requireExtension(\"URL\");"
		        "Console.writeLn(URL.getHostNameAndPort(\"http://example.com:80/\"));")) {
			printf("%s\n", (ExecutiveX::getError()).value());
			printf("%s", (ExecutiveX::getStackTrace()).value());
		};
		ExecutiveX::endProcessing();
	};
	return 0;
};
```

Registering only makes the extension *available*: scripts still call
`Script.requireExtension("URL")`. With the DLL build of the engine an
external `quantum-script--url.dll` found on the include path wins over the
internal one for `requireExtension`; use
`Script.requireInternalExtension("URL")` to force the internal one.

In the host's `fabricare.json`:

```json
{
	"name": "my-host",
	"make": "exe",
	"sourcePath": "XYO/MyHost",
	"dependency": [
		"quantum-script--url"
	]
}
```

A host built with the static CRT (`"crt": "static"`) depends on
`"quantum-script--url.static"` instead, as `fabricare` does. fabricare
resolves `quantum-script` and `quantum-script--console` transitively.

## 5. Static builds

The `quantum-script--url` project is `dll-or-lib`: on a static platform (for
example `win64-msvc-2026.static`, which sets `XYO_PLATFORM_COMPILE_STATIC`)
it builds a static library. The export macros become empty and the
`quantumScriptExtension` DLL entry point is left out (it is compiled only
with `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY`). The
`quantum-script--url.static` project and
`XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY` have the same effect for a static
library on a dynamic platform.

A static host must register the extension with `registerInternalExtension`
(section 4): external DLLs cannot be loaded into a host that does not use
the engine DLL.

## 6. Threads

Each thread that runs scripts has its own engine, so every thread loads the
extension itself with `Script.requireExtension("URL")`. The functions keep
no state: they are safe to call from any thread that loaded the extension.
