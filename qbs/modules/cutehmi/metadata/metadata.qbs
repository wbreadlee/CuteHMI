import qbs
import qbs.File
import qbs.FileInfo
import qbs.Process
import qbs.TextFile
import qbs.Utilities

/**
  This module collects properties defined within Qbs product and generates 'cutehmi.metadata.json' and 'cutehmi.metadata.hpp' artifacts.

  @warning generated artifact 'cutehmi.metadata.json' must not be added to the 'files' list of the product!
  Otherwise Qbs will not regenerate it automatically upon source 'qbs' modification.
  */
Module {
	additionalProductTypes: ["cutehmi.metadata.json", "cutehmi.metadata.hpp"]

//<workaround id="qbs-cutehmi-depends-1" target="Qbs" cause="design">
//	Parameter 'cutehmi.depends.reqMinor' can be specified within dependencies. It denotes required minor version of the dependency.
//	The build will stop if minor version requirement is not satisfied. This is similar to 'versionAtLeast' parameter
//	functionality provided by Qbs, with a difference, that if 'cutehmi.depends.reqMinor' parameter is set, then dependency will be
//	added to the 'dependencies' list of 'cutehmi.metadata.json' artifact.

	//Depends { name: "cutehmi.depends" }
//</workaround>

	FileTagger {
		patterns: ["*.qbs"]
		fileTags: ["qbs"]
	}

	Rule {
		multiplex: true
		inputs: ["qbs"]
		inputsFromDependencies: "qbs"

		prepare: {
			var productRequiredProperties = [
						'name',
						'cutehmiType',
						'major',
						'minor',
						'micro',
						'friendlyName',
						'vendor',
						'description',
						'author',
						'copyright',
						'license'
			]
			for (var prop in productRequiredProperties)
				if (product[productRequiredProperties[prop]] === undefined)
					console.warn("Undefined property 'product." + productRequiredProperties[prop] +"' in '" + product.name + "' (product directory: '" + product.sourceDirectory + "').")

			var jsonCmd = new JavaScriptCommand();
			jsonCmd.description = "generating " + product.sourceDirectory + "/cutehmi.metadata.json"
			jsonCmd.highlight = "codegen";
			jsonCmd.sourceCode = function() {
				console.info("Regenerating file " + product.sourceDirectory + "/cutehmi.metadata.json")

				var metadata = {
					"_comment": "This file has been autogenerated by Qbs cutehmi.metadata module.",
					"name": product.name,
					"cutehmiType": product.cutehmiType,
					"major": product.major,
					"minor": product.minor,
					"micro": product.micro,
					"friendlyName": product.friendlyName,
					"vendor": product.vendor,
					"description": product.description,
					"author": product.author,
					"copyright": product.copyright,
					"license": product.license,
					"dependencies": []
				}

				for (i in product.dependencies) {
					var dependency = product.dependencies[i]

//<workaround id="qbs-cutehmi-depends-1" target="Qbs" cause="design">
					var reqMinor = product[dependency.name].reqMinor
					// Instead of:
					//	var reqMinor = dependency.parameters.cutehmi.depends.reqMinor
//</workaround>

					if (reqMinor !== undefined) {
						var metadataDepencency = {
							"name": dependency.name,
							"reqMinor": reqMinor
						}
						metadata.dependencies.push(metadataDepencency)
						if (dependency.minor !== undefined)
							if (Number(dependency.minor) < reqMinor)
								throw "Product '" + product.name + "'"
											  + " requires dependency '" + dependency.name + "'"
											  + " to have minor version number at least '" + reqMinor + "'"
											  + ", while dependency has minor version number '" + Number(dependency.minor) + "'."
					}
				}

				var f = new TextFile(product.sourceDirectory + "/cutehmi.metadata.json", TextFile.WriteOnly);
				try {
					f.write(JSON.stringify(metadata))
				} finally {
					f.close()
				}
			}

			var hppCmd = new JavaScriptCommand();
			hppCmd.description = "generating " + product.sourceDirectory + "/cutehmi.metadata.hpp"
			hppCmd.highlight = "codegen";
			hppCmd.sourceCode = function() {
				console.info("Regenerating file " + product.sourceDirectory + "/cutehmi.metadata.hpp")

				var f = new TextFile(product.sourceDirectory + "/cutehmi.metadata.hpp", TextFile.WriteOnly);
				try {
					var shortPrefix = product.baseName.toUpperCase().replace(/\./g, '_')
					var prefix = product.name.toUpperCase().replace(/\./g, '_')

					f.writeLine("#ifndef " + prefix + "_METADATA_HPP")
					f.writeLine("#define " + prefix + "_METADATA_HPP")

					f.writeLine("")
					f.writeLine("// This file has been autogenerated by Qbs cutehmi.metadata module.")
					f.writeLine("")

					f.writeLine("#define " + shortPrefix + "_NAME \"" + product.name + "\"")
					f.writeLine("#define " + shortPrefix + "_CUTEHMI_TYPE \"" + product.cutehmiType + "\"")
					f.writeLine("#define " + shortPrefix + "_FRIENDLY_NAME \"" + product.friendlyName + "\"")
					f.writeLine("#define " + shortPrefix + "_VENDOR \"" + product.vendor + "\"")
					f.writeLine("#define " + shortPrefix + "_VERSION \"" + product.major + "." + product.minor + "." + product.micro + "\"")
					f.writeLine("#define " + shortPrefix + "_MAJOR " + product.major + "")
					f.writeLine("#define " + shortPrefix + "_MINOR " + product.minor + "")
					f.writeLine("#define " + shortPrefix + "_MICRO " + product.micro + "")

					f.writeLine("")
					f.writeLine("#ifdef " + shortPrefix + "_BUILD")
					f.writeLine("  #define " + prefix + "_" + product.minor)
					f.writeLine("#endif")

					for (var m = product.minor; m > 0; m--) {
						f.writeLine("")
						f.writeLine("#ifdef " + prefix + "_" + m)
						f.writeLine("  #ifndef " + prefix + "_CURRENT ")
						f.writeLine("    #define " + prefix + "_CURRENT " + m)
						f.writeLine("  #endif")
						f.writeLine("  #define " + prefix + "_" + (m - 1))
						f.writeLine("#endif")
					}

					f.writeLine("")
					f.writeLine("#ifndef " + prefix + "_CURRENT ")
					f.writeLine("  #define " + prefix + "_CURRENT 0")
					f.writeLine("#endif")

					f.writeLine("")
					f.writeLine("#endif")
				} finally {
					f.close()
				}
			}

			return [jsonCmd, hppCmd]
		}

		Artifact {
			filePath: product.sourceDirectory + "/cutehmi.metadata.json"
			fileTags: ["cutehmi.metadata.json", "qt.core.resource_data", "qt_plugin_metadata"]
		}

		Artifact {
			filePath: product.sourceDirectory + "/cutehmi.metadata.hpp"
			fileTags: ["cutehmi.metadata.hpp", "hpp"]
		}
	}

	Depends { name: "Qt.core" }
	Qt.core.resourcePrefix: product.name
}
﻿
//(c)MP: Copyright © 2019, Michal Policht <michpolicht@gmail.com>. All rights reserved.
//(c)MP: This file is a part of CuteHMI.
//(c)MP: CuteHMI is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//(c)MP: CuteHMI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
//(c)MP: You should have received a copy of the GNU Lesser General Public License along with CuteHMI.  If not, see <https://www.gnu.org/licenses/>.
