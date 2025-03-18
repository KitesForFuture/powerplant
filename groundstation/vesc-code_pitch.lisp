(define generatormode 0)
(define motormode 1)
(define mode generatormode)

(define counter 10)

(define max-reel-out-tension 10)

(define offset (get-dist))

(define current 0)
(define oldcurrent 0)


(loopwhile t
    (progn
        (define line-length (- 0 (- (get-dist) offset) ) )
        (define counter (- counter 1))
        (if (< counter 0) (define counter 0) 0)
        
        (if (= mode generatormode)
            (progn
                (progn
                    (print "generating") 
                    (if (< line-length 200)
                    	(set-brake 20) ; GENERATING max-reel-out-tension
                    	(set-brake 50)
                    )
                )
                
                ; SWITCHING TO REEL-IN
                (define oldcurrent current)
                (define current (get-current))
                (if (and (and (> current -0.05) (> oldcurrent -0.05)) (= counter 0))
                    (progn
                        (print current) 
                        (define mode motormode)
                        (define counter 50)
                    )
                )
            )
            
            (progn
                (progn
                    ; REEL-IN WITH LOW TENSION
                    
                    (if (< line-length 0)
                        (set-current 3);(set-brake 40)
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
