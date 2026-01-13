// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Framework/DeviceControl/KeepAwake.h>

#ifdef Q_OS_IOS
#import <UIKit/UIKit.h>

void KeepAwake::applyPlatform(bool on)
{
  dispach_async(dispatch_get_main_queue(), ^{
    [UIApplication sharedApplication].idleTimerDisabled = on ? YES : NO;
  });
}

#endif
