function gbtest65
%GBTEST65 test gb.mis

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
load west0479 ;

ok = true ;
try
    iset = gb.mis (ones (3,4)) ;
catch expected_error
    expected_error
end
assert (ok) ;

ok = true ;
try
    iset = gb.mis (ones (3), 'gunk') ;
catch expected_error
    expected_error
end
assert (ok) ;

ok = true ;
try
    iset = gb.mis (ones (3), 'check') ;
catch expected_error
    expected_error
end
assert (ok) ;

ok = true ;
try
    iset = gb.mis (gb.offdiag (west0479), 'check') ;
catch expected_error
    expected_error
end
assert (ok) ;

ok = true ;
try
    iset = gb.mis (ones (3)) ;
catch expected_error
    expected_error
end
assert (ok) ;

A = gb.offdiag (spones (west0479)) ;
A = A+A' ;

maxisize = 0 ;
n = size (A, 1) ;

for trial = 1:100

    if (mod (trial, 4) == 1)
        iset  = gb.mis (A, 'check') ;
    else
        iset  = gb.mis (A) ;
    end

    % assert that iset is an independent set
    p = find (iset) ;
    assert (nnz (A (p,p)) == 0) ;
    isize = length (p) ;

    if (isize > maxisize)
        fprintf ('trial %3d: iset size: %d\n', trial, isize) ;
        maxisize = isize ;
    end

    % assert that iset is maximal
    q = find (~iset) ;
    d = gb.entries (A (p, q), 'col', 'degree') ;
    assert (all (d > 0)) ;
end

fprintf ('max independent set found: %d of %d nodes\n', maxisize, n) ;

fprintf ('gbtest65: all tests passed\n') ;
