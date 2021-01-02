//
// Quantum Script Extension URL
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_EXTENSION_URL_HPP
#define QUANTUM_SCRIPT_EXTENSION_URL_HPP

#ifndef QUANTUM_SCRIPT_HPP
#include "quantum-script.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_URL__EXPORT_HPP
#include "quantum-script-extension-url--export.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_URL_COPYRIGHT_HPP
#include "quantum-script-extension-url-copyright.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_URL_LICENSE_HPP
#include "quantum-script-extension-url-license.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_URL_VERSION_HPP
#include "quantum-script-extension-url-version.hpp"
#endif

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace URL {

				using namespace Quantum::Script;

				QUANTUM_SCRIPT_EXTENSION_URL_EXPORT void initExecutive(Executive *executive, void *extensionId);
				QUANTUM_SCRIPT_EXTENSION_URL_EXPORT void registerInternalExtension(Executive *executive);

			};
		};
	};
};

#endif

