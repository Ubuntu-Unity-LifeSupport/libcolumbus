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

#ifndef MATCHRESULTS_HH_
#define MATCHRESULTS_HH_

#include "ColumbusCore.hh"

COL_NAMESPACE_START

struct MatchResultsPrivate;
class Word;

class COL_PUBLIC MatchResults final {
    MatchResultsPrivate *p;

    void sortIfRequired() const;

public:
    MatchResults();
    ~MatchResults();
    MatchResults(const MatchResults &other);
    MatchResults(MatchResults &&other);

    const MatchResults& operator=(MatchResults &&other);
    const MatchResults& operator=(const MatchResults &other);

    void addResult(DocumentID docID, double relevancy);
    void addResults(const MatchResults &r);
    void copyResult(const MatchResults &other, const size_t i);
    size_t size() const;
    DocumentID getDocumentID(size_t i) const;
    double getRelevancy(size_t i) const;
};

COL_NAMESPACE_END

#endif /* MATCHRESULTS_HH_ */
