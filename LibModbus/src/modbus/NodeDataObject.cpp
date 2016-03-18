#include "NodeDataObject.hpp"

namespace cutehmi {
namespace modbus {

NodeDataObject::NodeDataObject(std::unique_ptr<Client> client, std::unique_ptr<ClientRunner> clientRunner, QObject * parent):
	QObject(parent),
	m_client(std::move(client)),
	m_clientRunner(std::move(clientRunner))
{
}

Client * NodeDataObject::client() const
{
	return m_client.get();
}

ClientRunner * NodeDataObject::clientRunner() const
{
	return m_clientRunner.get();
}

}
}

//(c)MP: Copyright © 2016, Michal Policht. All rights reserved.
//(c)MP: This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
