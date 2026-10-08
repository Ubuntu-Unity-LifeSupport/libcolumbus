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

#include "LevenshteinIndex.hh"
#include "Word.hh"
#include "IndexMatches.hh"
#include "ErrorValues.hh"
#include "WordStore.hh"
#include <cstdio>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cassert>

using namespace Columbus;

void load_data(LevenshteinIndex &ind, WordStore &s, char *file) {
    FILE *f = fopen(file, "r");
    char buffer[1024];
    if(!f) {
        printf("Could not open file %s.\n", file);
        exit(1);
    }
    while(fgets(buffer, 1024, f) != NULL) {
        unsigned int slen = strlen(buffer);
        assert(buffer[slen-1] == '\n');
        buffer[slen-1] = '\0'; // Chop the \n.
        Word w(buffer);
        ind.insertWord(w, s.getID(w));
    }
    fclose(f);
}

void queryAndPrint(LevenshteinIndex &ind, WordStore &s, Word &query, int maxError) {
    IndexMatches matches;
    ErrorValues e;
    ind.findWords(query, e, maxError, matches);
    if(matches.size() == 0) {
        printf("No matches.\n");
        return;
    }
    for(size_t i=0; i<matches.size(); i++) {
        const unsigned int bufSize = 1024;
        char buf[bufSize];
        s.getWord(matches.getMatch(i)).toUtf8(buf, bufSize);
        printf("%s %d\n", buf, matches.getMatchError(i));
    }
}

int run_test(int argc, char **argv) {
    LevenshteinIndex ind;
    WordStore s;
    char *file;
    int maxError;
    Word *query;
    if(argc < 4) {
        printf("%s datafile queryword max_error\n", argv[0]);
        return 0;
    }
    file = argv[1];
    try {
        query = new Word(argv[2]);
    } catch(std::invalid_argument &e) {
        printf("Query term \"%s\" must not have whitespace in it.", argv[2]);
        return 1;
    }
    maxError = atoi(argv[3]);

    try {
        load_data(ind, s, file);
    } catch(std::invalid_argument &e) {
        printf("Data file malformed: %s\n", e.what());
        return 1;
    }

    try {
        queryAndPrint(ind, s, *query, maxError);
    } catch(std::exception &e) {
        fprintf(stderr, "Query failed with exception: %s\n", e.what());
        return 99;
    }

    delete query;
    return 0;
}

int main(int argc, char **argv) {
    try {
        return run_test(argc, argv);
    } catch(std::exception &e) {
        printf("Fail: %s.\n", e.what());
        return 105;
    }
}

