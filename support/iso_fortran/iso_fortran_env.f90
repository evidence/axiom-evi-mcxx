MODULE ISO_FORTRAN_ENV, INTRINSIC

  INTEGER :: CHARACTER_STORAGE_SIZE
  INTEGER :: ERROR_UNIT
  INTEGER :: FILE_STORAGE_SIZE
  INTEGER :: INPUT_UNIT
  INTEGER :: IOSTAT_END
  INTEGER :: IOSTAT_EOR
  INTEGER :: NUMERIC_STORAGE_SIZE
  INTEGER :: OUTPUT_UNIT
  INTEGER :: IOSTAT_INQUIRE_INTERNAL_UNIT

  INTEGER, PARAMETER :: CHARACTER_KINDS(1) = (/ 1 /)

  INTEGER, PARAMETER :: INT8 = 1
  INTEGER, PARAMETER :: INT16 = 2
  INTEGER, PARAMETER :: INT32 = 4
  INTEGER, PARAMETER :: INT64 = 8
  INTEGER, PARAMETER :: INT128 = 16
  INTEGER, PARAMETER :: INTEGER_KINDS(4) = (/ INT8, INT16, INT32, INT64, INT128 /)

  INTEGER, PARAMETER :: LOGICAL_KINDS(4) = INTEGER_KINDS

  INTEGER, PARAMETER :: REAL32 = 4
  INTEGER, PARAMETER :: REAL64 = 8
  INTEGER, PARAMETER :: REAL128 = 16
  INTEGER, PARAMETER :: REAL_KINDS(3) = (/ REAL32, REAL64, REAL128 /)

  ! These are here for completness, Mercurium does not support ATOMIC types, LOCKS or COARRRAYS
  INTEGER, PARAMETER :: ATOMIC_INT_KIND = 4
  INTEGER, PARAMETER :: ATOMIC_LOGICAL_KIND = 4

  INTEGER :: STAT_LOCKED
  INTEGER :: STAT_LOCKED_OTHER_IMAGE
  INTEGER :: STAT_STOPPED_IMAGE
  INTEGER :: STAT_UNLOCKED

  TYPE LOCK_TYPE
      PRIVATE
      INTEGER :: DUMMY
  END TYPE LOCK_TYPE

  CONTAINS
      FUNCTION COMPILER_VERSION() RESULT(STR)
          CHARACTER(LEN=100) :: STR
          STR = "Mercurium Fortran"
      END FUNCTION

END MODULE ISO_FORTRAN_ENV

