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

#include "IndexWeights.hh"
#include "Word.hh"
#include <map>

COL_NAMESPACE_START
using namespace std;

struct IndexWeightsPrivate {
    map<Word, double> weigths;
};

IndexWeights::IndexWeights() {
    p = new IndexWeightsPrivate();

}

IndexWeights::~IndexWeights() {
    delete p;
}


void IndexWeights::setWeight(const Word &w, double weigth) {
    p->weigths[w] = weigth;
}

double IndexWeights::getWeight(const Word &w) const {
    map<Word, double>::iterator it = p->weigths.find(w);
    if(it == p->weigths.end())
        return 1.0;
    return it->second;
}

COL_NAMESPACE_END
