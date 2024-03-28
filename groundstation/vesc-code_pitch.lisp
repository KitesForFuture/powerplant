(define generatormode 0)
(define motormode 1)
(define mode generatormode)

(define counter 10)

(define max-reel-out-tension 10)

(define offset (get-dist))


(loopwhile t
    (progn
        (define line-length (- 0 (- (get-dist) offset) ) )
        (define counter (- counter 1))
        (if (< counter 0) (define counter 0) 0)
        
        (if (= mode generatormode)
            (progn
                (progn
                    (print "generating") 
                    (set-brake 40) ; GENERATING max-reel-out-tension
                )
                
                ; SWITCHING TO REEL-IN
                (if (and (> (get-current) -0.05) (= counter 0))
                    (progn
                        (define mode motormode)
                        (define counter 50)
                    )
                )
            )
            
            (progn
                (progn
                    ; REEL-IN WITH LOW TENSION
                    
                    (if (< line-length 0)
                        (set-current 0)
                        (set-current 3.5)
                    )
                    
                    ; SWITCHING TO REEL-OUT
                    (if (and (< (get-duty) 0.03) (= counter 0))
                        (progn
                            ;(set-current 0)
                            (define mode generatormode)
                            (define counter 10)
                        )
                    )
                )
            )
        )
        
        (sleep 0.005)
    )
)
