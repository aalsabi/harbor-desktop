#include "Files.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QProcess>
#include <QCoreApplication>
#include <QJsonDocument>
Files::Files(QObject* p):QObject(p){locations<<QDir::homePath();histories<<QStringList{path()};positions<<0;connect(&watcher,&QFileSystemWatcher::directoryChanged,this,&Files::refresh);watch();}
QString Files::path()const{return locations.value(active);}
void Files::watch(){if(!watcher.directories().isEmpty())watcher.removePaths(watcher.directories());watcher.addPath(path());}
void Files::refresh(){emit changed();}
void Files::go(QString p,bool record){if(p.startsWith("file:"))p=QUrl(p).toLocalFile();if(p=="~")p=QDir::homePath();QFileInfo f(p);if(!f.isDir()||!f.isReadable()){message=tr("Cannot read this folder");emit changed();return;}p=f.absoluteFilePath();locations[active]=QDir::cleanPath(p);if(record&&histories[active].value(positions[active])!=path()){histories[active]=histories[active].mid(0,positions[active]+1);histories[active]<<path();positions[active]=histories[active].size()-1;}filter.clear();message.clear();watch();emit changed();}
void Files::navigate(QString p){go(p,true);}void Files::up(){navigate(parentPath(path()));}QString Files::parentPath(QString p){return QFileInfo(p).dir().absolutePath();}
void Files::back(){if(positions[active]>0)go(histories[active][--positions[active]],false);}void Files::forward(){if(positions[active]+1<histories[active].size())go(histories[active][++positions[active]],false);}
void Files::addTab(){locations<<path();histories<<QStringList{path()};positions<<0;active=locations.size()-1;watch();emit changed();}
void Files::selectTab(int i){if(i>=0&&i<locations.size()){active=i;filter.clear();watch();emit changed();}}
void Files::closeTab(int i){if(locations.size()<2||i<0||i>=locations.size())return;locations.removeAt(i);histories.removeAt(i);positions.removeAt(i);active=qMin(active,int(locations.size()-1));watch();emit changed();}
void Files::setHidden(bool v){showHidden=v;emit changed();}void Files::setSearch(QString v){filter=v;emit changed();}
QVariantList Files::list(QString p,bool search)const{QVariantList out;QDir d(p);auto flags=QDir::AllEntries|QDir::System|QDir::NoDotAndDotDot;if(showHidden)flags|=QDir::Hidden;QMimeDatabase mime;for(const auto& f:d.entryInfoList(flags,QDir::DirsFirst|QDir::Name|QDir::IgnoreCase)){if(search&&!filter.isEmpty()&&!f.fileName().contains(filter,Qt::CaseInsensitive))continue;auto type=mime.mimeTypeForFile(f,QMimeDatabase::MatchExtension);out<<QVariantMap{{"name",f.fileName()},{"path",f.absoluteFilePath()},{"url",QUrl::fromLocalFile(f.absoluteFilePath()).toString()},{"thumbnail",f.isFile()&&f.size()<20*1024*1024&&type.name().startsWith("image/")?QUrl::fromLocalFile(f.absoluteFilePath()).toString():QString()},{"folder",f.isDir()},{"link",f.isSymLink()},{"icon",f.isDir()?"system-file-manager":type.iconName()},{"size",f.size()},{"modified",f.lastModified().toString("yyyy-MM-dd hh:mm")},{"kind",f.isDir()?tr("Folder"):type.comment()}};}return out;}
QVariantList Files::entries()const{return list(path(),true);}QVariantList Files::children(QString p){return list(p,false);}
QVariantList Files::places()const{QVariantList out;for(auto t:{QStandardPaths::HomeLocation,QStandardPaths::DesktopLocation,QStandardPaths::DocumentsLocation,QStandardPaths::DownloadLocation,QStandardPaths::PicturesLocation,QStandardPaths::MusicLocation,QStandardPaths::MoviesLocation}){auto p=QStandardPaths::writableLocation(t);if(QFileInfo(p).isDir())out<<QVariantMap{{"name",QStandardPaths::displayName(t)},{"path",p}};}for(auto s:QStorageInfo::mountedVolumes())if(s.isValid()&&s.isReady()&&(s.rootPath()=="/"||s.rootPath().startsWith("/media/")||s.rootPath().startsWith("/mnt/")||s.rootPath().startsWith("/run/media/")||s.rootPath()=="/home"))out<<QVariantMap{{"name",s.displayName()},{"path",s.rootPath()}};return out;}
QVariantMap Files::preview(QString p){QFileInfo f(p);QMimeDatabase db;auto m=db.mimeTypeForFile(f);QVariantMap out{{"name",f.fileName()},{"kind",m.comment()},{"size",f.size()},{"modified",f.lastModified().toString()},{"image",""},{"text",""}};if(f.isFile()&&f.size()<20*1024*1024&&m.name().startsWith("image/"))out["image"]=QUrl::fromLocalFile(p).toString();else if(f.isFile()&&m.inherits("text/plain")){QFile file(p);if(file.open(QIODevice::ReadOnly))out["text"]=QString::fromUtf8(file.read(65536));}return out;}
void Files::open(QString p){if(QFileInfo(p).isDir())navigate(p);else if(!QDesktopServices::openUrl(QUrl::fromLocalFile(p))){message=tr("No application could open this file");emit changed();}}
void Files::copy(QStringList p,bool cut){clipboard=p;cutting=cut;message=cut?tr("Ready to move; choose destination and Paste"):tr("Ready to copy; choose destination and Paste");emit changed();}void Files::paste(){if(!clipboard.isEmpty())operate(cutting?"move":"copy",clipboard);}
void Files::drop(QVariantList urls,bool move){QStringList p;for(auto u:urls){QUrl url(u.toString());if(url.isLocalFile())p<<url.toLocalFile();}if(!p.isEmpty())operate(move?"move":"copy",p);}
void Files::operate(QString action,QStringList sources,QString name){if(working)return;if(!QStringList{"copy","move","rename","mkdir","trash"}.contains(action))return;auto helper=QStandardPaths::findExecutable("harbor-file-operation");if(helper.isEmpty()){message=tr("Install harbor-file-operation before changing files");emit changed();return;}auto p=new QProcess(this);working=true;message.clear();emit changed();connect(p,&QProcess::finished,this,[this,p,action](int code,QProcess::ExitStatus exit){working=false;message=exit==QProcess::NormalExit&&code==0?tr("Completed"):QString::fromUtf8(p->readAllStandardError());if(code==0&&action=="move")clipboard.clear();p->deleteLater();refresh();});connect(p,&QProcess::errorOccurred,this,[this,p](QProcess::ProcessError e){if(e==QProcess::FailedToStart){working=false;message=p->errorString();p->deleteLater();emit changed();}});QStringList args{action,"--destination",path(),"--name",name,"--"};args<<sources;p->start(helper,args);}

void Files::goHome(){navigate(QDir::homePath());}
