#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QtCore/QJniObject>

extern "C" void gx_startForegroundAudioService_android()
{
#ifdef Q_OS_ANDROID
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid()) return;

    QJniObject intent("android/content/Intent");

    QJniObject pkgName = context.callObjectMethod("getPackageName", "()Ljava/lang/String;");
    intent.callObjectMethod("setClassName",
                            "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                            pkgName.object<jstring>(),
                            QJniObject::fromString("com.genesisx.background.GXAudioService")
                                .object<jstring>());

    context.callObjectMethod("startForegroundService",
                             "(Landroid/content/Intent;)Landroid/content/ComponentName;",
                             intent.object<jobject>());
#endif
}

extern "C" void gx_stopForegroundAudioService_android()
{
#ifdef Q_OS_ANDROID
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid()) return;

    QJniObject intent("android/content/Intent");

    intent.callObjectMethod("setAction",
                            "(Ljava/lang/String;)Landroid/content/Intent;",
                            QJniObject::fromString("com.genesisx.app.ACTION_STOP")
                                .object<jstring>());

    // context.startService(intent);
    context.callObjectMethod("startService",
                             "(Landroid/content/Intent;)Landroid/content/ComponentName;",
                             intent.object<jobject>());
#endif
}
