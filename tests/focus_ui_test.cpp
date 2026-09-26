#include <QtTest>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTemporaryDir>
#include "core/NotificationPreferences.h"
#include "core/Preferences.h"
class FocusUiTest : public QObject {
    Q_OBJECT
private slots:
    void editorSavesRealProfile() {
        QTemporaryDir tmp;
        Preferences prefs(tmp.filePath("ui.ini"));
        NotificationPreferences policy(tmp.filePath("focus.ini"));
        QQuickView view;
        view.rootContext()->setContextProperty("Prefs", &prefs);
        view.rootContext()->setContextProperty("NotificationPrefs", &policy);
        QStringList errors;
        connect(view.engine(), &QQmlEngine::warnings, this, [&](const QList<QQmlError>& list) {
            for (auto e : list)
                errors << e.toString();
        });
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(740, 1000);
        view.setSource(QUrl("qrc:/qml/FocusSection.qml"));
        QCOMPARE(view.status(), QQuickView::Ready);
        view.show();
        QVERIFY(
            QMetaObject::invokeMethod(view.rootObject(), "edit", Q_ARG(QVariant, QVariant(QVariantMap{}))));
        QTest::qWait(50);
        auto name = view.rootObject()->findChild<QObject*>("focus-profile-name");
        QVERIFY(name);
        name->setProperty("text", "Work");
        auto save = view.rootObject()->findChild<QObject*>("save-focus-profile");
        QVERIFY(save);
        QVERIFY(QMetaObject::invokeMethod(save, "clicked"));
        QTRY_COMPARE(policy.profiles().size(), 1);
        QCOMPARE(policy.profiles()[0].toMap()["name"].toString(), QString("Work"));
        QCOMPARE(policy.profiles()[0].toMap()["days"].toList().size(), 5);
        QVERIFY2(errors.isEmpty(), qPrintable(errors.join('\n')));
    }
};
QTEST_MAIN(FocusUiTest)
#include "focus_ui_test.moc"
