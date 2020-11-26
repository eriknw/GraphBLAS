//------------------------------------------------------------------------------
// GB_AxB_dot2_template:  C=A'B, C<!M>=A'*B, or C<M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// TODO: rename GB_bitmap_AxB_dot_template.c

// A and B are sparse, bitmap, or full; never hypersparse.  If the input
// matrices A and/or B are hypersparse, they are packed into sparse matrices,
// and C is unpacked from bitmap to sparse/hypersparse when done.

{

    //--------------------------------------------------------------------------
    // C=A'*B, C<M>=A'*B, or C<!M>=A'*B where C is bitmap
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:cnvals)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        const int a_tid = tid / nbslice ;
        const int b_tid = tid % nbslice ;
        const int64_t kA_start = A_slice [a_tid] ;
        const int64_t kA_end   = A_slice [a_tid+1] ;
        const int64_t kB_start = B_slice [b_tid] ;
        const int64_t kB_end   = B_slice [b_tid+1] ;

        //----------------------------------------------------------------------
        // C=A'*B, C<M>=A'*B, or C<!M>=A'*B via dot products
        //----------------------------------------------------------------------

        for (int64_t j = kB_start ; j < kB_end ; j++)
        {

            //------------------------------------------------------------------
            // get C(:,j)
            //------------------------------------------------------------------

            const int64_t pC_start = j * cvlen ;

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            #if GB_B_IS_SPARSE_OR_HYPER

                // B is sparse (never hypersparse)
                const int64_t pB_start = Bp [j] ;
                const int64_t pB_end   = Bp [j+1] ;
                const int64_t bjnz = pB_end - pB_start ;
                if (bjnz == 0)
                { 
                    // no work to do if B(:,j) is empty, except to clear Cb
                    memset (&Cb [pC_start + kA_start], 0, kA_end - kA_start) ;
                    continue ;
                }

                #if ( GB_A_IS_SPARSE_OR_HYPER )
                    // get the first and last index in B(:,j)
                    const int64_t ib_first = Bi [pB_start] ;
                    const int64_t ib_last  = Bi [pB_end-1] ;
                #endif

            #else

                // B is bitmap or full
                const int64_t pB_start = j * vlen ;

            #endif

            //------------------------------------------------------------------
            // C(:,j)<#M(:,j)> = A'*B(:,j), or C(:,j) = A'*B(:,j) if no mask
            //------------------------------------------------------------------

            for (int64_t i = kA_start ; i < kA_end ; i++)
            {

                //--------------------------------------------------------------
                // get C(i,j), M(i,j), and clear the C(i,j) bitmap
                //--------------------------------------------------------------

                int64_t pC = pC_start + i ;     // C is bitmap

                #if defined ( GB_ANY_SPECIALIZED )

                // M is bitmap and structural; Mask_comp true
                Cb [pC] = 0 ;
                if (!Mb [pC])

                #elif defined ( GB_MASK_IS_PRESENT )

                bool mij ;
                if (M_is_bitmap)
                { 
                    // M is bitmap
                    mij = Mb [pC] && GB_mcast (Mx, pC, msize) ;
                }
                else if (M_is_full)
                { 
                    // M is full
                    mij = GB_mcast (Mx, pC, msize) ;
                }
                else // M is sparse or hyper
                { 
                    // M has been scattered into the C bitmap
                    mij = (Cb [pC] > 1) ;
                }
                Cb [pC] = 0 ;
                if (mij ^ Mask_comp)

                #else

                // M is not present
                Cb [pC] = 0 ;

                #endif
                { 

                    //----------------------------------------------------------
                    // C(i,j) = A(:,i)'*B(:,j)
                    //----------------------------------------------------------

                    #if GB_A_IS_SPARSE_OR_HYPER
                    int64_t pA = Ap [i] ;
                    const int64_t pA_end = Ap [i+1] ;
                    int64_t ainz = pA_end - pA ;
                    if (ainz == 0) continue ;
                    #else
                    const int64_t pA = i * vlen ;
                    #endif
                    #include "GB_AxB_dot2_cij.c"
                }
            }
        }
    }
}

#undef GB_A_IS_SPARSE_OR_HYPER
#undef GB_A_IS_BITMAP
#undef GB_A_IS_FULL
#undef GB_B_IS_SPARSE_OR_HYPER
#undef GB_B_IS_BITMAP
#undef GB_B_IS_FULL

