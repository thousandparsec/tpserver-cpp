(define-syntax make-component_list-type
  (syntax-rules ()
    [(make-component_list-type len) 
     (make-struct-type 'component_list #f 0 len 0)
     ]
    )
  )
(define-syntax make-component_list-accessor
  (syntax-rules ()
    [(make-component_list-accessor component_list-ref id name)
     (make-struct-field-accessor component_list-ref (- id 1) (string->symbol name))
     ]
    )
  )
(define-syntax make-component_list-setter
  (syntax-rules ()
    [(make-component_list-setter component_list-set id name)
     (make-struct-field-mutator component_list-set (- id 1) (string->symbol name))
     ]
    )
  )