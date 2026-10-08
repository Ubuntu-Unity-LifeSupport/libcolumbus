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

#include "Corpus.hh"
#include "Document.hh"
#include <vector>
#include <stdexcept>

COL_NAMESPACE_START
using namespace std;

struct CorpusPrivate {
    vector<Document> documents;
};

Corpus::Corpus() {
    p = new CorpusPrivate();
}

Corpus::~Corpus() {
    delete p;
}

void Corpus::addDocument(const Document &d) {
    p->documents.push_back(d);
}

size_t Corpus::size() const {
    return p->documents.size();
}

const Document& Corpus::getDocument(size_t i) const {
    if(i >= p->documents.size())
        throw out_of_range("Out of bounds access in Document.");
    return p->documents[i];
}

COL_NAMESPACE_END
