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

#ifndef MATCHER_HH_
#define MATCHER_HH_

#include "ColumbusCore.hh"
#include<string>

COL_NAMESPACE_START

class Corpus;
struct MatcherPrivate;
class Word;
class Document;
class WordList;
class MatchResults;
class ErrorValues;
class IndexWeights;
class ResultFilter;
class SearchParameters;

class COL_PUBLIC Matcher final {
private:
    MatcherPrivate *p;

    void buildIndexes(const Corpus &c);
    void addToIndex(const Word &word, const WordID wordID, const WordID indexID);
    void relevancyMatch(const WordList &query, const SearchParameters &params, const int extraError, MatchResults &matchedDocuments);

public:
    Matcher();
    ~Matcher();
    Matcher& operator=(const Matcher &m) = delete;

    // The simple API
    MatchResults match(const char *queryAsUtf8);
    MatchResults match(const WordList &query);
    MatchResults match(const std::string &queryAsUtf8);

    // When you want to specify search parameters exactly.
    MatchResults match(const char *queryAsUtf8, const SearchParameters &params);
    MatchResults match(const WordList &query, const SearchParameters &params);
    void index(const Corpus &c);
    ErrorValues& getErrorValues();
    IndexWeights& getIndexWeights();
    /*
     * This function is optimized for online matches, that is, queries
     * that are live updated during typing. It uses slightly different
     * search heuristics to ensure results that "feel good" to humans.
     *
     * The second argument is the field that should be the primary focus.
     * Usually it means having the text that will be shown to the user.
     * As an example, in the HUD, this field would contain the command
     * (and nothing else) that will be executed.
     */
    MatchResults onlineMatch(const WordList &query, const Word &primaryIndex);
};

COL_NAMESPACE_END

#endif /* MATCHER_HH_ */
