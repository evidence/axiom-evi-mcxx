! <testinfo>
! test_generator=(config/mercurium-ompss "config/mercurium-ompss-v2 openmp-compatibility")
! </testinfo>
PROGRAM P
    IMPLICIT NONE
    INTEGER, PARAMETER :: N = 1000
    INTEGER :: V(N)
    INTEGER :: I
    INTEGER :: RES

    DO I=1, N
        V(I) = I
    ENDDO

    RES = 0
    DO I=1, N
        !$OMP TASK REDUCTION(+: RES) SHARED(V) FIRSTPRIVATE(I)
            RES = RES + V(I)
        !$OMP END TASK
    ENDDO

    !$OMP TASK IN(RES)
        IF (RES /= ((N*(N+1))/2)) STOP 1
    !$OMP END TASK

    !$OMP TASKWAIT

END PROGRAM P
