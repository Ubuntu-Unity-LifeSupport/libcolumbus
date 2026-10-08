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
 * This file contains various library helper functions.
 */

#include "ColumbusHelpers.hh"
#include "Word.hh"
#include "WordList.hh"
#include <iconv.h>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <stdexcept>
#include <cstring>
#include <sys/time.h>
#include <unicode/uchar.h>
#include <cstdlib>

COL_NAMESPACE_START

static const Letter whitespaceLetters[] = {' ', '\t', '\n', '\r', '\0'};
static const int numWhitespaceLetters = 4;

static Letter lowerLetter(Letter l) {
    return Letter(u_tolower(l)); // Have to use ICU because towlower libc function does not work.
}

Letter* utf8ToInternal(const char *utf8Text, unsigned int &resultStringSize) {
    iconv_t ic = iconv_open(INTERNAL_ENCODING, "UTF-8");
    char *tmp;
    char *txt;
    char *inBuf;
    char *outBuf;
    size_t badConvertedCharacters;
    size_t inBytes, outBytes, outBytesOriginal;
    size_t bytesWritten;
    if (ic == (iconv_t)-1) {
        throw std::runtime_error("Could not create iconv converter.");
    }

    unsigned int inputLen = strlen((const char*)(utf8Text));
    txt = (char*)new Letter[inputLen+1];
    tmp = strdup((const char*)(utf8Text)); // Iconv should take a const pointer but does not. Protect against it screwing up.
    assert(tmp);
    inBytes = inputLen;
    outBytes = sizeof(Letter)*(inBytes+1);
    outBytesOriginal = outBytes;

    inBuf = tmp;
    outBuf = txt;
    badConvertedCharacters = iconv(ic, &inBuf, &inBytes, &outBuf, &outBytes);
    free(tmp);
    iconv_close(ic);
    if(badConvertedCharacters == (size_t)-1) {
        std::string err("Could not convert UTF8-string to internal representation: ");
        err += (const char*)(utf8Text);
        throw std::runtime_error(err);
    }
    bytesWritten = outBytesOriginal - outBytes;
    resultStringSize = bytesWritten/sizeof(Letter);

    if(bytesWritten < inputLen) {
        // Shrink allocated memory size to exactly the produced string.
        size_t newArraySize = bytesWritten + sizeof(Letter);
        char *newtxt = new char[newArraySize];
        memcpy(newtxt, txt, newArraySize);
        delete []txt;
        txt = newtxt;
    }
    Letter* text = reinterpret_cast<Letter*>(txt);
    text[resultStringSize] = 0; // Null terminated.
    // Now convert all letters to lower case, because we don't care about case difference when matching.
    for(size_t i=0; i<resultStringSize; i++) {
        text[i] = lowerLetter(text[i]);
    }
    return text;
}

void internalToUtf8(const Letter* source, unsigned int characters, char *buf, unsigned int bufsize) {
    iconv_t ic = iconv_open("UTF-8", INTERNAL_ENCODING);
    char *inBuf = reinterpret_cast<char*>(const_cast<Letter*>(source));
    char *outBuf;
    size_t badConvertedCharacters;
    size_t inBytes, outBytes, outBytesOriginal, resultStringSize;
    if (ic == (iconv_t)-1) {
        throw std::runtime_error("Could not create iconv converter.");
    }

    inBytes = characters*sizeof(Letter);
    outBytes = bufsize;
    outBytesOriginal = outBytes;

    outBuf = buf;
    badConvertedCharacters = iconv(ic, &inBuf, &inBytes, &outBuf, &outBytes);
    iconv_close(ic);
    if(badConvertedCharacters == (size_t)-1) {
        throw std::runtime_error("Could not convert internal string to UTF-8.");
    }
    resultStringSize = outBytesOriginal - outBytes;
    buf[resultStringSize] = 0; // Null terminated, just in case.
}

double hiresTimestamp() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec + now.tv_usec/1000000.0;

}

WordList splitToWords(const char *utf8Text) {
    return split(utf8Text, whitespaceLetters, numWhitespaceLetters);
}

static bool isInList(const Letter l, const Letter *chars, int numChars) {
    for(int i=0; i<numChars;i++)
        if(chars[i] == l)
            return true;
    return false;
}

WordList split(const char *utf8Text, const Letter *splitChars, int numChars) {
    WordList list;
    unsigned int strSize = strlen(utf8Text);
    size_t begin, end;
    end = 0;
    char *word = 0;
    unsigned int bufSize = 0;

    do {
        begin = end;
        while(isInList(utf8Text[begin], splitChars, numChars) && begin < strSize) {
            begin++;
        }
        if(begin >= strSize) {
            delete []word;
            return list;
        }
        end = begin+1;
        while(!isInList(utf8Text[end], splitChars, numChars) && end < strSize) {
            end++;
        }
        // End points to one past the last letter.
        unsigned int wordLen = end-begin;
        if(wordLen +1 > bufSize) {
            delete[] word;
            word = new char[wordLen+1];
            bufSize = wordLen + 1;
        }
        memcpy(word, utf8Text+begin, wordLen);
        word[wordLen] = '\0';
        try {
            Word w(word);
            list.addWord(w);
        } catch(std::invalid_argument &ex) {
            delete []word;
            throw ex;
        }
    } while(end < strSize);
    delete []word;
    return list;
}

bool isWhitespace(Letter l) {
    return isInList(l, whitespaceLetters, numWhitespaceLetters);
}

COL_NAMESPACE_END
