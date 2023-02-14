// Quantum Script Extension URL
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_URL_COPYRIGHT_HPP
#define XYO_QUANTUMSCRIPT_EXTENSION_URL_COPYRIGHT_HPP

#ifndef XYO_QUANTUMSCRIPT_EXTENSION_URL_DEPENDENCY_HPP
#	include <XYO/QuantumScript.Extension/URL/Dependency.hpp>
#endif

namespace XYO::QuantumScript::Extension::URL::Copyright {
	XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT const char *copyright();
	XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT const char *publisher();
	XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT const char *company();
	XYO_QUANTUMSCRIPT_EXTENSION_URL_EXPORT const char *contact();
};

#endif