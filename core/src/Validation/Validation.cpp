// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#include <GenesisX/Validation/Validation.h>

#include <QChar>
#include <QRegularExpression>

using namespace gx::validation;

static inline Validation::Result okResult(const QString& messageKey = QString())
{
    Validation::Result r;
    r.ok = true;
    r.code = Validation::Code::Ok;
    r.messageKey = messageKey;
    return r;
}

static inline Validation::Result failResult(Validation::Code code, const QString& messageKey)
{
    Validation::Result r;
    r.ok = false;
    r.code = code;
    r.messageKey = messageKey;
    return r;
}

Validation::Result Validation::required(const QString &s, const QString &keyPrefix)
{
    if (s.trimmed().isEmpty()) {
        return failResult(Code::Empty, key(keyPrefix, "required"));
    }

    return okResult();
}

Validation::Result Validation::minLength(const QString &s, int minLen, const QString &keyPrefix)
{
    if (s.size() < minLen) {
        return failResult(Code::TooShort, key(keyPrefix, "tooShort"));
    }

    return okResult();
}

Validation::Result Validation::maxLength(const QString &s, int maxLen, const QString &keyPrefix)
{
    if (s.size() > maxLen) {
        return failResult(Code::TooLong, key(keyPrefix, "tooLang"));
    }

    return okResult();
}

Validation::Result Validation::email(const QString &s)
{
    auto req = required(s, "validation.email");
    if (!req.ok) return req;

    static const QRegularExpression reEmail(
        QStringLiteral(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)"),
        QRegularExpression::UseUnicodePropertiesOption
        );

    if (!reEmail.match(s).hasMatch()) {
        return failResult(Code::InvalidFormat, "validation.email.invalid");
    }

    return okResult();
}

Validation::Result Validation::passwordLettersNumbers(const QString &s, int minLen)
{
    auto req = required(s, "validation.password");
    if (!req.ok) return req;

    if (s.size() < minLen) {
        return failResult(Code::TooShort, "validation.password.tooShort");
    }

    bool hasLetter = false;
    bool hasNumber = false;

    for (const QChar ch : s) {
        if (ch.isLetter()) hasLetter = true;
        else if (ch.isDigit()) hasNumber = true;
    }

    if (!hasLetter) return failResult(Code::MissingLetter, "validation.password.missingLetter");
    if (!hasNumber) return failResult(Code::MissingNumber, "validation.password.missingNumber");

    return okResult();
}

Validation::Result Validation::passwordLettersNumbersSpecial(const QString &s, int minLen)
{
    auto base = passwordLettersNumbers(s, minLen);
    if (!base.ok) return base;

    bool hasSpecial = false;
    for (const QChar ch : s) {
        // "Special" = not letter/digit. You can tighten this to a whitelist later.
        if (!ch.isLetterOrNumber()) {
            hasSpecial = true;
            break;
        }
    }

    if (!hasSpecial) {
        return failResult(Code::MissingSpecial, "validation.password.missingSpecial");
    }

    return okResult();
}

Validation::Result Validation::onlyLetters(const QString &s)
{
    auto req = required(s, "validation.onlyLetters");
    if (!req.ok) return req;

    // Unicode letters only
    static const QRegularExpression reOnlyLetters(
        QStringLiteral(R"(^\p{L}+$)"),
        QRegularExpression::UseUnicodePropertiesOption
        );

    if (!reOnlyLetters.match(s).hasMatch()) {
        return failResult(Code::ContainsInvalidChars, "validation.onlyLetters.invalidChars");
    }

    return okResult();
}

Validation::Result Validation::onlyNumbers(const QString &s)
{
    auto req = required(s, "validation.onlyNumbers");
    if (!req.ok) return req;

    // Unicode decimal digits only
    static const QRegularExpression reOnlyNumbers(
        QStringLiteral(R"(^\p{Nd}+$)"),
        QRegularExpression::UseUnicodePropertiesOption
        );

    if (!reOnlyNumbers.match(s).hasMatch()) {
        return failResult(Code::ContainsInvalidChars, "validation.onlyNumbers.invalidChars");
    }

    return okResult();
}

Validation::Result Validation::onlyLettersAndNumbers(const QString &s)
{
    auto req = required(s, "validation.onlyLettersAndNumbers");
    if (!req.ok) return req;

    // Unicode letters + Unicode decimal digits only
    static const QRegularExpression reOnlyLettersNumbers(
        QStringLiteral(R"(^[\p{L}\p{Nd}]+$)"),
        QRegularExpression::UseUnicodePropertiesOption
        );

    if (!reOnlyLettersNumbers.match(s).hasMatch()) {
        return failResult(Code::ContainsInvalidChars, "validation.onlyLettersAndNumbers.invalidChars");
    }

    return okResult();
}

Validation::Result Validation::onlyLettersAndCharacters(const QString &s, const QString &extraChars)
{
    auto req = required(s, "validation.onlyLettersAndChars");
    if (!req.ok) return req;

    // Build: ^[\p{L}<extras>]+$
    // extras must be safe inside a regex char class
    const QString extrasEscaped = escapeForCharClass(extraChars);
    const QString pattern = QStringLiteral(R"(^[\p{L}%1]+$)").arg(extrasEscaped);

    const QRegularExpression re(
        pattern,
        QRegularExpression::UseUnicodePropertiesOption
        );

    if (!re.isValid()) {
        // If somebody passes a weird extraChars string and the pattern becomes invalid,
        // fail closed (invalid format).
        return failResult(Code::InvalidFormat, "validation.onlyLettersAndChars.invalidRule");
    }

    if (!re.match(s).hasMatch()) {
        return failResult(Code::ContainsInvalidChars, "validation.onlyLettersAndChars.invalidChars");
    }

    return okResult();
}

Validation::Result Validation::onlyLettersAndNumbersAndCharacters(const QString &s, const QString &extraChars)
{
    auto req = required(s, "validation.onlyLettersNumbersAndChars");
    if (!req.ok) return req;

    // Build: ^[\p{L}\p{Nd}<extras>]+$
    const QString extrasEscaped = escapeForCharClass(extraChars);
    const QString pattern = QStringLiteral(R"(^[\p{L}\p{Nd}%1]+$)").arg(extrasEscaped);

    const QRegularExpression re(
        pattern,
        QRegularExpression::UseUnicodePropertiesOption
        );

    if (!re.isValid()) {
        return failResult(Code::InvalidFormat, "validation.onlyLettersNumbersAndChars.invalidRule");
    }

    if (!re.match(s).hasMatch()) {
        return failResult(Code::ContainsInvalidChars, "validation.onlyLettersNumbersAndChars.invalidChars");
    }

    return okResult();
}

QString Validation::key(const QString &base, const QString &suffix)
{
    if (base.isEmpty()) return suffix;
    if (suffix.isEmpty()) return base;

    return base + "." + suffix;
}

QString Validation::escapeForCharClass(const QString &s)
{
    QString out;
    out.reserve(s.size() * 2);

    for (const QChar ch : s) {
        switch (ch.unicode()) {
        case '\\': out += QStringLiteral("\\\\"); break;
        case ']':  out += QStringLiteral("\\]");  break;
        case '^':  out += QStringLiteral("\\^");  break;
        case '-':  out += QStringLiteral("\\-");  break;
        default:
            out += ch;
            break;
        }
    }

    return out;
}
