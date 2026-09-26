#include "core/Icons.h"
#include <QGuiApplication>
#include <QQuickView>
#include <QQuickStyle>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QScreen>
#include <QIcon>
#include <QPalette>
#include <QFile>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDBusConnection>
#include <QTextStream>
#include <LayerShellQt/Window>
#include <KWindowEffects>
#include "core/Applications.h"
#include "core/Notifications.h"
#include "core/GlobalMenu.h"
#include "core/Tray.h"
#include "core/Accounts.h"
#include "core/Files.h"
#include "core/Translation.h"
#include "core/Preferences.h"
#include "core/Region.h"
#include "core/DateTimeSettings.h"
#include "core/StorageSettings.h"
#include "core/PointerSettings.h"
#include "core/DefaultApps.h"
#include "core/Printers.h"
#include "core/NotificationPreferences.h"
#include "core/UserSettings.h"
#include "core/AccessibilitySettings.h"
#include "core/NetworkSettings.h"
#include "core/DisplaySettings.h"
#include "core/AudioStreams.h"
#include "core/PowerSettings.h"
#include "core/SoftwareUpdate.h"
#include "core/PrinterDrivers.h"
#include "core/ApplicationStorage.h"
#include "core/GestureSettings.h"
#include "core/SystemServices.h"
#include "kwin/WindowModel.h"
#include "Controller.h"
#include <QDBusMessage>
#include <QDBusPendingCall>
#include "core/FilesMenu.h"
#include "core/Keyboard.h"
#include "kwin/WindowMenu.h"
#include <QQuickItem>
int main(int argc,char** argv){
 QQuickStyle::setStyle("Basic");
 QQuickWindow::setDefaultAlphaBuffer(true);
 QGuiApplication app(argc,argv);Icons::configureTheme();app.setApplicationName("Harbor");app.setOrganizationName("Harbor");app.setFont(QFont("Noto Sans",10));app.setDesktopFileName("org.harbor.Shell");app.setQuitOnLastWindowClosed(false);
 const auto args=app.arguments();bool preview=args.contains("--preview"),settings=args.contains("--settings"),filesMode=args.contains("--files"),controlMode=args.contains("--control");
 QString controlPage="ControlCenter";if(controlMode){auto requested=args.value(args.indexOf("--control")+1);if(QStringList{"HarborMenu","BluetoothMenu","RecentItems"}.contains(requested))controlPage=requested;}
 Files browser; if(filesMode)app.setDesktopFileName("org.harbor.Files");if(filesMode||preview){int n=args.indexOf(filesMode?"--files":"--preview");if(n+1<args.size()&&!args[n+1].startsWith("--"))browser.navigate(args[n+1]);}
 Preferences preferences;Keyboard keyboard;Region region;
 DateTimeSettings dateTimeSettings;StorageSettings storageSettings;PointerSettings pointerSettings;DefaultApps defaultApps;Printers printers;
 NotificationPreferences notificationPreferences;UserSettings userSettings;AccessibilitySettings accessibilitySettings;NetworkSettings networkSettings;DisplaySettings displaySettings;AudioStreams audioStreams;PowerSettings powerSettings;SoftwareUpdate softwareUpdate;PrinterDrivers printerDrivers;ApplicationStorage applicationStorage;GestureSettings gestureSettings;
 auto updatePalette=[&]{QPalette p;bool d=preferences.dark();p.setColor(QPalette::Window,d?QColor("#252528"):QColor("#f6f6f8"));p.setColor(QPalette::WindowText,d?QColor("#eeeeef"):QColor("#26262a"));p.setColor(QPalette::Text,p.color(QPalette::WindowText));p.setColor(QPalette::ButtonText,p.color(QPalette::WindowText));p.setColor(QPalette::Base,d?QColor("#333337"):Qt::white);p.setColor(QPalette::Button,d?QColor("#3d3d43"):QColor("#ededf1"));p.setColor(QPalette::Highlight,QColor(preferences.accent()));app.setPalette(p);};updatePalette();QObject::connect(&preferences,&Preferences::changed,&app,updatePalette);
 Applications applications;SystemServices services;WindowModel windows;Controller controller;Notifications notifications(!preview&&!settings&&!filesMode&&!controlMode);Tray tray(!preview&&!settings&&!filesMode&&!controlMode);Accounts accounts;QTimer::singleShot(0,&accounts,&Accounts::refresh);
 QObject::connect(&windows,&WindowModel::changed,&applications,[&]{applications.setWindows(windows.windows());});
 QObject::connect(&applications,&Applications::activateRequested,&windows,&WindowModel::activate);
 QObject::connect(&applications,&Applications::error,&notifications,[&](QString error){notifications.Notify("Harbor",0,"dialog-error",QObject::tr("Application launch"),error,{}, {},10000);});
 GlobalMenu menu;QObject::connect(&menu,&GlobalMenu::activated,&controller,&Controller::dismiss);QObject::connect(&windows,&WindowModel::changed,&menu,[&]{if(!preview)menu.setSource(windows.menuService(),windows.menuPath());});
 Translation translation;translation.arabic=preferences.language()=="ar";app.installTranslator(&translation);app.setLayoutDirection(translation.arabic?Qt::RightToLeft:Qt::LeftToRight);
 QQmlEngine engine;QObject::connect(&preferences,&Preferences::changed,&engine,[&]{bool ar=preferences.language()=="ar";if(ar!=translation.arabic){translation.arabic=ar;app.setLayoutDirection(ar?Qt::RightToLeft:Qt::LeftToRight);engine.retranslate();}});
 engine.addImageProvider("icons",new Icons);
 auto ctx=engine.rootContext();ctx->setContextProperty("ApplicationStorage",&applicationStorage);ctx->setContextProperty("PrinterDrivers",&printerDrivers);ctx->setContextProperty("GestureSettings",&gestureSettings);ctx->setContextProperty("NotificationPrefs",&notificationPreferences);ctx->setContextProperty("UserSettings",&userSettings);ctx->setContextProperty("AccessibilitySettings",&accessibilitySettings);ctx->setContextProperty("NetworkSettings",&networkSettings);ctx->setContextProperty("DisplaySettings",&displaySettings);ctx->setContextProperty("AudioStreams",&audioStreams);ctx->setContextProperty("PowerSettings",&powerSettings);ctx->setContextProperty("SoftwareUpdate",&softwareUpdate);ctx->setContextProperty("DateTimeSettings",&dateTimeSettings);ctx->setContextProperty("StorageSettings",&storageSettings);ctx->setContextProperty("PointerSettings",&pointerSettings);ctx->setContextProperty("DefaultApps",&defaultApps);ctx->setContextProperty("Printers",&printers);ctx->setContextProperty("Region",&region);ctx->setContextProperty("Keyboard",&keyboard);ctx->setContextProperty("HarborVersion",QStringLiteral(HARBOR_VERSION));ctx->setContextProperty("Browser",&browser);ctx->setContextProperty("Notifications",&notifications);ctx->setContextProperty("GlobalMenu",&menu);ctx->setContextProperty("Tray",&tray);ctx->setContextProperty("Accounts",&accounts);ctx->setContextProperty("Prefs",&preferences);ctx->setContextProperty("Apps",&applications);ctx->setContextProperty("System",&services);ctx->setContextProperty("Windows",&windows);ctx->setContextProperty("UI",&controller);
controller.onList=[&]{return QString::fromUtf8(QJsonDocument(QJsonArray::fromVariantList(windows.windows())).toJson(QJsonDocument::Compact));};controller.onActivate=[&](QString id){windows.activate(id);};controller.onWindowClose=[&](QString id){windows.close(id);};
 if(!preview&&!settings&&!filesMode&&!controlMode){auto bus=QDBusConnection::sessionBus();if(!bus.registerService("org.harbor.Shell"))return 4;bus.registerObject("/Shell",&controller,QDBusConnection::ExportAllSlots);}
 if(!preview&&!settings&&!filesMode&&!controlMode){QTimer::singleShot(800,&accessibilitySettings,&AccessibilitySettings::restore);QTimer::singleShot(1200,&gestureSettings,&GestureSettings::restore);}
 QList<QQuickView*> surfaces;QQuickView* popup=nullptr;
 auto make=[&](QString name,QScreen* screen,int width,int height,int role)->QQuickView*{
  auto view=new QQuickView(&engine,nullptr);view->setResizeMode(QQuickView::SizeRootObjectToView);view->setColor(Qt::transparent);view->setScreen(screen);view->resize(width,height);if(name=="Settings"||name=="Files"){view->setFlags(Qt::FramelessWindowHint);view->setMinimumSize(QSize(740,560));}
  if(name=="Files")view->setMinimumSize(QSize(1000,600));
  // Keep the initial geometry on the target output: Qt may otherwise
  // select the primary output when creating the native Wayland surface.
  view->setPosition(screen->geometry().topLeft());
  view->setTitle("Harbor — "+name);view->setProperty("harborScreen",QVariant::fromValue(screen));
  if(role>=0&&!preview&&app.platformName()=="wayland"){
   view->setFlags(Qt::FramelessWindowHint);auto layer=LayerShellQt::Window::get(view);layer->setScope("harbor-"+name.toLower());
   using W=LayerShellQt::Window;
   if(role==0){layer->setLayer(W::LayerBackground);layer->setAnchors(W::Anchors(W::AnchorTop)|W::AnchorBottom|W::AnchorLeft|W::AnchorRight);layer->setExclusiveZone(-1);}
   if(role==1){layer->setLayer(W::LayerTop);layer->setAnchors(W::Anchors(W::AnchorTop)|W::AnchorLeft|W::AnchorRight);layer->setExclusiveZone(height);}
   if(role==2){layer->setLayer(W::LayerTop);layer->setAnchors(W::AnchorBottom);layer->setMargins(QMargins(0,0,0,10));layer->setExclusiveZone(height+10);}
   if(role==3){layer->setLayer(W::LayerOverlay);layer->setAnchors(W::Anchors(W::AnchorTop)|(name=="HarborMenu"||name=="RecentItems"?W::AnchorLeft:W::AnchorRight));layer->setMargins(QMargins(18,42,18,0));layer->setExclusiveZone(0);}
   layer->setKeyboardInteractivity(role==3?W::KeyboardInteractivityOnDemand:W::KeyboardInteractivityNone);
  }
  view->setSource(QUrl("qrc:/qml/"+name+".qml"));if(view->status()==QQuickView::Error){delete view;return nullptr;}
  view->show();if(role>0&&!preview&&app.platformName()=="wayland")KWindowEffects::enableBlurBehind(view,true);return view;
 };
 controller.onWindowAction=[&](QString name){QQuickView* v=popup?popup:((settings||filesMode)&&!surfaces.isEmpty()?surfaces[0]:nullptr);if(!v)return;if(name=="move")v->startSystemMove();else if(name=="resize")v->startSystemResize(Qt::RightEdge|Qt::BottomEdge);else if(name=="close")v->close();else if(name=="minimize")v->showMinimized();else if(name=="maximize"){if(v->visibility()==QWindow::Maximized)v->showNormal();else v->showMaximized();}};
 controller.onLogout=[&]{if(!preview&&!settings&&!filesMode&&!controlMode)QTimer::singleShot(100,&app,&QCoreApplication::quit);else{auto message=QDBusMessage::createMethodCall("org.harbor.Shell","/Shell","org.harbor.Shell","Logout");QDBusConnection::sessionBus().asyncCall(message,2000);}};
 controller.onClose=[&]{if(popup){popup->hide();popup->deleteLater();popup=nullptr;}else if(controlMode)app.quit();};
 controller.onOpen=[&](QString page){controller.dismiss();const bool settingsPage=page=="settings"||page.startsWith("settings:");QString name=page=="harbor"?"HarborMenu":page=="bluetooth"?"BluetoothMenu":page=="recent"?"RecentItems":page=="input"?"InputMenu":page=="menu"?"AppMenu":page=="notifications"?"NotificationCenter":page=="launcher"?"Launcher":settingsPage?"Settings":page=="windows"?"WindowList":"ControlCenter";popup=make(name,app.primaryScreen(),name=="Settings"?960:name=="Launcher"?680:name=="InputMenu"?320:390,name=="Settings"?680:name=="Launcher"?560:name=="InputMenu"?320:name=="HarborMenu"?450:560,name=="Settings"?-1:3);if(popup){if(settingsPage&&page.startsWith("settings:"))popup->rootObject()->setProperty("section",page.mid(9));popup->requestActivate();}};
 if(preview||settings||filesMode||controlMode){auto v=make(filesMode?"Files":settings?"Settings":controlMode?controlPage:"Preview",app.primaryScreen(),filesMode?1200:settings?960:controlMode?410:1440,filesMode?760:settings?680:controlMode?(controlPage=="HarborMenu"?450:600):900,-1);if(!v)return 2;surfaces<<v;
  if(settings){int pageIndex=args.indexOf("--settings")+1;if(pageIndex<args.size()&&!args[pageIndex].startsWith("--")){const QStringList pages{"Gestures","Notifications & Focus","Startup Applications","Privacy & Permissions","Date & Time","Storage","Default Applications","Mouse","Touchpad","Printers","Language & Region","General","Wi-Fi","Bluetooth","Network","Sound","Appearance","Accessibility","Desktop & Dock","Displays","Keyboard","Battery","Users & Groups","About","Software Update"};if(pages.contains(args[pageIndex]))v->rootObject()->setProperty("section",args[pageIndex]);}}
  if(filesMode||preview){auto fileRoot=filesMode?v->rootObject():v->rootObject()->findChild<QQuickItem*>("filesView");if(fileRoot){auto exporter=new FilesMenu(v);fileRoot->setProperty("menuExporter",QVariant::fromValue(static_cast<QObject*>(exporter)));if(filesMode)attachWindowMenu(v,exporter->objectPath());else menu.setSource(QDBusConnection::sessionBus().baseService(),exporter->objectPath());}}
  QObject::connect(v,&QWindow::visibleChanged,&app,[&app,v]{if(!v->isVisible())app.quit();});
  int i=args.indexOf("--screenshot");if(i>=0&&i+1<args.size()){auto path=args[i+1];int sizeArg=args.indexOf("--screenshot-size");if(sizeArg>=0&&sizeArg+1<args.size()){auto parts=args[sizeArg+1].split('x');if(parts.size()==2){int w=parts[0].toInt(),h=parts[1].toInt();if(w>=740&&w<=3840&&h>=560&&h<=2160)v->resize(w,h);}}QTimer::singleShot(1800,&app,[&app,v,path]{bool ok=v->grabWindow().save(path);app.exit(ok?0:3);});}
 }else{
  auto addScreen=[&](QScreen* screen){auto geo=screen->geometry();for(auto spec:QList<QPair<QString,int>>{{"Desktop",0},{"MenuBar",1},{"Dock",2}}){auto v=make(spec.first,screen,spec.second==2?720:geo.width(),spec.second==0?geo.height():spec.second==1?38:78,spec.second);if(v){surfaces<<v;QObject::connect(screen,&QScreen::geometryChanged,v,[v,spec](QRect r){if(spec.second!=2)v->setWidth(r.width());if(spec.second==0)v->setHeight(r.height());});}}};
  for(auto screen:app.screens())addScreen(screen);
  QObject::connect(&app,&QGuiApplication::screenAdded,&app,addScreen);
  QObject::connect(&app,&QGuiApplication::screenRemoved,&app,[&](QScreen* screen){for(int i=surfaces.size()-1;i>=0;--i)if(surfaces[i]->property("harborScreen").value<QScreen*>()==screen){delete surfaces.takeAt(i);}});
 }
 if(args.contains("--diagnose"))QTimer::singleShot(1200,&app,[&]{QJsonObject j{{"windowManagement",windows.available()},{"platform",app.platformName()},{"screens",app.screens().size()},{"windows",windows.windows().size()},{"bluetoothAvailable",services.state().value("bluetoothAvailable").toBool()},{"bluetoothStatus",services.state().value("bluetoothStatus").toString()},{"bluetoothDeviceCount",services.state().value("bluetoothDevices").toList().size()}};QTextStream(stdout)<<QJsonDocument(j).toJson();app.quit();});
 int result=app.exec();controller.dismiss();qDeleteAll(surfaces);return result;
}
