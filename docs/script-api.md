# Script API

Everything the extension defines after `Script.requireExtension("URL")`.
`URL` is a plain object holding eight native functions; it has no
constructor and no state.

Every function takes one argument and converts it to a string with the
engine's `toString` first. A Quantum Script string is a sequence of bytes
(UTF-8 by convention), and all functions work on those bytes.

## Percent-encoding

### `URL.encodeComponent(value)`

Returns a `String` where every byte of `value` that is not in the kept set
is written as `%` followed by two **uppercase** hex digits. It never fails.

Kept as they are: `A-Z`, `a-z`, `.`, `!`, `~`, `*`, `'`, `(`, `)`.

Everything else is encoded, **including the digits `0-9` and `-` `_`**,
which JavaScript's `encodeURIComponent` keeps. The result is still a valid
URL component (every decoder accepts `%30` for `0`), it is just longer and
differs from what other tools produce.

```javascript
URL.encodeComponent("Hello");            // "Hello"
URL.encodeComponent("a b");              // "a%20b"     (space is %20, never "+")
URL.encodeComponent("a&b=c");            // "a%26b%3Dc"
URL.encodeComponent("/path?x#y");        // "%2Fpath%3Fx%23y"
URL.encodeComponent("2026");             // "%32%30%32%36"
URL.encodeComponent("my-file_1.txt");    // "my%2Dfile%5F%31.txt"
URL.encodeComponent(".!~*'()");          // ".!~*'()"
URL.encodeComponent("a\r\nb");           // "a%0D%0Ab"
URL.encodeComponent("");                 // ""
```

Argument conversion — anything that is not a string is encoded as its text
form:

| Argument | Encoded text | Result |
|----------|--------------|--------|
| number | decimal text | `URL.encodeComponent(255)` → `"%32%35%35"` |
| `true` / `null` | `"true"` / `"null"` | `"true"` / `"null"` |
| missing / `undefined` | `"undefined"` | `"undefined"` — no error |

Encode **one component at a time** (a query name, a query value, a path
segment), never a whole URL: `:`, `/`, `?`, `&`, `=` and `#` are encoded
too, so the result is no longer a URL.

### Non-ASCII text

Text is encoded byte by byte, so UTF-8 characters become one escape per
byte, as in every other URL encoder:

```javascript
URL.encodeComponent("ă");            // "%C4%83"
URL.encodeComponent("café €");       // "caf%C3%A9%20%E2%82%AC"
URL.decodeComponent("caf%C3%A9");    // "café"
```

Builds made before this was fixed wrote bytes `0x80`-`0xFF` as
`%FFFFFFXX` (`"ă"` → `"%FFFFFFC4%FFFFFF83"`), and the result did not
decode back. `fabricare` keeps its own copy of the extension, so its
scripts behave the old way until fabricare is rebuilt with the fixed copy.
`test/test.0002.js` checks the fixed behavior.

### Encoding like other tools

To produce the same output as JavaScript's `encodeURIComponent` or
RFC 3986 (digits, `-`, `_` kept as they are), encode the bytes yourself
with the `Buffer` extension:

```javascript
Script.requireExtension("Buffer");

function encodeComponentRFC3986(text) {
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
	return out;
};

encodeComponentRFC3986("café 2026");   // "caf%C3%A9%202026"   (URL.encodeComponent: "caf%C3%A9%20%32%30%32%36")
```

### `URL.decodeComponent(str)`

Returns a `String` where every `%XX` escape is replaced by the byte it
names. Hex digits may be uppercase or lowercase. All other characters are
copied unchanged.

```javascript
URL.decodeComponent("a%20b");            // "a b"
URL.decodeComponent("%2f%2F");           // "//"
URL.decodeComponent("%31%32");           // "12"
URL.decodeComponent("caf%C3%A9");        // "café"  (bytes kept as they are)
URL.decodeComponent("a%00b").length;     // 3       (zero byte kept)
URL.decodeComponent("a+b");              // "a+b"   ("+" is NOT a space)
URL.decodeComponent("");                 // ""
```

- **`+` is not decoded.** HTML forms (`application/x-www-form-urlencoded`)
  encode a space as `+`; replace it first:
  `URL.decodeComponent(value.replace("+", " "))` (`replace` replaces every
  occurrence).
- **Malformed escapes are not reported.** There is no error and no
  `undefined`; a `%` always consumes the two characters after it:

  | Input | Result | Why |
  |-------|--------|-----|
  | `"ab%"` | `"ab"` | `%` at the end is dropped |
  | `"ab%4"` | `"ab"` | `%` and the single character after it are dropped |
  | `"a%zzb"` | `"ab"` | not hex: both characters are dropped |
  | `"a%4gb"` | `"a\x04b"` | the leading hex digit is used, `g` is dropped |
  | `"%%41"` | `"1"` | `%4` is not a valid escape, `1` is copied |

  Validate input yourself when it matters, for example by checking that
  `URL.encodeComponent(URL.decodeComponent(t))` gives back an expected
  form, or by scanning for `%` not followed by two hex digits.
- The result is not checked for UTF-8: escapes of arbitrary bytes produce a
  string of raw bytes.
- Round trip: for any string `s`,
  `URL.decodeComponent(URL.encodeComponent(s)) == s`.

## Splitting a URL

All six `get*` functions follow the same steps on the text of the argument:

1. Find the **first** `://`. Not found → return **`null`**. The scheme is
   everything before it.
2. From just after `://`, find the **first** `/`. Everything between
   `://` and that `/` is the *authority*: `[user[:password]@]host[:port]`.
3. Cut the authority at its first `@` for user info and host; cut the part
   from the `/` on at `?` and `#` for path, query and fragment.

```
https://user:pass@example.com:8443/dir/file.html?q=1&x=2#top
\___/   \_______/ \______________/\____________/ \_____/ \_/
scheme   user info   host and port     path        query   fragment
```

Nothing is decoded, validated or normalized: parts are returned exactly as
written, `%XX` escapes and letter case included. A URL without `://`
(`mailto:x@y`, `example.com/a`, a relative path) gives `null` from every
`get*` function.

Results for typical inputs:

| URL | `getSchemeName` | `getHostNameAndPort` | `getUsernameAndPassword` | `getPathAndFileName` | `getPathAndFileNameWithQuery` | `getQuery` |
|-----|-----------------|----------------------|--------------------------|----------------------|-------------------------------|------------|
| `http://example.com:8080/a/f.html?x=1&y=2#top` | `"http"` | `"example.com:8080"` | `null` | `"/a/f.html"` | `"/a/f.html?x=1&y=2"` | `"x=1&y=2"` |
| `https://user:pass@example.com/p?q=a%20b` | `"https"` | `"example.com"` | `"user:pass"` | `"/p"` | `"/p?q=a%20b"` | `"q=a%20b"` |
| `http://example.com/` | `"http"` | `"example.com"` | `null` | `"/"` | `"/"` | `null` |
| `http://example.com/a?` | `"http"` | `"example.com"` | `null` | `"/a"` | `"/a?"` | `""` |
| `http://example.com/a#frag` | `"http"` | `"example.com"` | `null` | `"/a#frag"` | `"/a"` | `null` |
| `file:///C:/dir/f.txt` | `"file"` | `""` | `null` | `"/C:/dir/f.txt"` | `"/C:/dir/f.txt"` | `null` |
| **no `/` after the host:** | | | | | | |
| `http://example.com` | `"http"` | `"example.com"` | `null` | `"/"` | `null` | `null` |
| `http://user:pass@example.com` | `"http"` | `"user:pass@example.com"` | `null` | `"/"` | `null` | `null` |
| `http://example.com?x=1` | `"http"` | `"example.com?x=1"` | `null` | `"/"` | `null` | `null` |
| **no `://`:** | | | | | | |
| `mailto:someone@example.com`, `example.com/a`, `""` | `null` | `null` | `null` | `null` | `null` | `null` |

### `URL.getSchemeName(url)`

Everything before the first `://`, as written: `"https"`, `"ftp"`, `"file"`,
`"HTTP"` stays `"HTTP"`. Compare case-insensitively:
`URL.getSchemeName(url).toLowerCaseASCII() == "https"` — after checking for
`null`, because calling a method on `null` throws.

### `URL.getHostNameAndPort(url)`

The authority without its user info: `"example.com"`, `"example.com:8080"`,
`"[::1]:8080"`. The port is not separated or defaulted (see
[Split host and port](#split-host-and-port)).

When no `/` follows the authority, the function returns **everything after
`://`** without removing the user info, query or fragment
(`"user:pass@example.com"`, `"example.com?x=1"`). Normalize such URLs first
([recipe](#normalize-a-url-before-splitting-it)).

If the user info itself contains `@` (it should be encoded as `%40`), the
split happens at the first `@`.

### `URL.getUsernameAndPassword(url)`

The user info before the first `@` of the authority, still percent-encoded:
`"user:pass"`, `"user"`, `"user:p%40ss"`. **`null`** when there is no `@`,
and also when no `/` follows the authority. Split it with
`info.indexOf(":")` and decode both parts with `URL.decodeComponent`.

### `URL.getPathAndFileName(url)`

From the first `/` after the authority up to the first `?`, or to the end:
`"/dir/file.html"`. Returns `"/"` when the URL has no path.

The fragment is removed only together with a query: for
`http://example.com/a#frag` the result is `"/a#frag"`. Use
`getPathAndFileNameWithQuery` (which cuts at `#`) when a fragment may be
present and there is no query.

### `URL.getPathAndFileNameWithQuery(url)`

From the first `/` after the authority up to the first `#`, or to the end:
`"/dir/file.html?q=1"`. This is the *request target* of an HTTP request
line. Returns **`null`** when the URL has no `/` after the authority (unlike
`getPathAndFileName`, which returns `"/"`); use
`r = URL.getPathAndFileNameWithQuery(url); if (Script.isNull(r)) { r = "/"; };`.

### `URL.getQuery(url)`

The text after the first `?` that follows the path, up to the first `#`,
without the `?`: `"x=1&y=2"`. Not decoded. Returns:

- **`null`** when there is no `?` after the path, or no `/` after the
  authority (`http://example.com?x=1` → `null`);
- **`""`** for a bare `?` (`http://example.com/a?`) — falsy, so test with
  `Script.isNull`, not `!r`, when the difference matters.

A `?` inside the query is part of the value: `/a?x=1?y=2` → `"x=1?y=2"`.

## Recipes

`String.prototype.replace` replaces every occurrence, `indexOf` returns
`-1` when not found, and `substring(start, length)` takes a length, not an
end index.

### Build a query string

```javascript
function buildQuery(params) {
	var query = "";
	for (var name in params) {
		if (query != "") {
			query += "&";
		};
		query += URL.encodeComponent(name) + "=" + URL.encodeComponent(params[name]);
	};
	return query;
};

var url = "http://example.com/search?" + buildQuery({q: "hello world", page: 2, x: "a&b"});
// "http://example.com/search?q=hello%20world&page=%32&x=a%26b"
```

### Parse a query string into an object

```javascript
function parseQuery(query) {
	var result = {};
	if (Script.isNull(query) || query == "") {
		return result;
	};
	var pairs = query.split("&");
	for (var i = 0; i < pairs.length; ++i) {
		if (pairs[i] == "") {
			continue;
		};
		var index = pairs[i].indexOf("=");
		var name, value;
		if (index < 0) {
			name = pairs[i];
			value = "";
		} else {
			name = pairs[i].substring(0, index);
			value = pairs[i].substring(index + 1);
		};
		// "+" is a space in HTML form data; decodeComponent does not handle it
		result[URL.decodeComponent(name.replace("+", " "))] = URL.decodeComponent(value.replace("+", " "));
	};
	return result;
};

var params = parseQuery(URL.getQuery("http://h/s?q=hello+world&x=a%26b&flag"));
// {q: "hello world", x: "a&b", flag: ""}
```

In a CGI script the query comes from the environment:
`parseQuery(Shell.getenv("QUERY_STRING"))`. A repeated name keeps its last
value; collect into arrays if repeated names matter.

### Normalize a URL before splitting it

Most `get*` surprises come from URLs without a `/` after the host. Insert
one before the first `?` or `#`:

```javascript
function normalizeURL(url) {
	var scheme = URL.getSchemeName(url);
	if (Script.isNull(scheme)) {
		return url;
	};
	var start = scheme.length + 3;
	var stop = url.length;
	var marks = ["/", "?", "#"];
	for (var i = 0; i < marks.length; ++i) {
		var index = url.indexOf(marks[i], start);
		if (index >= 0 && index < stop) {
			stop = index;
		};
	};
	if (url.substring(stop, 1) == "/") {
		return url;
	};
	return url.substring(0, stop) + "/" + url.substring(stop);
};

normalizeURL("http://user:pass@example.com?x=1");   // "http://user:pass@example.com/?x=1"
// now getHostNameAndPort is "example.com", getUsernameAndPassword "user:pass", getQuery "x=1"
```

### Split host and port

```javascript
function splitHostAndPort(hostAndPort) {
	var index = hostAndPort.lastIndexOf(":");
	if (index < 0 || hostAndPort.indexOf("]", index) >= 0) {
		return {host: hostAndPort, port: ""};   // no port, or an IPv6 address without port
	};
	return {host: hostAndPort.substring(0, index), port: hostAndPort.substring(index + 1)};
};

splitHostAndPort("example.com:8443");   // {host: "example.com", port: "8443"}
splitHostAndPort("[::1]:8080");         // {host: "[::1]", port: "8080"}
splitHostAndPort("example.com");        // {host: "example.com", port: ""}
```

The port is a string and has no default: use `80` for `http`, `443` for
`https` when it is empty.

### Open a connection and send a request line

This is what the `HTTP` extension (`HTTP.json`, `HTTP.post`, ...) does
with a URL:

```javascript
var scheme = URL.getSchemeName(url);
if (Script.isNull(scheme) || scheme.toUpperCaseASCII() != "HTTP") {
	throw "not an http URL: " + url;
};
var host = URL.getHostNameAndPort(url);              // "example.com:8080"
if (host.indexOf(":") < 0) {
	host += ":80";
};
// socket.openClient(host) ...
var target = URL.getPathAndFileNameWithQuery(url);
if (Script.isNull(target)) {
	target = "/";
};
// socket.writeLn("GET " + target + " HTTP/1.0");
// socket.writeLn("Host: " + URL.getHostNameAndPort(url));
```

### File name of a download

```javascript
Script.requireExtension("Shell");

var fileName = Shell.getFileName(URL.getPathAndFileName(url));   // ".../v1.0/app.7z?x=1" -> "app.7z"
fileName = URL.decodeComponent(fileName);                          // "my%20file.txt" -> "my file.txt"
```

### Base URL of a server

```javascript
var server = URL.getSchemeName(url) + "://" + URL.getHostNameAndPort(url);
var api = server + "/api/v1/repos/" + owner + "/" + repo;
```

### Credentials

```javascript
var info = URL.getUsernameAndPassword(url);          // "user:p%40ss" or null
if (!Script.isNull(info)) {
	var index = info.indexOf(":");
	var user = URL.decodeComponent(index < 0 ? info : info.substring(0, index));
	var password = (index < 0) ? "" : URL.decodeComponent(info.substring(index + 1));
};
```

Do not log or print URLs that carry credentials.

## Related extensions

| Extension | Uses `URL` for |
|-----------|----------------|
| `HTTP` | `HTTP.json`, `post`, `postRequest`, `downloadFile` over plain `http://` (scheme, host and port, request line) |
| `OpenSSL` | the `HTTPS` object: the same over `https://` |
| `SSHRemote` | `ssh://user:password@host:port/` locations for `plink`: host, port, user and password, with host, user and password passed through `decodeComponent` |
| `Shell` | `Shell.getFileName` / `getFilePath` on the result of `getPathAndFileName` |
| `Buffer` | byte access for a custom encoder (see [Encoding like other tools](#encoding-like-other-tools)) |
