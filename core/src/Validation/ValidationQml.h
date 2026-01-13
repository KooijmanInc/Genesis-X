// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef VALIDATIONQML_H
#define VALIDATIONQML_H

#include <QObject>
#include <QVariantMap>
#include <QtQml/qqml.h>

#include <GenesisX/genesisx_global.h>

class QQmlEngine;

namespace gx::validation {

GENESISX_CORE_EXPORT void registerGenesisXValidation(QQmlEngine* engine);

class GENESISX_CORE_EXPORT ValidationQml : public QObject
{
    Q_OBJECT

public:
    explicit ValidationQml(QObject* parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QVariantMap minLength(const QString& s, int minLen);
    Q_INVOKABLE QVariantMap maxLength(const QString& s, int maxLen);

    Q_INVOKABLE QVariantMap email(const QString& s) const;
    Q_INVOKABLE QVariantMap onlyLetters(const QString& s) const;
    Q_INVOKABLE QVariantMap onlyNumbers(const QString& s) const;
    Q_INVOKABLE QVariantMap onlyLettersAndNumbers(const QString& s) const;
    Q_INVOKABLE QVariantMap onlyLettersAndCharacters(const QString& s, const QString& extraChars = " -_'.") const;
    Q_INVOKABLE QVariantMap onlyLettersAndNumbersAndCharacters(const QString& s, const QString& extraChars = " -_'.");

    Q_INVOKABLE QVariantMap passwordLettersNumbers(const QString& s, int minLen = 8) const;
    Q_INVOKABLE QVariantMap passwordLettersNumbersSpecial(const QString& s, int minLen = 10) const;
};

}

#endif // VALIDATIONQML_H
