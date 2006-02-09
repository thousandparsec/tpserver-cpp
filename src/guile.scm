#!/usr/bin/guile -s
!#

;;
;; class-root
;;
;; the design-type class object.  class-root is a vtable vtable
;;
(define class-root (make-vtable-vtable "pw" 0))


;;
;; make-design-vtable
;;
;; function to create a ship design.
;;
;; Invoking this guy produces a vtable object - an object that describes a
;; structure, not the structure itself.
(define (make-design-vtable design-name)
    (make-struct class-root 0
	(make-struct-layout design-name)
	(lambda (design port)
	    (format port "a stellar design for a ~A~%"
		    (ship-design-name design)))
	design-name 'foo 'bar 'bee)
)



