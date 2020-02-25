// Public domain
// http://unlicense.org/
Script.requireExtension("Console");
Script.requireExtension("URL");

var input = "~!@#$%^&*()=+[]{}\\;:'\",/?";
var output = "~!%40%23%24%25%5E%26*()%3D%2B%5B%5D%7B%7D%5C%3B%3A'%22%2C%2F%3F";
var result = URL.encodeComponent(input);
var resultOut = URL.decodeComponent(result);
if((input == resultOut) && (result == output)) {
	Console.writeLn("-> test 0001 ok");
} else {
	Console.writeLn("-> test 0001 fail");
};

