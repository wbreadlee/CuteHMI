#ifndef CUTEHMI_APPFULL_SRC_SETTINGS_HPP
#define CUTEHMI_APPFULL_SRC_SETTINGS_HPP

#include <QSettings>

namespace cutehmi {

class Settings:
	public QSettings
{
	public:
		Settings();
};

}

#endif

//(c)MP: Copyright © 2016, Michal Policht. All rights reserved.
//(c)MP: This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
