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

/*
 * This exe is meant to be used to measure speed and
 * memory consumption of Index building.
 */

#include "columbus.hh" // This app should only need public API from Columbus.
#include <fstream>
#include <string>
#include <climits>

using namespace std;
using namespace Columbus;

Matcher* build_matcher(const char *dataFile, int maxLines) {
    Matcher *m = 0;
    Corpus *c = new Corpus();
    const int batchSize = 100000;
    Word field("name");
    double dataReadStart, dataReadEnd;
    int i = 0;
    size_t totalDocs = 0;

    ifstream ifile(dataFile);
    if(ifile.fail()) {
        printf("Could not open file %s.\n", dataFile);
        exit(1);
    }
    string line;

    m = new Matcher();

    // Build Corpus.
    dataReadStart = hiresTimestamp();
    while(getline(ifile, line)) {
        if(line.size() == 0)
            continue;
        totalDocs++;
        Document d(totalDocs);
        d.addText(field, line.c_str());
        c->addDocument(d);
        i++;
        if(i % batchSize == 0) {
            m->index(*c);
            delete c;
            c = new Corpus();
        }
        if(i >= maxLines)
            break;
    }
    m->index(*c);
    delete c;
    dataReadEnd = hiresTimestamp();
    printf("Read in %lu documents in %.2f seconds.\n", (unsigned long)totalDocs, dataReadEnd - dataReadStart);
    return m;
}

int main(int argc, char **argv) {
    int maxLines = INT_MAX;
    Matcher *m;
    if(argc == 1) {
        printf("%s datafile.txt [num of lines]\n", argv[0]);
        return 1;
    }
    if(argc > 2)
        maxLines = atoi(argv[2]);
    try {
        m = build_matcher(argv[1], maxLines);
        delete m;
    } catch(const std::exception &e) {
        fprintf(stderr, "Fail: %s\n", e.what());
        return 666;
    }
    return 0;
}
