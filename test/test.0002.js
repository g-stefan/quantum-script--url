// Public domain
// http://unlicense.org/
Script.requireExtension("Console");
Script.requireExtension("URL");

// UTF-8 bytes 0x80-0xFF must be encoded as two hex digits
var input = "caf\u00e9 \u0103\u20ac";
var output = "caf%C3%A9%20%C4%83%E2%82%AC";
var result = URL.encodeComponent(input);
var resultOut = URL.decodeComponent(result);
if((input == resultOut) && (result == output)) {
	Console.writeLn("-> test 0002 ok");
} else {
	Console.writeLn("-> test 0002 fail");
};
