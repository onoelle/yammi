
#include "options.h"

/*
  dirty workaround until qt4 can be used ...
  */

QDebug& qDebug() {
    QDebug *a = new QDebug();
    return *a;
}

QDebug& qWarning() {
    QDebug *a = new QDebug();
    return *a;
}

QDebug& qError() {
    QDebug *a = new QDebug();
    return *a;
}

QDebug& qFatal() {
    QDebug *a = new QDebug();
    return *a;
}
