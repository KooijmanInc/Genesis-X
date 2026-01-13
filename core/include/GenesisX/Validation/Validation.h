// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef VALIDATION_H
#define VALIDATION_H

#include <QString>

namespace gx::validation {

class Validation
{
public:
    enum class Code {
        Ok = 0,

        Empty,
        TooShort,
        TooLong,

        InvalidFormat,
        ContainsInvalidChars,

        MissingLetter,
        MissingNumber,
        MissingSpecial
    };

    struct Result {
        bool ok = false;
        Code code = Code::Ok;

        QString messageKey;
    };

    static Result required(const QString& s, const QString& keyPrefix = "validation");
    static Result minLength(const QString& s, int minLen, const QString& keyPrefix = "validation");
    static Result maxLength(const QString& s, int maxLen, const QString& keyPrefix = "validation");

    static Result email(const QString& s);
    static Result passwordLettersNumbers(const QString& s, int minLen = 8);
    static Result passwordLettersNumbersSpecial(const QString& s, int minLen = 10);
    static Result onlyLetters(const QString& s);
    static Result onlyNumbers(const QString& s);
    static Result onlyLettersAndNumbers(const QString& s);
    static Result onlyLettersAndCharacters(const QString& s, const QString& extraChars = " -_'.");
    static Result onlyLettersAndNumbersAndCharacters(const QString& s, const QString& extraChars = " -_'.");

private:
    static QString key(const QString& base, const QString& suffix);
    static QString escapeForCharClass(const QString& s);
};

}

#endif // VALIDATION_H
