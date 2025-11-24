// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "AudioRecorder.h"

/*!
    \class gx::app::audiorecorder::AudioRecorder
    \inmodule GenesisX
    \ingroup app-classes
    \since Qt 6.10
    \brief Minimal Audio Recorder facade (constructor-only).

    This class will serve as the bridge to the app's .

    \note Linked QML module: \c GenesisX\App\AudioRecorder 1.0 (singleton).
    \note Enabled when the app uses qmake flag \c genesisx_app_audiorecorder
 */
using namespace gx::app::audiorecorder;

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject{parent}
{}
