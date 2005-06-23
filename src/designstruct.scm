(define-syntax make-design-type
  (syntax-rules ()
    [(make-design-type len) 
     (make-struct-type 'design #f 2 len 0)
     ]
    )
  )
(define-syntax make-property-accessor
  (syntax-rules ()
    [(make-design-accessor design-ref id name)
     (make-struct-field-accessor design-ref (+ id 1) (string->symbol name))
     ]
    )
  )
(define-syntax make-property-setter
  (syntax-rules ()
    [(make-design-setter design-set id name)
     (make-struct-field-mutator design-set (+ id 1) (string->symbol name))
     ]
    )
  )