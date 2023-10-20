////////////////////////////////////////////////////////////////////////////////
//////////////////////////  paru_intersection //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// ParU, Copyright (c) 2022, Mohsen Aznaveh and Timothy A. Davis,
// All Rights Reserved.
// SPDX-License-Identifier: GNU GPL 3.0

/*! @brief  finding the number of intersection between new columns and piror
 * block columns
 *
 *  @author Aznaveh
 */
#include "paru_internal.hpp"
#define Const 4

int paru_intersection(int64_t e, paru_element **elementList,
                      std::set<int64_t> &stl_newColSet)
{
    DEBUGLEVEL(0);
    PARU_DEFINE_PRLEVEL;
#ifndef NDEBUG
    PRLEVEL(PR, ("%%stl_newColSet:\n%%"));
    for (std::set<int64_t>::iterator it = stl_newColSet.begin();
         it != stl_newColSet.end(); it++)
    {
        PRLEVEL(PR, (" %ld", *it));
    }
    PRLEVEL(PR, ("\n"));

#endif

    paru_element *el = elementList[e];

    int64_t nEl = el->ncols;
    int64_t *el_colIndex = (int64_t *)(el + 1);

    // find the intersection between columns of e and
    int64_t intersection = 0;
    // conditions for early stop
    if (el_colIndex[el->lac] > *stl_newColSet.end()) 
        return 0;

    PRLEVEL(PR, ("%% newColSet.size = %ld\n", stl_newColSet.size()));
    PRLEVEL(PR, ("%% nEl = %ld\n", nEl));
    std::set<int64_t>::iterator it;
    if ((int64_t)(Const * stl_newColSet.size()) < nEl - el->lac)
    // if size el >> stl_newColSet
    //   binary search each of elements in stl_newColSet in el
    //   log(nEl)*stl_newColSet.size()
    {
        PRLEVEL(PR, ("%%el >> stl_newColSet\n"));
        for (it = stl_newColSet.begin(); it != stl_newColSet.end(); it++)
        {
            int64_t c = *it;
            int64_t col = paru_bin_srch_col(el_colIndex, el->lac, nEl - 1, c);
            PRLEVEL(PR, ("%%intersection=%ld", intersection));
            PRLEVEL(PR, ("%%after bsearch for c=%ld col=%ld \n", c, col));
            if (col != -1 && el_colIndex[col] == c)
            {
                intersection++;
                PRLEVEL(PR, ("%%##1: c=%ld ", c));
                PRLEVEL(PR, ("%%intersection=%ld\n", intersection));
            };
        }
    }
    else if ((int64_t)stl_newColSet.size() > Const * (nEl - el->lac))
    {  //  else if stl_newColSet >> el
        PRLEVEL(PR, ("%%el << stl_newColSet\n"));
        //      binary search each of elements in el in stl_newColSet
        //      log(stl_newColSet.size())*nEl
        int64_t ncolsseen = el->ncolsleft;
        for (int64_t c = el->lac; c < nEl; c++)
        {
            int64_t col = el_colIndex[c];
            if (col < 0) continue;
            ncolsseen--;
            if (stl_newColSet.find(col) != stl_newColSet.end())
            {
                intersection++;
                PRLEVEL(PR, ("%%2: col=%ld ", col));
                PRLEVEL(PR, ("%%intersection=%ld\n", intersection));
            };
            if (ncolsseen == 0) return intersection;
        }
    }
    else
    {  // Merge style m+n
        PRLEVEL(PR, ("%%Merge style\n"));
        it = stl_newColSet.begin();
        int64_t c = el->lac;
        int64_t ncolsseen = el->ncolsleft;
        while (it != stl_newColSet.end() && ncolsseen > 0)
        {
            while (el_colIndex[c] < 0 && c < nEl) ++c;  // skip dead columns
            if (c >= nEl) break;

            if (*it < el_colIndex[c])
                it++;
            else if (el_colIndex[c] < *it)
            {
                c++;
                ncolsseen--;
            }
            else
            // *it == col
            {
                intersection++;
                PRLEVEL(PR, ("%%3: c=%ld ", c));
                PRLEVEL(PR, ("%%col= %ld", el_colIndex[c]));
                PRLEVEL(PR, ("%%intersection=%ld\n", intersection));
                it++;
                c++;
                ncolsseen--;
            };
        }
    }
    PRLEVEL(PR, (" e = %ld intersection= %ld\n", e, intersection));
    return intersection;
}