(define launch-line-length 75)
(define min-eight-line-length 80)
(define max-eight-line-length 120)

(define safety-distance-for-landing-debug 0)

;(define launch-line-length 0.2)
;(define min-eight-line-length 0.4)
;(define max-eight-line-length 0.6)

(define line-length 0)
(define old-line-length-unfiltered 0)

(define launch-command 0.0)
(define land-command 1.0)
(define final-land-command 2.0)
(define emergency-land-command 112)

(define launch 0)
(define eight 1)
(define landing 2)
(define final-landing 3)
(define emergency-landing 112)
(define flightmode launch)

(define undefined 100)
(define flightmode-request launch)

(define counter 10)

(define max-reel-out-tension 10)

(uart-start 115200)

(define offset (get-dist))

(define arr (array-create 20))
(define array (array-create 16))

(define uart-send-counter 0)

(gpio-configure 'pin-adc1 'pin-mode-out)

(gpio-write 'pin-adc1 0)

(loopwhile t
    (progn
        
        (define counter (- counter 1))
        (if (< counter 0) (define counter 0) 0)
        
        
        (define uart-send-counter (- uart-send-counter 1))
        (if (< uart-send-counter 0) (define uart-send-counter 0) 0)
        (if (= uart-send-counter 0)
            (progn
                (define uart-send-counter 10)
                (define line-length-unfiltered (- 0 (- (get-dist) offset) ) )
                
                
                ; this is all for finding the line-length bug
                (if (or (< (+ old-line-length-unfiltered 3) line-length-unfiltered) (> (- old-line-length-unfiltered 3) line-length-unfiltered)) ;if line-length-unfiltered jumps by more than 10 metres
                	(gpio-write 'pin-adc1 1)
                	(define line-length line-length-unfiltered)
                )
                (define old-line-length-unfiltered line-length-unfiltered)
                
                
                ; SEND line-length AND flight-mode request
                (bufset-f32 arr 0 1234567.0)
                (bufset-f32 arr 4 line-length)
                (bufset-f32 arr 8 flightmode-request)
                (bufset-f32 arr 12 (- 0 (get-speed)))
                (bufset-f32 arr 16 -1234567.0)
                (print "sending UART") 
                (uart-write arr)
                ;(if (= led-state 0)
                ;	(define led-state 1)
                ;	(define led-state 0)
                ;)
                ;(gpio-write 'pin-adc1 led-state)
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
		            		;(define mode motormode)
                        )
                    )
                    (if (= (bufget-f32 array 4) final-land-command)
                        (progn
		            		(define flightmode final-landing)
		            		(define flightmode-request final-landing)
		            		;(define mode motormode)
                        )
                    )
                    (if (= (bufget-f32 array 4) emergency-land-command)
                        (progn
		            		(define flightmode emergency-landing)
		            		(define flightmode-request emergency-landing)
		            		;(define mode motormode)
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
        
        (if (= flightmode launch)
            (set-current -0.3) ; LAUNCHING
            (if (= flightmode eight)
                (progn
                    (define s (get-speed))
                    (print s)
                    (if (> s 0)
                        (set-current 23)
                        (set-current 20)
                    )
                )
                (if (= flightmode emergency-landing)
                	(set-current 0)
                	; flightmode = landing or final-landing
	                (if (> line-length (+ 15.75 safety-distance-for-landing-debug))
	                    (set-current 8)
	                    (if (> line-length safety-distance-for-landing-debug)
	                        (set-current (+ 1.7 (* 0.4 (- line-length safety-distance-for-landing-debug))))
	                        (set-current 1.7)
	                    )
	                )
	            )
            )
        )
        
        (if (and (= flightmode landing) (< line-length min-eight-line-length))
        	(progn
        		(define flightmode eight)
        		(define flightmode-request eight)
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
        
        
        
        (sleep 0.005)
    )
)
