// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Framework/DeviceControl/KeepAwake.h>

#ifdef Q_OS_IOS
#import <UIKit/UIKit.h>

void gx::framework::KeepAwake::applyPlatform(bool on)
{
  dispatch_async(dispatch_get_main_queue(), ^{
    [UIApplication sharedApplication].idleTimerDisabled = on ? YES : NO;
  });
}

#endif
