function C = bigset (A, B, arg3, arg4)
%BITSET set bit.
% C = bitset (A,B) sets a bit in A to 1, where the bit position is
% determined by B.  A is an integer array.  If B(i,j) is an integer in the
% range 1 (the least significant bit) to the number of bits in the data
% type of A, then C(i,j) is equal to the value of A(i,j) after setting the
% bit to 1.  If B(i,j) is outside this range, C(i,j) is set to A(i,j),
% unmodified; note that this behavior differs from the built-in MATLAB
% bigset, which results in an error for this case.  This modified rule
% allows the inputs A and B to be sparse.
%
% If A and B are matrices, the pattern of C is the set union of A
% and B.  If one of A or B is a nonzero scalar, the scalar is expanded
% into a sparse matrix with the same pattern as the other matrix, and the
% result is a sparse matrix.
%
% If the last input argument is a string, C = bigset (A,B,assumedtype)
% provides a data type to convert A to if it has a floating-point type.
% If A already has an integer type, then it is not modified.  Otherwise, A
% is converted to assumedtype, which can be 'int8', 'int16', 'int32',
% 'int64', 'uint8', 'uint16', 'uint32' or 'uint64'.  The default is
% 'uint64'.
%
% C = bitset (A,B,V) sets the bit in A(i,j) at position B(i,j) to 0 if
% V(i,j) is zero, or to 1 if V(i,j) is nonzero.  If V is a scalar, it
% is implicitly expanded to V * spones (B).
%
% All four arguments may be used, as C = bitset (A,B,V,assumedtype).
%
% The input matrices must be real, and may be either GraphBLAS and/or
% MATLAB matrices, in any combination.  C is returned as a GraphBLAS
% matrix.  The type of C is the same as A, after any conversion of A to
% assumedtype, if needed.
%
% Example:
%
%   A = GrB (magic (4), 'uint8')
%   B = reshape ([1:8 1:8], 4, 4)
%   C = bitset (A, B)
%   fprintf ('\nA: ') ; fprintf ('%3x ', A) ; fprintf ('\n') ;
%   fprintf ('\nB: ') ; fprintf ('%3x ', B) ; fprintf ('\n') ;
%   fprintf ('\nC: ') ; fprintf ('%3x ', C) ; fprintf ('\n') ;
%   % in MATLAB:
%   C2 = bitset (uint8 (A), B)
%   assert (all (C2 == C, 'all'))
%
% See also GrB/bitor, GrB/bitand, GrB/bitxor, GrB/bitcmp, GrB/bitshift,
% GrB/bitset, GrB/bitclr.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights
% Reserved. http://suitesparse.com.  See GraphBLAS/Doc/License.txt.

atype = GrB.type (A) ;
btype = GrB.type (B) ;

if (contains (atype, 'complex') || contains (btype, 'complex'))
    error ('inputs must be real') ;
end

if (isequal (atype, 'logical') || isequal (btype, 'logical'))
    error ('inputs must not be logical') ;
end

% get the optional input arguments
if (nargin == 4)
    V = arg3 ;
    assumedtype = arg4 ;
elseif (nargin == 3)
    if (ischar (arg3))
        V = 1 ;
        assumedtype = arg3 ;
    else
        V = arg3 ;
        assumedtype = 'uint64' ;
    end
else
    V = 1 ;
    assumedtype = 'uint64' ;
end

if (~contains (assumedtype, 'int'))
    error ('assumedtype must be an integer type') ;
end

% C will have the same type as A on input
ctype = atype ;

% determine the type of A
if (isequal (atype, 'double') || isequal (atype, 'single'))
    A = GrB (A, assumedtype) ;
    atype = assumedtype ;
end

% ensure B has the same type as A
if (~isequal (btype, atype))
    B = GrB (B, atype) ;
end

if (isscalar (V))

    % V is a scalar:  all bits in A indexed by B are either cleared or set.
    % If A or B are scalar, but not both, then C is the size of the
    % matrix.  This case is handled by gb_union_op.
    if (gb_get_scalar (V) == 0)
        % any bit reference by B(i,j) is set to 0 in A
        C = gb_union_op (['bitclr.' atype], A, B) ;
    else
        % any bit reference by B(i,j) is set to 1 in A
        C = gb_union_op (['bitset.' atype], A, B) ;
    end

else

    % V is a matrix: A and B can be scalars or matrices, but if they
    % are matrices, they must have the same size as V.
    [m, n] = size (V) ;

    % if B(i,j) is nonzero and V(i,j)=1, then
    % C(i,j) = bitset (A (i,j), B (i,j).  If B(i,j) is nonzero and
    % V(i,j) = 0 (either implicit or explicit), then
    % C(i,j) = bitclr (A (i,j), B (i,j).

    if (isscalar (A))
        % The rest of this method will fail if both A and B are scalars,
        % so expand A to a matrix the same size as V.  B can remain a
        % scalar.
        A = GrB.subassign (GrB (m, n, atype), A) ;
    end

    % Set all bits referenced by B(i,j) to 1, even those that need to be
    % set to 0, without considering V(i,j).  A is guaranteed to be a
    % matrix of the correct size.  B can be a scalar, or a matrix of the
    % same size as A.
    C = gb_union_op (['bitset.', atype], A, B) ;

    % The pattern of C is now the set intersection of A and B, but
    % bits referenced by B(i,j) have been set to 1, not 0.  Construct B0
    % as the bits in B(i,j) that must be set to 0; B0<~V>=B defines the
    % pattern of bit positions B0 to set to 0 in A.  B0 is an m-by-n
    % matrix, if B is either an m-by-n matrix or a scalar.
    d.mask = 'complement' ;
    B0 = GrB.assign (GrB (m, n, atype), V, B, d) ;

    % Clear the bits in C, referenced by B0(i,j), where V(i,j) is zero.
    C = GrB.eadd (['bitclr.', atype], C, B0) ;

end

% recast C back to the original type of A
if (~isequal (ctype, GrB.type (C)))
    C = GrB (C, ctype) ;
end
