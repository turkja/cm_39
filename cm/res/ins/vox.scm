;;; -------- mlbvoi
;;;
;;; translation from MUS10 of Marc LeBrun's waveshaping voice instrument (using FM here)
;;; this version translated (and simplified slightly) from CLM's mlbvoi.ins

(definstrument (vox beg dur freq amp ampfun freqfun freqscl phonemes formant-amps formant-indices (vibscl .1) (deg 0) (pcrev 0))  
  (let ((formants
	 '((I 390 1990 2550)  (E 530 1840 2480)  (AE 660 1720 2410)
	   (UH 520 1190 2390) (A 730 1090 2440)  (OW 570 840 2410)
	   (U 440 1020 2240)  (OO 300 870 2240)  (ER 490 1350 1690)
	   (W 300 610 2200)   (LL 380 880 2575)  (R 420 1300 1600)
	   (Y 300 2200 3065)  (EE 260 3500 3800) (LH 280 1450 1600)
	   (L 300 1300 3000)  (I2 350 2300 3340) (B 200 800 1750)
	   (D 300 1700 2600)  (G 250 1350 2000)  (M 280 900 2200)
	   (N 280 1700 2600)  (NG 280 2300 2750) (P 300 800 1750)
	   (T 200 1700 2600)  (K 350 1350 2000)  (F 175 900 4400)
	   (TH 200 1400 2200) (S 200 1300 2500)  (SH 200 1800 2000)
	   (V 175 1100 2400)  (THE 200 1600 2200)(Z 200 1300 2500)
	   (ZH 175 1800 2000) (ZZ 900 2400 3800) (VV 565 1045 2400))))
    ;;formant center frequencies for a male speaker
    
    (define (find-phoneme phoneme forms)
      (if (eq? phoneme (car (car forms)))
	  (cdr (car forms))
	  (find-phoneme phoneme (cdr forms))))
    
    (define (vox-fun phons which)
      (let ((f1 ())
	    (len (length phons)))
	(do ((i 0 (+ i 2)))
	    ((>= i len))
	  (set! f1 (cons (phons i) f1))
	  (set! f1 (cons ((find-phoneme (phons (+ i 1)) formants) which) f1)))
	(reverse f1)))
    
    (let ((start (seconds->samples beg))
	  (end (seconds->samples (+ beg dur)))
	  (car-os (make-oscil 0))
	  (fs (length formant-amps)))
      (let ((evens (make-vector fs))
	    (odds (make-vector fs))
	    (amps (make-float-vector fs 0.0))
	    (indices (make-float-vector fs 0.0))
	    (frmfs (make-vector fs))
	    (ampf (make-env ampfun :scaler amp :duration dur))
	    (freqf (make-env freqfun :duration dur :scaler (* freqscl freq) :offset freq))
	    (frq 0.0)
	    (carrier 0.0)
	    (frm-int 0)
	    (rfrq 0.0)
	    (frm0 0.0)
	    (even-amp 0.0)
	    (odd-amp 0.0)
	    (even-freq 0.0)
	    (odd-freq 0.0)
	    (sum 0.0)
	    (loc (make-locsig deg 1.0 pcrev))
	    (per-vib (make-triangle-wave :frequency 6 :amplitude (* freq vibscl)))
	    (ran-vib (make-rand-interp :frequency 20 :amplitude (* freq .5 vibscl))))
	(do ((i 0 (+ i 1)))
	    ((= i fs))
	  (let ((amp (formant-amps i))
		(index (formant-indices i)))
	    (set! (evens i) (make-oscil 0))
	    (set! (odds i) (make-oscil 0))
	    (set! (amps i) amp)
	    (set! (indices i) index)
	    (set! (frmfs i) (make-env (vox-fun phonemes i) :duration dur))))
	(do ((i start (+ i 1))) 
	    ((= i end))
	  (set! frq (+ (env freqf) (triangle-wave per-vib) (rand-interp ran-vib)))
	  (set! rfrq (hz->radians frq))
	  (set! carrier (oscil car-os rfrq))
	  (set! sum 0.0)
	  (do ((k 0 (+ k 1))) 
	      ((= k fs))
	    (set! frm0 (/ (env (frmfs k)) frq))
	    (set! frm-int (floor frm0))
	    (if (even? frm-int)
		(begin
		  (set! even-freq (+ (* frm-int rfrq) (* (float-vector-ref indices k) carrier)))
		  (set! odd-freq (+ even-freq rfrq)) ; in the current optimizer, embedding set! here is slower
		  (set! odd-amp (- frm0 frm-int))
		  (set! even-amp (- 1.0 odd-amp)))
		(begin
		  (set! odd-freq (+ (* frm-int rfrq) (* (float-vector-ref indices k) carrier)))
		  (set! even-freq (+ odd-freq rfrq))
		  (set! even-amp (- frm0 frm-int))
		  (set! odd-amp (- 1.0 even-amp))))
	    (set! sum (+ sum 
			 (* (float-vector-ref amps k) 
			    (+ (* even-amp (oscil (vector-ref evens k) even-freq))
			       (* odd-amp  (oscil (vector-ref odds k)  odd-freq)))))))
	  (locsig loc i (* sum (env ampf))))))))

;;; (vox 0 2 170 .4 '(0 0 25 1 75 1 100 0) '(0 0 5 .5 10 0 100 1) .1 '(0 E 25 AE 35 ER 65 ER 75 I 100 UH) '(.8 .15 .05) '(.005 .0125 .025) .05 .1)
;;; (vox 0 2 300 .4 '(0 0 25 1 75 1 100 0) '(0 0 5 .5 10 0 100 1) .1 '(0 I 5 OW 10 I 50 AE 100 OO) '(.8 .15 .05) '(.05 .0125 .025) .02 .1)
;;; (vox 0 5 600 .4 '(0 0 25 1 75 1 100 0) '(0 0 5 .5 10 0 100 1) .1 '(0 I 5 OW 10 I 50 AE 100 OO) '(.8 .16 .04) '(.01 .01 .1) .01 .1)
