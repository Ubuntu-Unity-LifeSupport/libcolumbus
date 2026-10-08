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

#include "Word.hh"
#include "MatcherStatistics.hh"

#ifdef HAS_SPARSE_HASH
#include <google/sparse_hash_map>
using google::sparse_hash_map;
#define hashmap sparse_hash_map
#else
#include <unordered_map>
#define hashmap unordered_map
#endif

COL_NAMESPACE_START
using namespace std;

struct MatcherStatisticsPrivate {
    hashmap<WordID, size_t> totalWordCounts;
};

MatcherStatistics::MatcherStatistics() {
    p = new MatcherStatisticsPrivate();

}

MatcherStatistics::~MatcherStatistics() {
    delete p;
}

void MatcherStatistics::wordProcessed(const WordID w) {
    auto it = p->totalWordCounts.find(w);
    if(it == p->totalWordCounts.end()) {
        p->totalWordCounts[w] = 1;
    } else {
        it->second++;
    }
}

size_t MatcherStatistics::getTotalWordCount(const WordID w) const {
    auto it = p->totalWordCounts.find(w);
    if(it == p->totalWordCounts.end()) {
        return 0;
    } else {
        return it->second;
    }

}

void MatcherStatistics::addedWordToIndex(const WordID /*word*/, const Word &/*fieldName*/) {
    // Doesn't do anything yet.
}

COL_NAMESPACE_END
