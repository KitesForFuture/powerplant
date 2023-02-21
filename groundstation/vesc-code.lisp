(define launch-line-length 180)
(define min-eight-line-length 200)
(define max-eight-line-length 230)

(define launch-command 0.0)
(define land-command 1.0)

(define launch 0)
(define eight 1)
(define landing 2)
(define final-landing 3)
(define flightmode launch)

(define generatormode 0)
(define motormode 1)
(define mode generatormode)

(define counter 10)

(define max-reel-out-tension 10)

(uart-start 115200)

(define offset (get-dist))

(define arr (array-create 16))
(define array (array-create 16))

(define uart-send-counter 0)

(loopwhile t
    (progn
        
        (define counter (- counter 1))
        (if (< counter 0) (define counter 0) 0)
        
        
        (define uart-send-counter (- uart-send-counter 1))
        (if (< uart-send-counter 0) (define uart-send-counter 0) 0)
        (if (= uart-send-counter 0)
            (progn
                (define uart-send-counter 20)
                (define line-length (- 0 (- (get-dist) offset) ) )
                ; SEND line-length AND line tension(current)
                (bufset-f32 arr 0 1234567.0)
                (bufset-f32 arr 4 line-length)
                (bufset-f32 arr 8 flightmode)
                (bufset-f32 arr 12 -1234567.0)
                (uart-write arr)
            )
        )
        
        ; RECEIVE tension-request from kite
        (bufclear array)
        (define num-bytes-read (uart-read array 16))
        
        ;(print num-bytes-read)
        (if (> num-bytes-read 15)
            (if (and (= (bufget-f32 array 0) 1234567.0) (= (bufget-f32 array 12) -1234567.0))
                ; received something
                (progn
                    ;(print (bufget-f32 array 4))
                    (if (= (bufget-f32 array 4) land-command)
                        (progn
                		(define flightmode final-landing)
                		(define mode motormode)
                        )
                    )
                    ;(if (= (bufget-f32 array 4) launch-command)
                    ;    (progn
                	;	(define flightmode launch)
                	;	(define mode generatormode)
                    ;    )
                    ;)
                )
            )
        )
        
        (if (= mode generatormode)
            (progn
                (if (= flightmode launch)
                    (set-brake 0.1) ; LAUNCHING
                    (progn
                        (print "generating") 
                        (set-brake 9) ; GENERATING max-reel-out-tension
                    )
                )
                
                ; SWITCHING TO REEL-IN
                (if (and (= flightmode eight) (> (get-current) -1) (= counter 0))
                    (progn
                        (define mode motormode)
                        (define counter 100)
                    )
                )
                
                (if (and (= flightmode launch) (> line-length launch-line-length))
                    (define flightmode eight)
                )
                
                (if (and (= flightmode eight) (> line-length max-eight-line-length))
                	(progn
                		(define flightmode landing)
                		(define mode motormode)
                	)
                )
            )
            
            (progn
                (progn
                    ; REEL-IN WITH LOW TENSION
                    (if (= flightmode eight)
                        (progn
                            (print "REEL IN while Eight")
                            (set-current 9)
                        )
                        ; flightmode = landing or final-landing
                        (if (> line-length 30)
                            (set-current 3)
                            (if (> line-length 0)
                                (progn
                                    (set-current (+ 1.5 (* 0.05 line-length)))
                                )
                            )
                        )
                    )
                    
                    (if (and (= flightmode landing) (< line-length min-eight-line-length))
                    	(define flightmode eight)
                    )
                    
                    ; SWITCHING TO REEL-OUT
                    (if (and (= flightmode eight) (< (get-duty) 0.1) (= counter 0))
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
