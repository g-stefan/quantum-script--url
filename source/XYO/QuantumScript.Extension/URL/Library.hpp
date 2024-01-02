// Quantum Script Extension URL
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY_HPP
#define XYO_QUANTUMSCRIPT_EXTENSION_URL_LIBRARY_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_URL_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/URL/Dependency.hpp>
#endif

namespace XYO::QuantumScript::Extension::URL {

	XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT void initExecutive(Executive *executive, void *extensionId);
	XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT void registerInternalExtension(Executive *executive);

};

#endif
