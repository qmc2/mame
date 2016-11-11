// license:BSD-3-Clause
// copyright-holders:MAMEdev Team

#ifndef ROLEITEMMODEL_H
#define ROLEITEMMODEL_H

#include <QStandardItemModel>

class RoleItemModel : public QStandardItemModel
{
	Q_OBJECT

	public:
		explicit RoleItemModel(const QHash<int, QByteArray> &roleNames);
		virtual QHash<int, QByteArray> roleNames() const { return m_roleNames; }

	private:
		QHash<int, QByteArray> m_roleNames;
};

#endif
