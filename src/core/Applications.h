#pragma once
#include <QObject>
#include <QVariantList>
class Applications:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList entries READ entries NOTIFY changed)
public:explicit Applications(QObject* parent=nullptr);QVariantList entries()const{return list;}
 Q_INVOKABLE void refresh();Q_INVOKABLE bool launch(QString id);
signals:void changed();void error(QString message);
private:QVariantList list;
};
