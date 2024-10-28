
(define offset (get-dist))

(loopwhile t
    (progn
        (define line-length (- 0 (- (get-dist) offset) ) )
        (define speed (get-speed))
        (if (< speed 0)
            (progn
                (define c (- 1.7 (* speed 10)))
                (set-current c)
            )
            (if (< line-length 0) (set-brake 40) (set-current 5))
        )
        (sleep 0.005)
    )
)
