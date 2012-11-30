#include <QDebug>
#include <QImage>
#include <QFile>
#include <QByteArray>

#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>
#include <bb/system/InvokeReply>
#include <unistd.h>

#include "network.h"
#include "pattern.h"


Pattern::Pattern(QObject *parent) :
    QObject(parent)
{
}

QString Pattern::title()
{
    return m_title;
}

void Pattern::setTitle(QString title)
{
    if (m_title != title ) {
        m_title = title;
        emit titleChanged();
    }
}

QString Pattern::userName()
{
    return m_userName;
}

void Pattern::setUserName(QString userName)
{
    if (m_userName != userName) {
        m_userName = userName;
        emit userNameChanged();
    }
}

QUrl Pattern::patternUrl()
{
    return m_imageUrl;
}

void Pattern::setPatternUrl(QUrl imageUrl)
{
    if (m_imageUrl != imageUrl) {
        m_imageUrl = imageUrl;
        emit patternUrlChanged();
    }
}


