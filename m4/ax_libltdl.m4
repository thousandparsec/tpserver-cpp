
AC_DEFUN([AX_LIBTOOL2],
[
	  m4_ifdef([LT_INIT], 
	           [AC_DEFINE([USE_LIBTOOL2], [], [Define whether application use libtool >= 2.0])], 
	           []) 

])
