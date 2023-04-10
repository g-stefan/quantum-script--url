// Quantum Script Extension URL
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/QuantumScript.Extension/URL/Library.hpp>
#include <XYO/QuantumScript.Extension/URL/Copyright.hpp>
#include <XYO/QuantumScript.Extension/URL/License.hpp>
#include <XYO/QuantumScript.Extension/URL/Version.hpp>

namespace XYO::QuantumScript::Extension::URL {

	static TPointer<Variable> decodeComponent(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-decode-component\n");
#endif
		String in = (arguments->index(0))->toString();
		String out;
		size_t k;
		int value;
		char buf[3];
		buf[2] = 0;
		for (k = 0; k < in.length(); ++k) {
			if (in[k] == '%') {
				if (k + 1 < in.length()) {
					++k;
					buf[0] = in[k];
					if (k + 1 < in.length()) {
						++k;
						buf[1] = in[k];
						if (sscanf(buf, "%02X", &value) == 1) {
							out << (char)value;
						};
					};
				};
				continue;
			};
			out << in[k];
		};
		return VariableString::newVariable(out);
	};

	static TPointer<Variable> encodeComponent(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-encode-component\n");
#endif

		String in = (arguments->index(0))->toString();
		String out;
		size_t k;
		int value;
		char buf[4];
		for (k = 0; k < in.length(); ++k) {
			if (
			    (in[k] >= 'A' && in[k] <= 'Z') || (in[k] >= 'a' && in[k] <= 'z') || (in[k] == '.') || (in[k] == '!') || (in[k] == '~') || (in[k] == '*') || (in[k] == '\'') || (in[k] == '(') || (in[k] == ')')) {
				out << in[k];
				continue;
			};
			out << '%';
			sprintf(buf, "%02X", in[k]);
			out << buf;
		};
		return VariableString::newVariable(out);
	};

	static TPointer<Variable> getSchemeName(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-get-scheme-name\n");
#endif

		String url = (arguments->index(0))->toString();
		size_t index;
		if (url.indexOf("://", 0, index)) {
			return VariableString::newVariable(url.substring(0, index));
		};
		return VariableNull::newVariable();
	};

	static TPointer<Variable> getHostNameAndPort(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-get-host-name-and-port\n");
#endif

		String url = (arguments->index(0))->toString();
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		if (url.indexOf("://", 0, index)) {
			if (url.indexOf("/", index + 3, part)) {
				if (url.substring(index + 3, part - (index + 3)).split2("@", firstPart, secondPart)) {
					return VariableString::newVariable(secondPart);
				};
				return VariableString::newVariable(url.substring(index + 3, part - (index + 3)));
			};
			return VariableString::newVariable(url.substring(index + 3));
		};
		return VariableNull::newVariable();
	};

	static TPointer<Variable> getUsernameAndPassword(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-get-username-and-password\n");
#endif

		String url = (arguments->index(0))->toString();
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		if (url.indexOf("://", 0, index)) {
			if (url.indexOf("/", index + 3, part)) {
				if (url.substring(index + 3, part - (index + 3)).split2("@", firstPart, secondPart)) {
					return VariableString::newVariable(firstPart);
				};
			};
		};
		return VariableNull::newVariable();
	};

	static TPointer<Variable> getPathAndFileName(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-get-path-and-file-name\n");
#endif

		String url = (arguments->index(0))->toString();
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		if (url.indexOf("://", 0, index)) {
			if (url.indexOf("/", index + 3, part)) {
				if (url.substring(part).split2("?", firstPart, secondPart)) {
					return VariableString::newVariable(firstPart);
				};
				return VariableString::newVariable(url.substring(part));
			};
			return VariableString::newVariable("/");
		};
		return VariableNull::newVariable();
	};

	static TPointer<Variable> getQuery(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-get-query\n");
#endif

		String url = (arguments->index(0))->toString();
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		String thirdPart;
		if (url.indexOf("://", 0, index)) {
			if (url.indexOf("/", index + 3, part)) {
				if (url.substring(part + 1).split2("?", firstPart, secondPart)) {
					if (secondPart.split2("#", firstPart, thirdPart)) {
						return VariableString::newVariable(firstPart);
					};
					return VariableString::newVariable(secondPart);
				};
			};
		};
		return VariableNull::newVariable();
	};

	static TPointer<Variable> getPathAndFileNameWithQuery(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
		printf("- url-get-path-and-file-name-with-query\n");
#endif

		String url = (arguments->index(0))->toString();
		size_t index;
		size_t part;
		String firstPart;
		String secondPart;
		if (url.indexOf("://", 0, index)) {
			if (url.indexOf("/", index + 3, part)) {
				if (url.substring(part).split2("#", firstPart, secondPart)) {
					return VariableString::newVariable(firstPart);
				};
				return VariableString::newVariable(url.substring(part));
			};
		};
		return VariableNull::newVariable();
	};

	void registerInternalExtension(Executive *executive) {
		executive->registerInternalExtension("URL", initExecutive);
	};

	void initExecutive(Executive *executive, void *extensionId) {
		String info = "URL\r\n";
		info << License::shortLicense().c_str();

		executive->setExtensionName(extensionId, "URL");
		executive->setExtensionInfo(extensionId, info);
		executive->setExtensionVersion(extensionId, Extension::URL::Version::versionWithBuild());
		executive->setExtensionPublic(extensionId, true);

		executive->compileStringX("var URL={};");
		executive->setFunction2("URL.decodeComponent(str)", decodeComponent);
		executive->setFunction2("URL.encodeComponent(value)", encodeComponent);
		executive->setFunction2("URL.getSchemeName(url)", getSchemeName);
		executive->setFunction2("URL.getHostNameAndPort(url)", getHostNameAndPort);
		executive->setFunction2("URL.getUsernameAndPassword(url)", getUsernameAndPassword);
		executive->setFunction2("URL.getPathAndFileName(url)", getPathAndFileName);
		executive->setFunction2("URL.getPathAndFileNameWithQuery(url)", getPathAndFileNameWithQuery);
		executive->setFunction2("URL.getQuery(url)", getQuery);
	};

};

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
extern "C" XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT void quantumScriptExtension(XYO::QuantumScript::Executive *executive, void *extensionId) {
	XYO::QuantumScript::Extension::URL::initExecutive(executive, extensionId);
};
#endif
