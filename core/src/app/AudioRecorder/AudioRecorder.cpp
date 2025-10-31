#include "AudioRecorder.h"

/*!
  \class gx::app::audiorecorder::AudioRecorder
  \inmodule GenesisX
  \ingroup genesisx-core
  \since 6.10
  \brief Minimal Audio Recorder facade (constructor-only).

  This class will serve as the bridge to the app's .

  \note Linked QML module: \c GenesisX\App\AudioRecorder 1.0 (singleton).
  \note Enabled when the app uses qmake flag \c genesisx_app_audiorecorder
 */
using namespace gx::app::audiorecorder;

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject{parent}
{}
