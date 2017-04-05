#include "../../include/modbus/internal/functions.hpp"
#include "../../include/modbus/Client.hpp"
#include "../../include/modbus/Exception.hpp"

#include <QtDebug>
#include <QMutexLocker>
#include <QtConcurrent>

namespace cutehmi {
namespace modbus {

QString Client::Error::str() const
{
	switch (code()) {
		case Error::UNABLE_TO_CONNECT:
			return tr("Unable to connect.");
		default:
			return base::Error::str();
	}
}

Client::Client(std::unique_ptr<internal::AbstractConnection> connection, QObject * parent):
	AbstractDevice(parent),
	m(new Members(this, std::move(connection)))
{
	QObject::connect(m->rValueRequestMapper, SIGNAL(mapped(int)), this, SLOT(rValueRequest(int)));
	QObject::connect(m->bValueRequestMapper, SIGNAL(mapped(int)), this, SLOT(bValueRequest(int)));
}

Client::~Client()
{
	QThreadPool::globalInstance()->waitForDone();

	for (IrDataContainer::KeysContainer::const_iterator it = m->irData.keys().begin(); it != m->irData.keys().end(); ++it)
		delete m->irData.at(*it);
	m->irData.clear();
	for (RDataContainer::KeysContainer::const_iterator it = m->rData.keys().begin(); it != m->rData.keys().end(); ++it)
		delete m->rData.at(*it);
	m->rData.clear();
	for (IbDataContainer::KeysContainer::const_iterator it = m->ibData.keys().begin(); it != m->ibData.keys().end(); ++it)
		delete m->ibData.at(*it);
	m->ibData.clear();
	for (BDataContainer::KeysContainer::const_iterator it = m->bData.keys().begin(); it != m->bData.keys().end(); ++it)
		delete m->bData.at(*it);
	m->bData.clear();
}

const QQmlListProperty<InputRegister> & Client::ir()
{
	return m->ir;
}

const QQmlListProperty<HoldingRegister> & Client::r()
{
	return m->r;
}

const QQmlListProperty<DiscreteInput> & Client::ib()
{
	return m->ib;
}

const QQmlListProperty<Coil> & Client::b()
{
	return m->b;
}

InputRegister * Client::irAt(int index)
{
	return IrAt(& m->ir, index);
}

HoldingRegister * Client::rAt(int index)
{
	return RAt(& m->r, index);
}

DiscreteInput * Client::ibAt(int index)
{
	return IbAt(& m->ib, index);
}

Coil * Client::bAt(int index)
{
	return BAt(& m->b, index);
}

void Client::readIr(int addr)
{
	static const int NUM_READ = 1;

	QMutexLocker locker(& m->irMutex);
	IrDataContainer::iterator it = m->irData.find(addr);
	Q_ASSERT_X(it != m->irData.end(), __func__, "register has not been referenced yet");
	uint16_t val;
	CUTEHMI_MODBUS_QDEBUG("Reading value from input register '" << addr << "'.");
	if (m->connection->readIr(addr, NUM_READ, & val) != NUM_READ)
		CUTEHMI_MODBUS_QWARNING("Failed reading input register value from the device.");
	else
		(*it)->updateValue(val);
}

void Client::readR(int addr)
{
	static const int NUM_READ = 1;

	QMutexLocker locker(& m->rMutex);
	RDataContainer::iterator it = m->rData.find(addr);
	Q_ASSERT_X(it != m->rData.end(), __func__, "register has not been referenced yet");
	uint16_t val;
	CUTEHMI_MODBUS_QDEBUG("Reading value from holding register '" << addr << "'.");
	if (m->connection->readR(addr, NUM_READ, & val) != NUM_READ)
		CUTEHMI_MODBUS_QWARNING("Failed reading register value from the device.");
	else
		(*it)->updateValue(val);
}

void Client::writeR(int addr)
{
	QMutexLocker locker(& m->rMutex);
	RDataContainer::iterator it = m->rData.find(addr);
	Q_ASSERT_X(it != m->rData.end(), __func__, "register has not been referenced yet");
	uint16_t val = (*it)->requestedValue();
	CUTEHMI_MODBUS_QDEBUG("Writing requested value '" << val << "' to holding register '" << addr << "'.");
	if (m->connection->writeR(addr, val) != 1)
		CUTEHMI_MODBUS_QWARNING("Failed to write register value to the device.");
	else
		emit (*it)->valueWritten();
}

void Client::readIb(int addr)
{
	static const int NUM_READ = 1;

	QMutexLocker locker(& m->ibMutex);
	IbDataContainer::iterator it = m->ibData.find(addr);
	Q_ASSERT_X(it != m->ibData.end(), __func__, "discrete input has not been referenced yet");
	bool val = 0;
	CUTEHMI_MODBUS_QDEBUG("Reading value from discrete input '" << addr << "'.");
	if (m->connection->readIb(addr, NUM_READ, & val) != NUM_READ)
		CUTEHMI_MODBUS_QWARNING("Failed reading discrete input value from the device.");
	else
		(*it)->updateValue(val);
}

void Client::readB(int addr)
{
	static const int NUM_READ = 1;

	QMutexLocker locker(& m->bMutex);
	BDataContainer::iterator it = m->bData.find(addr);
	Q_ASSERT_X(it != m->bData.end(), __func__, "coil has not been referenced yet");
	bool val = 0;
	CUTEHMI_MODBUS_QDEBUG("Reading value from coil '" << addr << "'.");
	if (m->connection->readB(addr, NUM_READ, & val) != NUM_READ)
		CUTEHMI_MODBUS_QWARNING("Failed reading coil value from the device.");
	else
		(*it)->updateValue(val);
}

void Client::writeB(int addr)
{
	QMutexLocker locker(& m->bMutex);
	BDataContainer::iterator it = m->bData.find(addr);
	Q_ASSERT_X(it != m->bData.end(), __func__, "coil has not been referenced yet");
	bool val = (*it)->requestedValue();
	CUTEHMI_MODBUS_QDEBUG("Writing requested value '" << val << "' to coil '" << addr << "'.");
	if (m->connection->writeB(addr, val) != 1)
		CUTEHMI_MODBUS_QWARNING("Failed to write coil value to the device.");
	else
		emit (*it)->valueWritten();
}

void Client::connect()
{
	if (m->connection->connect())
		emit connected();
	else
		emit error(base::errorInfo(Error(Error::UNABLE_TO_CONNECT)));
}

void Client::disconnect()
{
	m->connection->disconnect();
	emit disconnected();
}

void Client::readAll(const QAtomicInt & run)
{
	readRegisters<IrDataContainer>(m->irData, & Client::readIr, run);
	readRegisters<RDataContainer>(m->rData, & Client::readR, run);
	readRegisters<IbDataContainer>(m->ibData, & Client::readIb, run);
	readRegisters<BDataContainer>(m->bData, & Client::readB, run);
}

void Client::rValueRequest(int index)
{
	QtConcurrent::run([](Client * me, int index) {me->writeR(index); me->readR(index);}, this, index);
}

void Client::bValueRequest(int index)
{
	QtConcurrent::run([](Client * me, int index) {me->writeB(index); me->readB(index);}, this, index);
}

HoldingRegister * Client::RAt(QQmlListProperty<HoldingRegister> * property, int index)
{
	// Can convert lambda to function pointer if lambda captures nothing according to standard
	// (via stackoverflow -> http://stackoverflow.com/questions/28746744/passing-lambda-as-function-pointer).
	// "The closure type for a lambda-expression with no lambda-capture has a public non-virtual non-explicit const conversion function
	//  to pointer to function having the same parameter and return types as the closure type’s function call operator. The value
	//  returned by this conversion function shall be the address of a function that, when invoked, has the same effect as invoking the
	//  closure type’s function call operator." -- draft C++11 standard section 5.1.2 [expr.prim.lambda]
	auto onCreate = [](QQmlListProperty<HoldingRegister> * property, int index, HoldingRegister * reg) {
		Client * client = static_cast<Client *>(property->object);
		QSignalMapper * mapper = client->m->rValueRequestMapper;
		mapper->setMapping(reg, index);
		QObject::connect(reg, SIGNAL(valueRequested()), mapper, SLOT(map()));
	};
	HoldingRegister * reg = At<HoldingRegister>(property, index, onCreate);

	return reg;
}

InputRegister * Client::IrAt(QQmlListProperty<InputRegister> * property, int index)
{
	InputRegister * reg = At<InputRegister>(property, index);
	return reg;
}

Coil * Client::BAt(QQmlListProperty<Coil> * property, int index)
{
	// Can convert lambda to function pointer if lambda captures nothing according to standard
	// (via stackoverflow -> http://stackoverflow.com/questions/28746744/passing-lambda-as-function-pointer).
	// "The closure type for a lambda-expression with no lambda-capture has a public non-virtual non-explicit const conversion function
	//  to pointer to function having the same parameter and return types as the closure type’s function call operator. The value
	//  returned by this conversion function shall be the address of a function that, when invoked, has the same effect as invoking the
	//  closure type’s function call operator." -- draft C++11 standard section 5.1.2 [expr.prim.lambda]
	auto onCreate = [](QQmlListProperty<Coil> * property, int index, Coil * reg) {
		Client * client = static_cast<Client *>(property->object);
		QSignalMapper * mapper = client->m->bValueRequestMapper;
		mapper->setMapping(reg, index);
		QObject::connect(reg, SIGNAL(valueRequested()), mapper, SLOT(map()));
	};
	Coil * reg = At<Coil>(property, index, onCreate);

	return reg;
}

DiscreteInput * Client::IbAt(QQmlListProperty<DiscreteInput> * property, int index)
{
	DiscreteInput * reg = At<DiscreteInput>(property, index);
	return reg;
}

}
}

//(c)MP: Copyright © 2017, Michal Policht. All rights reserved.
//(c)MP: This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
