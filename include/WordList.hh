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

#ifndef WORDLIST_HH_
#define WORDLIST_HH_

#include "ColumbusCore.hh"

COL_NAMESPACE_START

struct WordListPrivate;
class Word;

class COL_PUBLIC WordList final {
private:
    WordListPrivate *p;

public:
    WordList();
    WordList(const WordList &wl);
    WordList(WordList &&wl);
    ~WordList();

    size_t size() const;
    const Word& operator[](const size_t i) const;
    const WordList& operator=(const WordList &l);
    const WordList& operator=(WordList &&wl);
    bool operator==(const WordList &l) const;
    bool operator!=(const WordList &l) const;
    void addWord(const Word &w); // This is more of an implementation detail and should not be exposed in a base class or interface.

    // Add proper iterators here.
};

COL_NAMESPACE_END

#endif /* WORDLIST_HH_ */
