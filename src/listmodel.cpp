/*
 * Copyright (C) 2012 Cornelius Hald <cornelius.hald@kodira.de>
 *
 * This file is part of Pattern.
 *
 * Pattern is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pattern is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pattern. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QXmlStreamReader>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QStringList>

#include "pattern.h"
#include "network.h"
#include "listmodel.h"

ListModel::ListModel(QObject *parent) :
	bb::cascades::GroupDataModel(parent)
{
    m_results = 20;
    m_type = "patterns";
    m_category = "top";
    init();
}

ListModel::ListModel(int results, QString type, QString category, QObject *parent) :
    bb::cascades::GroupDataModel(parent), m_results(results), m_type(type), m_category(category)
{
    init();
}

void ListModel::init()
{
    m_page = 0;
    m_loading = false;
    m_started = false;

    setGrouping(bb::cascades::ItemGrouping::None);
    fixSorting();

    connect(this, SIGNAL(resultsChanged()), this, SLOT(loadData()));
    connect(this, SIGNAL(typeChanged()), this, SLOT(loadData()));
    connect(this, SIGNAL(categoryChanged()), this, SLOT(loadData()));
}

bool ListModel::loading()
{
	return m_loading;
}

void ListModel::setLoading(bool loading)
{
	if (m_loading != loading) {
		m_loading = loading;
		emit loadingChanged();
	}
}

int ListModel::results()
{
    return m_results;
}

void ListModel::setResults(int results)
{
    if (m_results != results) {
        m_results = results;
        m_page = 0;
		clear();
        emit resultsChanged();
    }
}

QString ListModel::type()
{
    return m_type;
}

void ListModel::setType(QString type)
{
    if (m_type != type) {
        m_type = type;
        m_page = 0;
		clear();
        emit typeChanged();
    }
}

QString ListModel::category()
{
    return m_category;
}

void ListModel::setCategory(QString category)
{
    if (m_category != category) {
        m_category = category;
        m_page = 0;

        clear();
        fixSorting();
        emit categoryChanged();
    }
}

void ListModel::fixSorting()
{
    if (m_category == "top") {
    	m_orderCol = "score";
    	setSortingKeys(QStringList() << "rank");
    	setSortedAscending(true);
    } else if (m_category == "new") {
    	m_orderCol = "dateCreated";
    	setSortingKeys(QStringList() << "id");
    	setSortedAscending(false);
    } else {
    	qDebug() << "WARN: Unknown category:" << m_category;
    }
}

void ListModel::loadData()
{
    if (!m_started) {
        qDebug() << "WARN: Not yet started. Doing nothing.";
        return;
    }
    
    // TODO: Cancel old request if sill there.

	setLoading(true);

	qDebug() << "INFO: Doing network request";
	QNetworkConfigurationManager m;
	qDebug() << "INFO: Are we online:" << m.isOnline();

    QString url = QString("http://www.colourlovers.com/api/%1/%2/?numberResults=%3&resultOffset=%4?orderCol=%5&sortBy=DESC")
            .arg(m_type)
            .arg(m_category)
            .arg(m_results)
            .arg(m_results * m_page)
            .arg(m_orderCol);
    
    qDebug() << "INFO: Request:" << url;

    QNetworkReply *reply = Network::manager()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
}

void ListModel::requestFinished()
{
	QNetworkReply *reply = (QNetworkReply*) sender();
    qDebug() << "INFO: Request finished";
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "ERROR: Network error";
        // TODO: Emit error and to show note on screen
        reply->deleteLater();
        return;
    }

    QByteArray xmlData = reply->readAll();
    parseXml(xmlData);
    setLoading(false);

    reply->deleteLater();
}

void ListModel::parseXml(QByteArray xmlData)
{
    QXmlStreamReader xml;
    xml.addData(xmlData);

    Pattern *pattern = 0;

    while (!xml.atEnd()) {
        if (xml.readNextStartElement()) {

            if (xml.name() == "pattern" || xml.name() == "color") {
                pattern = new Pattern(this);
                this->insert(pattern);
                continue;
            }

            if (xml.name() == "title") {
                pattern->setTitle(xml.readElementText());
                continue;
            }

            if (xml.name() == "userName") {
                pattern->setUserName(xml.readElementText());
                continue;
            }

            if (xml.name() == "imageUrl") {
                pattern->setPatternUrl(QUrl(xml.readElementText()));
                continue;
            }

            if (xml.name() == "rank") {
            	pattern->setRank(xml.readElementText().toInt());
            	continue;
            }

            if (xml.name() == "id") {
            	pattern->setId(xml.readElementText().toInt());
            	continue;
            }
        } else {
            // Use this to read through the whole xml even if there are no start elements anymore
            xml.readNext();
        }
    }

    if (xml.hasError()) {
        qDebug() << "ERROR: Error parsing XML:" << xml.errorString();
    }
}

void ListModel::loadNextPage()
{
    m_page++;
    loadData();
}

void ListModel::start()
{
    m_started = true;
    loadData();
}

int ListModel::length()
{
	return childCount(QVariantList());
}

