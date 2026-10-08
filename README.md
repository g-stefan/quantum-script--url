# Quantum Script Extension URL

Quantum Script extension
- Percent-encoding of URL components: `URL.encodeComponent` writes every
byte outside `A-Z a-z . ! ~ * ' ( )` as `%XX`, `URL.decodeComponent`
turns `%XX` escapes back into bytes.
- Splitting a URL into scheme, host and port, user info, path and query,
as written (not decoded), with `null` for absent parts.
- The URL layer of the `HTTP`, `OpenSSL` (HTTPS) and `SSHRemote`
extensions; built into `fabricare` and `magnet`.

```javascript
Script.requireExtension("URL");

URL;
URL.decodeComponent(str);
URL.encodeComponent(value);
URL.getSchemeName(url);
URL.getHostNameAndPort(url);
URL.getUsernameAndPassword(url);
URL.getPathAndFileName(url);
URL.getPathAndFileNameWithQuery(url);
URL.getQuery(url);
```

Built on `quantum-script`, part of the XYO C++ SDK.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, load from a script, hosts, register in a C++ host
- [Script API](docs/script-api.md) - every function: exact rules, edge cases, recipes
- [C++ API](docs/cpp-api.md) - registration, DLL entry point, static project, notes for maintainers
- [API reference](docs/reference.md)

A Claude Code skill for this extension is in
[.claude/skills/quantum-script--url](.claude/skills/quantum-script--url/SKILL.md).

## License

Copyright (c) 2016-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
