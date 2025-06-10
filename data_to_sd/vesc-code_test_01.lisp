
(define offset (get-dist))

(loopwhile t
    (progn
        (define line-length (- 0 (- (get-dist) offset) ) )
        (define speed (get-speed))
        (if (< speed 0)
            (progn
                (define c (- 6 (* speed 10)))
                (set-current c)
            )
            (if (< line-length 0)
            	(set-brake 40)
            	(if (< line-length 5)
		        	(set-current 3)
		        	(set-current 6)
		        )
            )
        )
        (sleep 0.005)
    )
)

