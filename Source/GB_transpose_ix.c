//------------------------------------------------------------------------------
// GB_transpose_ix: transpose the values and pattern of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The values of A are typecasted to C->type, the type of the C matrix.
// A can be sparse or hypersparse, but C is not hypersparse.

// This method is parallel, but not highly scalable.  It uses only naslice =
// nnz(A)/(A->vlen) threads.

#include "GB_transpose.h"
#ifndef GBCOMPACT
#include "GB_unop__include.h"
#endif

void GB_transpose_ix            // transpose the pattern and values of a matrix
(
    GrB_Matrix C,                       // output matrix
    const GrB_Matrix A,                 // input matrix
    int64_t *GB_RESTRICT *Rowcounts,    // Rowcounts [naslice]
    const int64_t *GB_RESTRICT A_slice, // defines how A is sliced
    int naslice                         // # of slices of A
)
{ 

    GrB_Info info ;
    GrB_Type ctype = C->type ;
    GB_Type_code code1 = ctype->code ;          // defines ztype
    GB_Type_code code2 = A->type->code ;        // defines atype

    //--------------------------------------------------------------------------
    // built-in worker: transpose and typecast
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_unop_tran(zname,aname)                           \
            GB_unop_tran__identity ## zname ## aname

        #define GB_WORKER(ignore1,zname,ztype,aname,atype)          \
        {                                                           \
            info = GB_unop_tran (zname,aname)                       \
                (C, A, Rowcounts, A_slice, naslice) ;               \
            if (info == GrB_SUCCESS) return ;                       \
        }                                                           \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        // switch factory for two built-in types, controlled by code1 and code2
        #include "GB_2type_factory.c"

    #endif

    //--------------------------------------------------------------------------
    // generic worker: transpose and typecast
    //--------------------------------------------------------------------------

    GB_BURBLE_MATRIX (A, "generic ") ;

    size_t asize = A->type->size ;
    size_t csize = C->type->size ;
    GB_cast_function cast_A_to_X = GB_cast_factory (code1, code2) ;

    // Cx [pC] = (ctype) Ax [pA]
    #define GB_CAST_OP(pC,pA)  \
        cast_A_to_X (Cx +(pC*csize), Ax +(pA*asize), asize) ;

    #define GB_ATYPE GB_void
    #define GB_CTYPE GB_void

    #define GB_PHASE_2_OF_2
    #include "GB_unop_transpose.c"
}

