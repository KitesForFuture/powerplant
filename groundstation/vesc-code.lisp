(define launch-line-length 50)
(define min-eight-line-length 100)
(define max-eight-line-length 150)

;(define launch-line-length 1)
;(define min-eight-line-length 2)
;(define max-eight-line-length 3)

(define launch-command 0.0)
(define land-command 1.0)
(define final-land-command 2.0)

(define launch 0)
(define eight 1)
(define landing 2)
(define final-landing 3)
(define flightmode launch)

(define undefined 100)
(define flightmode-request launch)

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
                ; SEND line-length AND flight-mode request
                (bufset-f32 arr 0 1234567.0)
                (bufset-f32 arr 4 line-length)
                (bufset-f32 arr 8 flightmode-request)
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
                    (if (and (= (bufget-f32 array 4) land-command) (= flightmode-request landing))
                        (progn
		            		(define flightmode landing)
		            		(define flightmode-request undefined)
		            		(define mode motormode)
                        )
                    )
                    (if (= (bufget-f32 array 4) final-land-command)
                        (progn
		            		(define flightmode final-landing)
		            		(define flightmode-request final-landing)
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
                        ;(print "generating") 
                        (set-brake 12) ; GENERATING max-reel-out-tension
                        ;(set-brake 3) ; GENERATING max-reel-out-tension
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
                    (progn
                    	(define flightmode eight)
                    	(define flightmode-request eight)
                    )
                )
                
                (if (and (= flightmode eight) (> line-length max-eight-line-length))
                	(define flightmode-request landing)
                )
            )
            
            (progn
                (progn
                    ; REEL-IN WITH LOW TENSION
                    (if (= flightmode eight)
                        (progn
                            ;(print "REEL IN while Eight")
                            (set-current 15)
                            ;(set-current 6)
                        )
                        ; flightmode = landing or final-landing
                        (if (> line-length 43)
                            (set-current 6)
                            (if (> line-length 0)
                                (progn
                                    (set-current (+ 1.7 (* 0.1 line-length)))
                                )
                                (set-current 0)
                            )
                        )
                    )
                    
                    (if (and (= flightmode landing) (< line-length min-eight-line-length))
                    	(progn
                    		(define flightmode eight)
                    		(define flightmode-request eight)
                    	)
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
