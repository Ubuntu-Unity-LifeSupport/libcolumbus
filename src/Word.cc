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
#include <cstring>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include "ColumbusHelpers.hh"

using namespace std;

COL_NAMESPACE_START

static_assert(sizeof(size_t) <= sizeof(uint64_t), "Wow, you are running a 128 bit platform. Respect!");
static const int randomArrSize = 256;
const static uint64_t randomNumbers[randomArrSize] = {
0x31d04490f9cd0152, 0xcfd220f4878a1427, 0x9b2dd113758d9e8a, 0x35a4419e88a812d5, 0x9f9743e9ee40cd55, 0x7038be807e85f27f, 0x9ca0e3499edabe60, 0x9b3e409e7ffbe39f,
0xc58155e5a1e164e0, 0x1f3f0823c9670283, 0xddc1ff4e8431766f, 0xf708145c12c3a474, 0x1bd343edebb746e8, 0x59363d26f1d34003, 0xade2044c51ce1ab5, 0x86c0607a613fa4e6,
0x4751cef8b5647cf1, 0x618cdd1beaba96a6, 0x9a5616eed71a1b05, 0x90fffcf56ab61b54, 0xc7b408b8542bf4f9, 0x64d8fba24eed76cd, 0x483d04576118f39b,  0x5c9534dee689698,
0x25d7939c3cf11b2d, 0xe020bdf2ba9f78f5, 0xf441f807c4808932, 0x993166a178ddade4, 0x51c7de16e4a0e2bb,  0xa89b70521c0b028,  0x9b3f7f5af8b2f82, 0x6985efce9aa164a7,
0x692607c787097f9c, 0x6afaf7e9f5ee3211, 0xfa34657c280407b4, 0xa160382b0e3e03ec, 0xe8902b92a6dd18c4,  0x7cd35c609f728a7, 0xdd7ac1ab0ce338f3, 0xa7a9e144792de8b4,
0x435dc2030e1bd3bb, 0xba03839edae53f8c, 0x74918b9786b2ecf6,  0x183041d61d4e02d, 0xaa1dc5c7c7c5fb5b, 0x939564fc52bece9b, 0x3a3faae9201160d0, 0xc20d3f67a52cb7b6,
0x77ad9b3c19bda0f9, 0x65696731011860b4,  0xae6b011d726f2fe, 0xba5217bd2b48005f, 0x8f8e100ae6ba4e9d, 0x51967f54690c822d, 0x261a8bf80c1d6890, 0x58cb529d19f0856f,
0xc45e7d76ca927907, 0xc15c5589af3dbef0, 0xa8175814c7ff20f6, 0xaec21b2f3fddfc14,  0xaf247b61fd25583, 0x2d784f3af2691077, 0x58f3a2b1743759c6, 0x77115ac165a120a9,
};

Word::Word() : text(0), len(0){
}

Word::Word(const char *utf8Word) : text(0), len(0) {
    convertString(utf8Word);
}

Word::Word(const Word &w) : text(0), len(0) {
    duplicateFrom(w);
}

Word::Word(Word &&w) :
    text(w.text),
    len(w.len) {
    w.len = 0;
    w.text = 0;
}

Word::Word(Letter *letters, size_t length) {
    if(letters[length-1] == 0) {
        text = new Letter[length];
        len = length-1;
    } else {
        text = new Letter[length+1];
        text[length] = 0;
        len = length;
    }
    memcpy(text, letters, length*sizeof(Letter));
    if(hasWhitespace()) {
        delete []text;
        text = nullptr;
        throw std::invalid_argument("Tried to create a Word with whitespace.");
    }
}

Word::Word(const std::string &w) : text(0), len(0) {
    convertString(w.c_str());
}

Word::~Word() {
    delete []text;
}

void Word::convertString(const char *utf8Word) {
    text = utf8ToInternal(utf8Word, len);
    if(hasWhitespace()) {
        delete []text;
        text = nullptr;
        std::string err("Tried to create a word with whitespace in it: ");
        err += (const char*)utf8Word;
        throw std::invalid_argument(err);
    }
}

void Word::duplicateFrom(const Word &w) {
    if(this == &w) {
        return;
    }
    delete []text;
    len = w.len;
    if(len == 0) {
        text = 0;
    } else {
        text = new Letter[len+1];
        memcpy(text, w.text, (len+1)*sizeof(Letter));
    }
}

Letter Word::operator[](unsigned int i) const {
    if(i >= len) {
        std::string msg("Tried to access letter ");
        msg += i;
        msg += " in a word of size ";
        msg += len;
        msg += ".";
        throw std::out_of_range(msg);
    }
    return text[i];
}

Word& Word::operator=(const Word &w) {
    duplicateFrom(w);
    return *this;
}

Word& Word::operator=(Word &&w) {
    delete []text;

    text = w.text;
    len = w.len;

    w.text = 0;
    w.len = 0;
    return *this;
}

/**
 * A word is not supposed to have any whitespace in it. Verify that we don't.
 */
bool Word::hasWhitespace() {
    for(unsigned int i=0; i<len; i++) {
        Letter cur = text[i];
        if(isWhitespace(cur))
            return true;
    }
    return false;
}

bool Word::operator==(const Word &w) const {
    if(this == &w)
        return true;
    if(len != w.len)
        return false;
    for(unsigned int i=0; i<len; i++)
        if(text[i] != w.text[i])
            return false;
    return true;
}

bool Word::operator==(const char *utf8Word) const {
    return strcmp(asUtf8().c_str(), utf8Word) == 0;
}

bool Word::operator==(const string &utf8Str) const {
    return *this == utf8Str.c_str();
}


bool Word::operator!=(const Word &w) const {
    return !(*this == w);
}

bool Word::operator!=(const char *utf8Word) const {
    return !(*this == utf8Word);
}

bool Word::operator!=(const string &utf8Str) const {
    return !(*this == utf8Str);
}

bool Word::operator<(const Word &w) const {
    if(this == &w)
        return false;
    size_t minLen = len < w.len ? len : w.len;
    for(size_t i=0; i<minLen; i++) {
        if(text[i] < w.text[i])
            return true;
        if(text[i] > w.text[i])
            return false;
    }
    if(w.len > len)
        return true;
    return false;
}

void Word::toUtf8(char *buf, unsigned int bufSize) const {
    internalToUtf8(text, len, buf, bufSize);
}

string Word::asUtf8() const {
    size_t strSize = 4*(len+1); // One codepoint is max 4 bytes in UTF-8.
    char *u8 = new char[strSize];
    toUtf8(u8, strSize);
    string result(u8);
    delete []u8;
    return result;
}

Word Word::join(const Word &w) const {
    Word result;
    size_t newLen = length() + w.length();
    result.len = newLen;
    result.text = new Letter[newLen+1];
    memcpy(result.text, text, len*sizeof(Letter));
    memcpy(result.text + len, w.text, w.len*sizeof(Letter));
    result.text[newLen] = '\0';
    return result;
}

Word& Word::operator=(const char *utf8Word) {
    delete []text;
    text = nullptr;
    len = 0;
    convertString(utf8Word);
    return *this;
}

Word& Word::operator=(const string &utf8Str) {
    return *this = utf8Str.c_str();
}

size_t Word::hash() const {
    size_t result = 0;
    const size_t *nums = (const size_t*) randomNumbers;
    unsigned char *arr = (unsigned char*) text;
    for(size_t i=0; i<len*sizeof(Letter); i++) {
        uint8_t index = arr[i];
        index += (uint8_t) i;
        result ^= nums[index];
    }
    return result;
}

COL_NAMESPACE_END
