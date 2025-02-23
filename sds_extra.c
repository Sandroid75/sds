/* SDSLib 2.1 -- A C dynamic strings library extension
 *
 * Copyright (c) 2006-2015, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2015, Oran Agra
 * Copyright (c) 2015, Redis Labs, Inc
 * Copyright (c) 1990-2021, Sandroid75
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sds.h"
#include "sds_extra.h"

/* The sdssds() function finds the first occurrence of the substring needle
 * in the string haystack. The terminating null bytes '\0' are not compared.
 *
 * Return value:
 *
 *     positive if a match was found.
 *     negative if no match found.
 *     0 if haystack and needle are exactly the same string.
 */
int sdssds(const sds haystack, const sds needle) {
    size_t needle_len, haystack_len, ptr_len;
    char *ptr, chr;

    haystack_len = sdslen(haystack);
    needle_len = sdslen(needle);

    if (!haystack_len || !needle_len || haystack_len < needle_len) { // if one or both of two sds are empty return -1 also if haystack lenght is less than needle lenght
        return -1;                                                   // also if both are 0 lenght for me there is no string to compare
    }

    if (haystack_len == needle_len) {               // check if haystack and needle have the same lenght
        return (sdscmp(haystack, needle) ? -1 : 0); // return 0 if are the same otherwise -1
    }

    chr = needle[0];                                                                                // get the first char of needle
    for (ptr = memchr(haystack, chr, haystack_len); ptr != NULL; ptr = memchr(ptr, chr, ptr_len)) { // initializing for with the first occurrence of chr; check if is valid ptr; continuing with next occurrence
        if (!memcmp(ptr, needle, needle_len)) {                                                     // compare the ptr with match for needle lenght
            return 1;                                                                               // a match was found!
        }
        ptr++;                 // increase one char to step over the previous founded char
        ptr_len = strlen(ptr); // get the new lenght of ptr
    }

    return -1; // no match found
}

/* The sdscasesds() function finds the first occurrence of the substring needle
 * in the string haystack. The terminating null bytes '\0' are not compared.
 * This function ignore the case of both sds strings arguments.
 * Return value:
 *
 *     positive if a match was found.
 *     negative if no match found.
 *     0 if haystack and needle are exactly the same string.
 */
int sdscasesds(const sds haystack, const sds needle) {
    sds string, match;
    int result;

    string = sdsdup(haystack); // duplicate haystack
    match = sdsdup(needle);    // duplicate needle
    sdstolower(string);        // convert to lovwercase string
    sdstolower(match);         // convert to lowercase match

    result = sdssds(string, match); // all is lowercase than can call sdssds

    sdsfree(string); // destroy the sds
    sdsfree(match);  // destroy the sds

    return result; // no match found
}

/* The sdschremove() function remove all single char occurrence
 * of the substring cset from the string s.
 * The terminating null bytes '\0' are not compared.
 * This function is case sensitive.
 * Return value:
 *
 *     s string pointer
 */
sds sdschremove(sds s, char *cset) {
    char *ptr, *src, *dst, *purgedstr;
    size_t newlen = 0, len = sdslen(s);

    if (!len || !cset || !strlen(cset)) { // check if there is something to remove
        return s;
    }

    purgedstr = (char *)malloc(sdslen(s) + 1); // allocate memory for new string
    dst = purgedstr;                           // dst point to new empty string
    src = s;                                   // src point to original sds string

    for (int i = 0; i < len; i++) {                // walk to all binary 's' sds string
        for (ptr = cset; *ptr && i < len; ptr++) { // walk to all NULL terminated 'cset' string and up to the end of binary sds
            if (*src == *ptr) {
                src++;      // current ch is present in cset than skip to next
                i++;        // increment the index of binary sds
                ptr = cset; // reset the ptr pointer to beguin of cset string
            }
        }
        if (i < len) {   // check if the sds binary string is over
            *dst = *src; // the current char of scr is not present in cset string
            dst++;       // increment the dst pointer
            src++;       // increment the src pointer
            newlen++;    // increment the count of newlen
        }
    }

    sdsfree(s);                       // destroy original sds binary string
    s = sdsnewlen(purgedstr, newlen); // create the ner sds binary string purged from 'cset' char
    if (purgedstr) {                  // check purgedstr
        free(purgedstr);              // release the memory
    }

    return s;
}

/* The sdscasechremove() function remove all single char occurrence
 * of the substring cset from the string s.
 * The terminating null bytes '\0' are not compared.
 * This function ignore case.
 * Return value:
 *
 *     s string pointer
 */
sds sdscasechremove(sds s, char *cset) {
    char *ignorecase;
    size_t len = strlen(cset);
    int i;

    ignorecase = (char *)malloc(len + 1); // allocate memory for ignorecase string

    for (i = 0; i < len; i++) {
        ignorecase[i] = tolower(cset[i]); // convert cset to lowercase
    }
    ignorecase[len] = '\0';
    s = sdschremove(s, ignorecase); // call the sdschremove function with lowercase

    for (i = 0; i < len; i++) {
        ignorecase[i] = toupper(cset[i]); // convert cset to uppercase
    }
    ignorecase[len] = '\0';
    s = sdschremove(s, ignorecase); // call the sdschremove function with uppercase

    if (ignorecase) {     // check ignorecase
        free(ignorecase); // release the memory
    }

    return s;
}
