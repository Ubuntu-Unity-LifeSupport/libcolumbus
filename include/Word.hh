/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WORD_HH_
#define WORD_HH_

#include "ColumbusCore.hh"
#include <string>

COL_NAMESPACE_START

/**
 * A word encapsulates a single word. That is,
 * there is no whitespace in it.
 *
 * A word's contents are immutable.
 */
class COL_PUBLIC Word final {
private:

    Letter *text; // Change this to a shared pointer to save memory.
    unsigned int len;

    bool hasWhitespace();
    void duplicateFrom(const Word &w);
    void convertString(const char *utf8Word);

public:
    Word();
    Word(const Word &w);
    Word(Word &&w);
    Word(const std::string &w);
    explicit Word(const char *utf8Word);
    explicit Word(Letter *letters, size_t length);
    ~Word();

    unsigned int length() const { return len;}
    void toUtf8(char *buf, unsigned int bufSize) const;
    std::string asUtf8() const;

    Word join(const Word &w) const;

    Letter operator[](unsigned int i) const;
    bool operator==(const Word &w) const;
    bool operator==(const std::string &utf8Str) const;
    bool operator==(const char *utf8Word) const;
    bool operator!=(const Word &w) const;
    bool operator!=(const std::string &utf8Str) const;
    bool operator!=(const char *utf8Word) const;
    bool operator<(const Word &w) const;
    Word& operator=(const Word &w);
    Word& operator=(Word &&w);
    Word& operator=(const char *utf8Word);
    Word& operator=(const std::string &utf8Str);

    size_t hash() const;

    friend class LevenshteinIndex;
};

COL_NAMESPACE_END

#endif /* WORD_HH_ */
