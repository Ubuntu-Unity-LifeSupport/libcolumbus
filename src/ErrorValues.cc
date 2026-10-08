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
 * This class implements an error lookup system.
 * It is in the hottest of the hot paths in the entire system.
 * Use of crazy optimization techniques is approved.
 */

#include <map>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <cassert>
#include "ErrorValues.hh"
#include "Word.hh"
#include "ColumbusSlow.hh"

COL_NAMESPACE_START
using namespace std;

static const char *accentGroupDataFile[] =  {"latinAccentedLetterGroups.txt",
        "greekAccentedLetterGroups.txt"};

const int LUT_BITS = 9;
const int LUT_LETTERS = 1 << LUT_BITS;
const int LUT_SIZE = (LUT_LETTERS*LUT_LETTERS);
#define LUT_OFFSET(l1, l2) ((l1) << LUT_BITS | (l2))

static_assert(LUT_BITS > 0, "LUT_BITS must be larger than zero");

struct ErrorValuesPrivate {
    map<pair<Letter, Letter>, int> singleErrors;
    map<Letter, size_t> groupMap;
    vector<unsigned int> groupErrors;
    int *lut;

    ErrorValuesPrivate() { lut = new int[LUT_SIZE]; }
    ~ErrorValuesPrivate() { delete []lut; }
};

ErrorValues::ErrorValues() :
    insertionError(DEFAULT_ERROR),
    deletionError(DEFAULT_ERROR),
    endDeletionError(DEFAULT_ERROR),
    startInsertionError(DEFAULT_ERROR),
    substituteError(DEFAULT_ERROR),
    transposeError(DEFAULT_ERROR),
    substringStartLimit(0) {
    p = new ErrorValuesPrivate;
    clearLUT();
}

ErrorValues::~ErrorValues() {
    delete p;
}

void ErrorValues::clearLUT() {
    for(int i=0; i<LUT_LETTERS; i++) {
        for(int j=0; j<LUT_LETTERS; j++) {
            p->lut[LUT_OFFSET(i, j)] = i == j ? 0 : substituteError;
        }
    }
}

void ErrorValues::setError(Letter l1, Letter l2, const int error) {
    if(l1 > l2) {
        Letter tmp = l1;
        l1 = l2;
        l2 = tmp;
    }
    pair<Letter, Letter> in(l1, l2);
    p->singleErrors[in] = error;
    addToLUT(l1, l2, error);
}

int ErrorValues::getSubstituteError(Letter l1, Letter l2) const {
    if(l1 < LUT_LETTERS && l2 < LUT_LETTERS) {
        return p->lut[LUT_OFFSET(l1, l2)];
    }
    return substituteErrorSlow(l1, l2);
}

int ErrorValues::substituteErrorSlow(Letter l1, Letter l2) const {
    if(l1 == l2)
        return 0;
    if(l1 > l2) {
        Letter tmp = l1;
        l1 = l2;
        l2 = tmp;
    }
    pair<Letter, Letter> in(l1, l2);
    auto f = p->singleErrors.find(in);
    if(f != p->singleErrors.end())
        return f->second;

    // Are the letters in the same error group? Check the bigger
    // value first, because it is probably a more uncommon letter.
    auto g1 = p->groupMap.find(l2);
    if(g1 != p->groupMap.end()) {
        auto g2 = p->groupMap.find(l1);
        if(g2 != p->groupMap.end()) {
            if(g1->second == g2->second) {
                return p->groupErrors[g1->second];
            }
        }
    }
    return substituteError;
}

void ErrorValues::clearErrors() {
    p->singleErrors.clear();
    p->groupErrors.clear();
    p->groupMap.clear();
    clearLUT();
}

void ErrorValues::setGroupError(const Word &groupLetters, const int error) {
    size_t newGroupID = p->groupErrors.size();
    p->groupErrors.push_back(error);
    for(size_t i = 0; i < groupLetters.length(); i++) {
        Letter curLetter = groupLetters[i];
        if(isInGroup(curLetter)) {
            if(p->groupMap.find(curLetter)->second != newGroupID)
                throw runtime_error("Tried to add letter to two different error groups.");
        } else {
            p->groupMap[curLetter] = newGroupID;
        }
    }
    addGroupErrorToLUT(groupLetters, error);

    debugMessage("Added error group: %s\n", groupLetters.asUtf8().c_str());
}

void ErrorValues::addGroupErrorToLUT(const Word &groupLetters, const int error) {
    for(size_t i=0; i<groupLetters.length(); i++) {
        for(size_t j=i; j<groupLetters.length(); j++) {
            Letter l1 = groupLetters[i];
            Letter l2 = groupLetters[j];
            if(l1 == l2)
                continue;
            addToLUT(l1, l2, error);
        }
    }
}

bool ErrorValues::isInGroup(Letter l) {
    return p->groupMap.find(l) != p->groupMap.end();
}

void ErrorValues::addAccents(accentGroups group) {
    const char *baseName = accentGroupDataFile[group];
    string dataFile = findDataFile(baseName);
    string line;
    if(dataFile.length() == 0) {
        string s = "Could not find file ";
        s += baseName;
        s += ". Run make install or set COLUMBUS_DATADIR env var to your data directory.";
        throw runtime_error(s);
    }
    ifstream ifile(dataFile.c_str());
    if(ifile.fail()) {
        string s = "Could not open data file ";
        s += dataFile;
        throw runtime_error(s);
    }
    while(getline(ifile, line)) {
        Word group(line.c_str());
        if(group.length() == 0)
            continue;
        setGroupError(group, getDefaultGroupError());
    }
}

void ErrorValues::addKeyboardErrors() {
    int error = getDefaultTypoError();
    // Yes, this is a Finnish keyboard.
    const Letter line1[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '+'};
    const Letter line2[] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 0xe5};
    const Letter line3[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0xf6, 0xe4, '\''};
    const Letter line4[] = {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-'};

    const Letter *keyboard_layout[4] = {line1, line2, line3, line4};
    const size_t lineLens[] = {11, 11, 12, 10};
    for(size_t i = 0; i < 3; i++) {
        const Letter *cur_row = keyboard_layout[i];
        const Letter *next_row = keyboard_layout[i+1];
        for(size_t j1=0; j1 < lineLens[i]; j1++) {
            Letter l1 = cur_row[j1];
            if(j1 + 1 < lineLens[i])
                setError(l1, cur_row[j1+1], error);
            if(j1 > 0 && j1-1 < lineLens[i+1])
                setError(l1, next_row[j1-1], error);
            if(j1 < lineLens[i+1])
                setError(l1, next_row[j1], error);
        }
    }


}

void ErrorValues::setPadError(const Letter number, const char letters[4], int letterCount, int error) {
    assert(number >= '0' && number <= '9');
    for(int i=0; i<letterCount; i++) {
        assert(letters[i] != '?');
        setError(number, letters[i], error);
    }
}

void ErrorValues::addNumberpadErrors() {
    const int sameButton = 0;
    const int adjacentButton = 50;
    const int w = 3;
    const int h = 3;

    const char padLetters[h][w][4] = {
    {{'?', '?', '?', '?'}, {'a', 'b' ,'c', '?'}, {'d', 'e', 'f', '?'}},
    {{'g', 'h', 'i', '?'}, {'j', 'k', 'l', '?'}, {'m', 'n', 'o', '?'}},
    {{'p', 'q', 'r', 's'}, {'t', 'u', 'v', '?'}, {'w', 'x', 'y', 'z'}},
    };
    const char padNumbers[h][w] = {
            {'1', '2', '3'},
            {'4', '5', '6'},
            {'7', '8', '9'}
    };
    const int letterCount[h][w] = {
            {0, 3, 3},
            {3, 3, 3},
            {4, 3, 4}
    };
    //const int diagonalButton = 80;
    for(int j=0; j<h; j++) {
        for(int i=0; i<w; i++) {
            setPadError(padNumbers[j][i], padLetters[j][i], letterCount[j][i], sameButton);
            if(i-1 > 0)
                setPadError(padNumbers[j][i], padLetters[j][i-1], letterCount[j][i-1], adjacentButton);
            if(i+1 < w)
                setPadError(padNumbers[j][i], padLetters[j][i+1], letterCount[j][i+1], adjacentButton);
            if(j-1 > 0)
                setPadError(padNumbers[j][i], padLetters[j-1][i], letterCount[j-1][i], adjacentButton);
            if(j+1 < h)
                setPadError(padNumbers[j][i], padLetters[j+1][i], letterCount[j+1][i], adjacentButton);
        }
    }
}

void ErrorValues::addStandardErrors() {
    addAccents(latinAccentGroup);
    addAccents(greekAccentGroup);
    addKeyboardErrors();
}

void ErrorValues::addToLUT(Letter l1, Letter l2, int value) {
    if(l1 < LUT_LETTERS && l2 < LUT_LETTERS) {
        p->lut[LUT_OFFSET(l1, l2)] = value;
        p->lut[LUT_OFFSET(l2, l1)] = value;
    }
}

void ErrorValues::setSubstringMode() {
    startInsertionError = getSubstringDefaultStartInsertionError();
    endDeletionError = getSubstringDefaultEndDeletionError();
    substringStartLimit = DEFAULT_SUBSTRING_START_LENGTH;
}


COL_NAMESPACE_END
