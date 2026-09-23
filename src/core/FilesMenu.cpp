#include "FilesMenu.h"
#include <QDBusConnection>
#include <QDBusMetaType>
FilesMenu::FilesMenu(QObject* parent):QObject(parent){
 qDBusRegisterMetaType<MenuLayout>();address="/Harbor/FilesMenu";
 QDBusConnection::sessionBus().registerObject(address,this,QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals);
}
FilesMenu::~FilesMenu(){QDBusConnection::sessionBus().unregisterObject(address);}
void FilesMenu::setState(QVariantMap value){if(state==value)return;state=value;emit LayoutUpdated(++revision,0);}
static bool findItem(const MenuLayout& tree,int id,MenuLayout& result){
 if(tree.id==id){result=tree;return true;}
 for(const auto& child:tree.children)if(findItem(qdbus_cast<MenuLayout>(child.variant()),id,result))return true;
 return false;
}
MenuLayout FilesMenu::layout()const{
 const int count=state.value("selectionCount").toInt();const bool busy=state.value("busy").toBool(),modal=state.value("modal").toBool();
 auto item=[&](int id,QString label,QString command,bool enabled=true){return MenuLayout{id,{{"label",label},{"enabled",enabled&&!modal},{"x-harbor-action",command}}, {}};};
 auto group=[](int id,QString label,std::initializer_list<MenuLayout> items){MenuLayout result{id,{{"label",label},{"children-display","submenu"}}, {}};for(const auto& i:items)result.children.append(QDBusVariant(QVariant::fromValue(i)));return result;};
 auto check=[&](MenuLayout i,bool checked,QString type="checkmark"){i.properties["toggle-type"]=type;i.properties["toggle-state"]=checked?1:0;return i;};
 const int view=state.value("viewMode").toInt();
 const bool textFocus=state.value("textFocus").toBool(),textSelected=state.value("textSelected").toBool(),readOnly=state.value("textReadOnly").toBool();
 QList<MenuLayout> groups{
  group(100,"File",{item(101,"New Tab","newTab"),item(102,"Close Tab","closeTab",state.value("tabs",1).toInt()>1),item(103,"New Folder…","newFolder",!busy),item(104,"Open","open",count==1),item(105,"Rename…","rename",count==1&&!busy),item(106,"Move to Trash…","trash",count>0&&!busy),item(107,"Close Window","close")}),
  group(200,"Edit",{item(201,"Copy","copy",textFocus?textSelected:count>0),item(202,"Cut","cut",textFocus?(textSelected&&!readOnly):count>0),item(203,"Paste","paste",textFocus?!readOnly:!busy),item(204,"Select All","selectAll")}),
  group(300,"View",{check(item(301,"as Icons","icons"),view==0,"radio"),check(item(302,"as List","list"),view==1,"radio"),check(item(303,"as Columns","columns"),view==2,"radio"),check(item(304,"Show Preview","preview"),state.value("preview").toBool()),check(item(305,"Show Hidden Files","hidden"),state.value("hidden").toBool()),item(306,"Refresh","refresh")}),
  group(400,"Go",{item(401,"Back","back"),item(402,"Forward","forward"),item(403,"Enclosing Folder","up"),item(404,"Home","home"),item(405,"Go to Folder…","location")}),
  group(500,"Window",{item(501,"Minimize","minimize"),item(502,"Zoom","maximize")})};
 MenuLayout root;for(const auto& g:groups)root.children.append(QDBusVariant(QVariant::fromValue(g)));return root;
}
static void filterLayout(MenuLayout& tree,int depth,const QStringList& properties){
 if(!properties.isEmpty()){for(auto it=tree.properties.begin();it!=tree.properties.end();)if(!properties.contains(it.key()))it=tree.properties.erase(it);else ++it;}
 if(depth==0){tree.children.clear();return;}
 for(auto& child:tree.children){auto node=qdbus_cast<MenuLayout>(child.variant());filterLayout(node,depth<0?-1:depth-1,properties);child=QDBusVariant(QVariant::fromValue(node));}
}
uint FilesMenu::GetLayout(int id,int depth,QStringList properties,MenuLayout& tree){tree={};findItem(layout(),id,tree);filterLayout(tree,depth,properties);return revision;}
void FilesMenu::Event(int id,QString event,QDBusVariant,uint){
 MenuLayout node;if(event!="clicked"||!findItem(layout(),id,node)||!node.properties.value("enabled",false).toBool())return;
 const auto command=node.properties.value("x-harbor-action").toString();if(!command.isEmpty())emit action(command);
}
