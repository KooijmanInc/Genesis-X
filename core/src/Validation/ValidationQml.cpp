// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <QtQml/qqml.h>
#include <QQmlEngine>

#include "ValidationQml.h"
#include <GenesisX/Validation/Validation.h>

using namespace gx::validation;

static QVariantMap toMap(const Validation::Result& r)
{
    return {
        { "ok", r.ok },
        { "code", static_cast<int>(r.code) },
        { "messageKey", r.messageKey }
    };
}

void registerGenesisXValidation(QQmlEngine *engine)
{
    Q_UNUSED(engine);

    static ValidationQml* s_validation = nullptr;
    if (!s_validation) s_validation = new ValidationQml();

    qmlRegisterSingletonInstance("GenesisX.Validation", 1, 0, "Validation", s_validation);
}

QVariantMap ValidationQml::minLength(const QString &s, int minLen)
{
    return toMap(Validation::minLength(s, minLen));
}

QVariantMap ValidationQml::maxLength(const QString &s, int maxLen)
{
    return toMap(Validation::maxLength(s, maxLen));
}

QVariantMap ValidationQml::email(const QString &s) const
{
    return toMap(Validation::email(s));
}

QVariantMap ValidationQml::onlyLetters(const QString &s) const
{
    return toMap(Validation::onlyLetters(s));
}

QVariantMap ValidationQml::onlyNumbers(const QString &s) const
{
    return toMap(Validation::onlyNumbers(s));
}

QVariantMap ValidationQml::onlyLettersAndNumbers(const QString &s) const
{
    return toMap(Validation::onlyLettersAndNumbers(s));
}

QVariantMap ValidationQml::onlyLettersAndCharacters(const QString &s, const QString &extraChars) const
{
    return toMap(Validation::onlyLettersAndCharacters(s, extraChars));
}

QVariantMap ValidationQml::onlyLettersAndNumbersAndCharacters(const QString &s, const QString &extraChars)
{
    return toMap(Validation::onlyLettersAndNumbersAndCharacters(s, extraChars));
}

QVariantMap ValidationQml::passwordLettersNumbers(const QString &s, int minLen) const
{
    return toMap(Validation::passwordLettersNumbers(s, minLen));
}

QVariantMap ValidationQml::passwordLettersNumbersSpecial(const QString &s, int minLen) const
{
    return toMap(Validation::passwordLettersNumbersSpecial(s, minLen));
}
