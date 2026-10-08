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

#ifndef INDEXMATCHES_H_
#define INDEXMATCHES_H_

#include <cstddef>
#include "ColumbusCore.hh"

COL_NAMESPACE_START

class LevenshteinIndex;
struct IndexMatchesPrivate;
class Word;

/**
 * A class that contains a list of index matches
 * in growing error order.
 *
 */
class COL_PUBLIC IndexMatches final {
    friend class LevenshteinIndex;

private:

    IndexMatchesPrivate *p;

    void addMatch(const Word &queryWord, const WordID matchedWord, int error);
    void sort();

public:
    IndexMatches();
    ~IndexMatches();
    IndexMatches(const IndexMatches &other) = delete;
    const IndexMatches & operator=(const IndexMatches &other) = delete;

    size_t size() const;
    const WordID& getMatch(size_t num) const;
    const Word& getQuery(size_t num) const;
    int getMatchError(size_t num) const;
    void clear();

};

COL_NAMESPACE_END

#endif /* INDEXMATCHES_H_ */
