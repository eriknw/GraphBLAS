//------------------------------------------------------------------------------
// GxB_Matrix_export_FullR: export a full matrix, held by row
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_export.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Matrix_export_FullR  // export and free a full matrix, by row
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix
    void **Ax,          // values, size nrows*ncols
    const GrB_Descriptor desc
)
{

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Matrix_export_FullR (&A, &type, &nrows, &ncols, &Ax,"
        " desc)") ;
    GB_BURBLE_START ("GxB_Matrix_export_FullR") ;
    GB_RETURN_IF_NULL (A) ;
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6) ;

    //--------------------------------------------------------------------------
    // finish any pending work
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT (*A) ;
    if (!GB_is_dense (*A))
    { 
        // A must be dense or full
        return (GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // ensure the matrix is full CSR
    //--------------------------------------------------------------------------

    // ensure the matrix is in CSR format
    if ((*A)->is_csc)
    {
        // A = A', done in place, to put A in CSR format
        GBURBLE ("(transpose) ") ;
        GB_OK (GB_transpose (NULL, NULL, false, *A,
            NULL, NULL, NULL, false, Context)) ;
    }

    GB_convert_any_to_full (*A) ;
    ASSERT (GB_IS_FULL (*A)) ;

    //--------------------------------------------------------------------------
    // export the matrix
    //--------------------------------------------------------------------------

    info = GB_export (A, type, ncols, nrows, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, Ax, NULL, NULL, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}
