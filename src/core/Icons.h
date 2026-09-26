#pragma once
#include <QQuickImageProvider>
class Icons : public QQuickImageProvider {
public:
    Icons() : QQuickImageProvider(Pixmap) {}
    static void configureTheme();
    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requested) override;
};
