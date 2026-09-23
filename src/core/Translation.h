#pragma once
#include <QTranslator>
#include <QHash>
class Translation:public QTranslator {
public:using QTranslator::QTranslator;bool arabic=false;
 bool isEmpty()const override{return false;}
 QString translate(const char*,const char* source,const char* =nullptr,int =-1)const override{
  if(!arabic)return {};
  static const QHash<QString,QString> words{
   {"Appearance","المظهر"},{"Network","الشبكة"},{"Sound","الصوت"},{"Bluetooth","بلوتوث"},{"Displays","الشاشات"},{"Power","الطاقة"},{"Users","المستخدمون"},{"Updates","التحديثات"},{"About","حول"},
   {"System settings","إعدادات النظام"},{"Find a setting","ابحث في الإعدادات"},{"Make this desktop your own.","خصص سطح المكتب بطريقتك."},{"Dark","داكن"},{"Light","فاتح"},{"Glass opacity","عتامة الزجاج"},{"Reduce motion","تقليل الحركة"},
   {"Control center","مركز التحكم"},{"Your devices. One place.","أجهزتك في مكان واحد."},{"Brightness","السطوع"},{"Open system settings","فتح إعدادات النظام"},{"Toggle mute","تبديل كتم الصوت"},{"Light appearance","المظهر الفاتح"},{"Dark appearance","المظهر الداكن"},{"Lock","قفل"},
   {"Applications · right-click to pin","التطبيقات · انقر باليمين للتثبيت"},{"Search installed applications…","ابحث عن تطبيق مثبت…"},{"Open windows","النوافذ المفتوحة"},{"Choose a window to return to it","اختر نافذة للعودة إليها"},{"Window management protocol unavailable","بروتوكول إدارة النوافذ غير متاح"},{"Close panel","إغلاق اللوحة"},
   {"Notifications","الإشعارات"},{"Recent notifications","الإشعارات الحديثة"},{"Notification service owned by another desktop","خدمة الإشعارات مستخدمة بواسطة جلسة أخرى"},{"Wi-Fi unavailable","واي فاي غير متاح"},{"Unavailable","غير متاح"},{"Nearby networks","الشبكات القريبة"},{"No scan results","لا توجد نتائج مسح"},
   {"Manage connections — external editor","إدارة الاتصالات — محرر خارجي"},{"Audio devices — external mixer","أجهزة الصوت — أداة خارجية"},{"Mute / unmute","كتم / إلغاء الكتم"},{"Pair devices — external manager","اقتران الأجهزة — مدير خارجي"},
   {"Apply","تطبيق"},{"Keep display settings","الاحتفاظ بإعدادات العرض"},{"A display change reverts after 15 seconds unless confirmed.","تعود إعدادات العرض السابقة بعد 15 ثانية ما لم تؤكد التغيير."},
   {"Open user manager","فتح مدير المستخدمين"},{"Open software manager","فتح مدير البرامج"},{"Current profile: ","نمط الطاقة: "},{"An original translucent surface. Optical refraction is not simulated.","سطح شفاف بتصميم أصلي. لا يحاكي الانكسار الضوئي."}
  };return words.value(QString::fromUtf8(source));
 }
};
