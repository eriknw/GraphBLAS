% function test22
%TEST21 test reduce to scalar

rng ('default') ;
for trial = 1:40
    fprintf ('.') ;
    for m = 0:10
        for n = 0:10
            A = 100 * sprand (m, n, 0.5) ;
            G = gb (A) ;
            [i j x] = find (A) ;

            c1 = sum (A, 'all') ;
            c2 = gb.reduce ('+', A) ;
            assert (norm (c1-sparse(c2),1) <= 1e-12 * norm (c1,1)) ;

            c1 = pi + sum (A, 'all') ;
            c2 = gb.reduce (pi, '+', '+', A) ;
            assert (norm (c1-sparse(c2),1) <= 1e-12 * norm (c1,1)) ;

            c1 = prod (x, 'all') ;
            c2 = gb.reduce ('*', A) ;
            assert (norm (c1-sparse(c2),1) <= 1e-12 * norm (c1,1)) ;

            c1 = pi + prod (x, 'all') ;
            c2 = gb.reduce (pi, '+', '*', A) ;
            assert (norm (c1-sparse(c2),1) <= 1e-12 * norm (c1,1)) ;

            c1 = max (A, [ ], 'all') ;
            c2 = gb.reduce ('max', A) ;
            if (nnz (A) < m*n)
                c2 = max (full (c2), 0) ;
            end
            assert (norm (c1-sparse(c2),1) <= 1e-12 * norm (c1,1)) ;

            c1 = min (A, [ ], 'all') ;
            c2 = gb.reduce ('min', A) ;
            if (nnz (A) < m*n)
                c2 = min (full (c2), 0) ;
            end
            assert (norm (c1-sparse(c2),1) <= 1e-12 * norm (c1,1)) ;

            B = logical (A) ;
            G = gb (B) ;

            c1 = any (A, 'all') ;
            c2 = gb.reduce ('|.logical', A) ;
            assert (c1 == sparse (c2)) ;

            [i j x] = find (A) ;
            c1 = all (x, 'all') ;
            c2 = gb.reduce ('&.logical', A) ;
            assert (c1 == sparse (c2)) ;

        end
    end
end

fprintf ('\ntest22: all tests passed\n') ;

