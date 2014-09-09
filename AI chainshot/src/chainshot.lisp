(defvar *board* nil) 
(defvar *same-list* nil)
(defvar *queue* nil)
(defvar *move-score* 0)
(defvar *total-score* 0)


;;;create game board >> *board* in 2D array  //global var( *n* *board*)
(defun make-clean-board! (size)
  (setf *board* (make-array (list size size))))

(defun file-switch (size)
  (cond ((= size 5) "b5.lisp")
	((= size 7) "b7.lisp")
	((= size 10) "b10.lisp")
	(t (format t "~&Sorry, I have only 5,7 or 10 for you to choose."))))

(defun load-in-data (file-name size board)
  (with-open-file (in file-name)
    (dotimes (row size board)
      (dotimes (col size)
	(setf (aref board row col) (read in))))))

(defun create-game-board ()
  (format t "~&Please choose a board size.Only 3 options:5,7 or 10.")
  (let ((size (read)))
    (load-in-data (file-switch size) size (make-clean-board! size))))

;;;print on screen

(defun cell-color (board row col)
  (aref board row col))

(defun nil-filter (board row col)
    (if (equal nil (cell-color board row col)) 
	" "
	(cell-color board row col)))

(defun print-board (board)
  (dotimes (row (board-size board))
    (format t "~&~a  |" row) 
    (dotimes (col (board-size board))
      (format t " ~a " (nil-filter board row col))))
  (format t "~&    ") ;; for coordinate system
  (dotimes (col (board-size board))
    (format t "---"))
  (format t "~&    ")
  (dotimes (col (board-size board))
    (format t " ~a " col)))

;;;choose cell with error detection 1. out of bound 2.color or empty (user-r user-c)

(defun choose-a-cell (board)
  (format t "~&Please choose a cell. Type in row and column number with a space in between.")
  (let ((row (read))
	(col (read)))
    (cond ((or (> row (- (board-size board) 1))
	       (< row 0)
	       (> col (- (board-size board) 1))
	       (< col 0))
	   (format t "~&The cell is out of board. Choose another one.") 
	   (choose-a-cell board))
	  ((equal (cell-color board row col) nil) 
	   (format t "The cell has no color. Choose another one.")
	   (choose-a-cell board))
	  (t (push-in row col)))))

;;;search for the same color (*same-list* *queue*)

(defun in-bound? (board row col)
  (and (> row -1)(< row (board-size board))(> col -1)(< col (board-size board))))
(defun same-color? (base-color board row col)
  (equal base-color (cell-color board row col)))
(defun no-duplicate? (same-list row col)
  (if (member (list row col) same-list :test #'equal) nil t))

(defun e1  (board r c nr nc)
  (cond ((and (in-bound? board nr nc)
	      (same-color? (cell-color board  r c) board nr nc)
	      (no-duplicate? *same-list* nr nc))
	 (push-in nr nc))))

(defun e4 (board r c)
  (e1 board r c (1+ r) c)
  (e1 board r c (1- r) c)
  (e1 board r c r (1+ c))
  (e1 board r c r (1- c)))

(defun explore (board)
  (do ()
      ((null *queue*) *same-list*)
    (let ((qr (first (first *queue*)))
	  (qc (second (first *queue*))))
      (pop *queue*)
      (e4 board qr qc))))

      
;;;keep and show scores
(defun keep-score (number board)
  (cond ((< number 3) (format t "~&You need to choose a group more than two same color cells.")
	 (choose-a-cell board))
	(t (setf *move-score* (* (- number 2)(- number 2)))
	   (setf *total-score* (+ *total-score* *move-score*)))))

(defun show-score ()
  (format t "~&Good move! You get ~a points!" *move-score*)
  (format t "~&Total score: ~a points" *total-score*))

;;; move the board : downward and left shift

(defun nilize (board same-set)
  (do ()
      ((null same-set))
    (let ((x nil))
      (setf x (pop same-set))
      (write-cell-color board (car x) (cadr x) nil))))

 (defun downward (board) ;;input nilized board
  (do ((c 0 (1+ c)))
      ((= c (board-size board)))
    (downward-one-column board c)))
    
(defun downward-one-column (board c)
  (let ((qc nil))
    (do ((r (1- (board-size board)) (1- r))) ;;; pull out whole column into qc
	((< r 0))
      (push (cell-color board r c) qc))
    (setf qc (remove nil qc)) ;;;remove all nils then put the same number nils bk to qc
    (let ((x (- (board-size board) (length qc))))
      (dotimes (cnt x)
	(push nil qc)))
    (do ((r 0 (1+ r))) ;;;put qc bk to column
	((null qc))
      (let ((x (pop qc)))
	(write-cell-color board r c x)))))

(defun empty-column? (board c) ;;check for empty column
  (do ((r 0 (1+ r)))
      ((= r (board-size board)) t)
    (if (not (nil? (cell-color board r c)))
	(return nil))))
       
(defun left-shift (board) ;;input nilized board
  (do ((c 0 (1+ c)))
      ((= c (board-size board)))  ;;scan from column 0-end
    (if (empty-column? board c)  ;;if find a nil column
	(do ((r 0 (1+ r))) ;;scan from top row to bottom
	    ((= r (board-size board)))
	  (do ((cc c (1+ cc))) ;;start from where the empty column is
	      ((= cc (board-size board))) ;;to the end column of array
	    (if (= cc (1- (board-size board))) (write-cell-color board r cc nil)
		(write-cell-color board r cc (cell-color board r (1+ cc)))))))))
            ;; if cc pointer is already at the end of column, write in nil.
            ;; else write the color of right cell to where pointer is 

;;;end state test

(defun end-state? (board)
  (let ((pour nil))
    (do ((r 0 (1+ r)))
	((= r (board-size board)))
      (do ((c 0 (1+ c)))
	  ((= c (board-size board)))
	(unless (nil? (cell-color board r c))
	  (push (list r c) pour))))
    (do ((x (pop pour) (pop pour)))
	((null pour) t)
      (let ((*same-list* nil)
	    (*queue* nil))
	(push-in (car x) (cadr x))
	(explore board)
	(if (> (length *same-list*) 2)
	    (return nil))))))

(defun play-again ()
  (if (yes-or-no-p
       "~&Play again?")
      (chainshot-manual)
      (format t "~&Good day ^^!")))

;;; small tools
(defun make-nil-c (board c) 
  (do ((r 0 (1+ r)))
      ((= r (board-size board)))
    (write-cell-color board r c nil)))

(defun write-cell-color (board row col color)
  (setf (aref board row col) color))

(defun nil? (x)
  (equal nil x))

(defun push-in (row col)
  (push (list row col) *same-list*)
  (push (list row col) *queue*))

(defun board-size (board)
  (array-dimension board 0))

;;;;manual mode

(defun chainshot-manual ()
  (let ((*board* nil)
	(*move-score* 0)
	(*total-score* 0))
    (create-game-board)
    (do ()
	((end-state? *board*) (progn
			(format t "~&The game is over.")
			(play-again)))
      (print-board *board*)
      (let ((*same-list* nil)
	    (*queue* nil)) 
	(choose-a-cell *board*)
	(explore *board*)
	(keep-score (length *same-list*) *board*)
	(nilize *board* *same-list*))
      (downward *board*)
      (left-shift *board*)
      (print-board *board*)
      (show-score))))
   



(defun 3up-same (board)
  (let ((pour nil)
	(r nil))
    (do ((r 0 (1+ r)))  ;;pull out all non-nil cell cor into board
	((= r (board-size board)))
      (do ((c 0 (1+ c)))
	  ((= c (board-size board)))
	(unless (nil? (cell-color board r c))
	  (push (list r c) pour))))
    (do ()  
	((null pour) r)
      (let ((*queue* nil)
	    (*same-list* nil))
	(push-in (car (first pour)) (cadr (first pour)))
	(explore board)
	(if (> (length *same-list*) 2)
	    (push *same-list* r))
	(setf pour (set-exclusive-or *same-list* pour :test #'equal))))))

(Defun copy-array (array)
  (let ((copy nil)
	(size (board-size array)))
    (setf copy (make-array (list size size)))
    (do ((r 0 (1+ r)))
	((= r size) copy)
      (do ((c 0 (1+ c)))
	  ((= c size))
	(write-cell-color copy r c (cell-color array r c))))))
  
	
(defun left-beads (board)
  (let ((pour nil))
    (do ((r 0 (1+ r)))
	((= r (board-size board)))
      (do ((c 0 (1+ c)))
	  ((= c (board-size board)))
	(unless (nil? (cell-color board r c))
	  (push (list r c) pour))))
    (length pour)))

(defun nilize-m (board list)
  (let ((b nil))
    (setf b (copy-array board))
    (dolist (x list b)
      (setf (aref b (car x) (cadr x)) nil))))

(defun downward-m (ori-board)
  (labels ((d1c (board c)
	     (let ((qc nil))
	       (do ((r (1- (board-size board)) (1- r))) 
		   ((< r 0))
		 (push (cell-color board r c) qc))
	       (setf qc (remove nil qc)) 
	       (let ((x (- (board-size board) (length qc))))
		 (dotimes (cnt x)
		   (push nil qc)))
	       (do ((r 0 (1+ r))) 
		   ((null qc))
		 (let ((x (pop qc)))
		   (write-cell-color board r c x))))))
    (let ((b nil)) ;;like a firewall preventing side effects back to the original board
      (setf b (copy-array ori-board))
      (do ((c 0 (1+ c)))
	  ((= c (board-size b)) b)
	(d1c b c)))))

(defun left-shift-m (ori-board)
  (labels ((empty-column? (board c) 
	     (do ((r 0 (1+ r)))
		 ((= r (board-size board)) t)
	       (if (not (nil? (cell-color board r c)))
		   (return nil)))))
    (let ((b nil))
      (setf b (copy-array ori-board))
      (do ((c 0 (1+ c)))
	  ((= c (board-size b)) b) 
	(if (empty-column? b c)  
	    (do ((r 0 (1+ r))) 
		((= r (board-size b)))
	      (do ((cc c (1+ cc))) 
		  ((= cc (board-size b))) 
		(if (= cc (1- (board-size b))) (write-cell-color b r cc nil)
		    (write-cell-color b r cc (cell-color b r (1+ cc)))))))))))

	 

(defun exhaustive (board)  ;;DFS from the most left hand leaf
  (let ((r nil))           ;;result shall transcend recursive loop
    (setf r '(1000 0 nil)) ;; high celling for left beads. no fit for game-board with more than 1000 beads 
    (labels ((e (board score choice)
	       (cond ((null (3up-same board)) (cond ((< (left-beads board) (nth 0 r)) 
						     (setf r (list (left-beads board) score choice)))
						    ((and (= (left-beads board) (nth 0 r))
							  (> score (nth 1 r)))
						     (setf r (list (left-beads board) score choice)))))
		     (t 
		      (dolist (pointer (cutn (3up-same board) 2))
			(e (left-shift-m (downward-m (nilize-m board pointer)))
			   (+ score (* (- (length pointer) 2) (- (length pointer) 2)))
			   (cons (car pointer) choice)))))))
      (e board 0 nil))
    r))
	      
	      
(defun cutn (list n)
  (if (< (length list) n)
      list
      (cutn (cdr list) n)))

(defun make-board (size)
  (make-clean-board! size)
  (with-open-file (in "board.lisp")
    (dotimes (r size *board*)
      (dotimes (c size)
	(setf (aref *board* r c) (read in))))))

(defun f2 (list)
  (let ((x nil)
	(y nil))
    (dolist (p list)
      (if (> (length p) (length x))
	  (setf x p)))
    (dolist (p (remove x list :test #'equal))
      (if (> (length p) (length y))
	  (setf y p)))
    (list x y)))




(defun chainshot-auto (size)
  (make-board size)
  (exhaustive *board*))

	     


