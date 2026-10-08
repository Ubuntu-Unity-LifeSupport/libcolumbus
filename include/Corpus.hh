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

#ifndef CORPUS_HH_
#define CORPUS_HH_

#include "ColumbusCore.hh"

COL_NAMESPACE_START

struct CorpusPrivate;
class Document;

class COL_PUBLIC Corpus final {
private:
    CorpusPrivate *p;

public:
    Corpus();
    ~Corpus();
    Corpus(const Corpus &c) = delete;
    const Corpus& operator=(const Corpus &c) = delete;

    void addDocument(const Document &d);
    size_t size() const;
    const Document& getDocument(size_t i) const;

    // Add iterators here. This class should really only expose them.
};

COL_NAMESPACE_END

#endif /* CORPUS_HH_ */
