// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include "ConsoleQuestions.h"

#include <QTextStream>

ConsoleQuestions::ConsoleQuestions() {}

bool ConsoleQuestions::askYesNo(const QString &question, bool defaultValue)
{
    QTextStream input(stdin);
    QTextStream output(stdout);

    const QString options = defaultValue ? "[Y/n]" : "[y/N]";

    while (true) {
        output << question << ' ' << options << ": " << Qt::flush;

        const QString answer = input.readLine().trimmed().toLower();

        if (answer.isEmpty()) {
            return defaultValue;
        }

        if (answer == "y" || answer == "yes") {
            return true;
        }

        if (answer == "n" || answer == "no") {
            return false;
        }

        output << "Please enter yes or no.\n";
    }
}
