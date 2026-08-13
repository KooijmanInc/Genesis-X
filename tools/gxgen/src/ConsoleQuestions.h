// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef CONSOLEQUESTIONS_H
#define CONSOLEQUESTIONS_H

#include <QString>

class ConsoleQuestions
{
public:
    ConsoleQuestions();

    bool askYesNo(const QString& question, bool defaultValue = true);
};

#endif // CONSOLEQUESTIONS_H
