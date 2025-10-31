#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>

#include <GenesisX/genesisx_global.h>

namespace gx::app::audiorecorder {

class GENESISX_CORE_EXPORT AudioRecorder : public QObject
{
    Q_OBJECT
public:
    explicit AudioRecorder(QObject *parent = nullptr);

signals:
};

}

#endif // AUDIORECORDER_H
