;
;; Sending and Receiving OSC Data
;

; To run code put your cursor after each expression and press
; Command-Return, then check the console window for any output.

; These examples use SuperCollider to demonstrate how to send and
; receive OSC messages, but you can communicate with any OSC enabled
; app in the same manner.

; SUPERCOLLIDER CODE: copy the ( ) code the paste and eval in ScLang

"
(
  n = NetAddr(\localhost.asString, 7779);
  o = OSCFunc({|msg, time, from, port| msg.postln}, '/Grace', nil, 57120);
)
"

; As soon as you execute those expressions in SuperCollider it will be
; waiting on port 57120 for OSC input that matches the path "/Grace"

; Now use osc:open to open OSC ports in Grace. Set the input port to
; 7779 and the output port to 57120, which is SuperCollider's input
; port. If you want to send OSC to a remote machine you would pass an
; IP string to the output port, i.e. "good.tunes.com:57120" or
; "123.456.789.123:57120"

(osc:open 7779 57120)

; At this point OSC communication between Grace and SuperCollider
; should ready to go!

; Send a hello to SuperCollider

(osc:message "/Grace" "Hello, SuperCollider!")

; Send some basic OSC data (ints, floats, booleans, strings)

(osc:message "/Grace" -99 (random 1.0) #t #f (pick "=:)" "|:/" ">:("))

; You can send a message as a list too. Every list must start with the
; OSC path (string):

(osc:message '("/Grace" 123 456.789 #t #f "=:)") )

; To send blobs or midi you need to tag your data lists with :b or
; :m. This sends an OSC blob of 5 ints:

(osc:message "/Grace" :b '(0 2 4 6 8))

; Send OSC's 4-byte midi message: <status> <data1> <data2> <data3>

(osc:message "/Grace" :m (list 0 mm:on 60 127))

; Send a NoteOff:

(osc:message "/Grace" :m (list 0 mm:off 60 127))

;
; OSC Bundles
;

; A bundle lets you group multiple OSC messages under one timetag.
; The timetag 0 means IMMEDIATE, else it should be the number of
; seconds in the future the target should process the message. The
; target may or may not respect your future time stamps...

; Send bundle now

(osc:bundle 0 '("/Grace" 123) '("/Grace" 4 5 6))

; Send bundle 3 seconds in the future (maybe)

(osc:bundle 3 '("/Grace" "moe" "larry" "curley") '("/Grace" "nyuk-nyuk-nyuk" #t))

;
; Receiving OSC Messages
;

; First define a hook to receive OSC messages in Grace. The hook will
; be called whenever an OSC message arrives at Grace's input port

(define (myOscHook msg)
  (print "My OSC hook -> " msg))

; Set the hook...

(osc:receive myOscHook)

; SUPERCOLLIDER CODE: copy the code and evaluate in ScLang
; This will send a message to Grace, look in the Grace Console for
; printout from the hook:

"
n.sendMsg(\SuperCollider, 'Hello, Grace!')
"

; The hook we set in Grace is the default hook because it'll be called
; for all osc message arriving at Grace's osc port. You can also set
; hooks to be called for specific paths by providing the path string
; to osc:receive along with the hook. In this example we set a hook
; that is called if the path "/specific" arrives:

(define (mySpecificOscHook msg)
  (print "My specific Osc hook -> " msg))

(osc:receive "/specific" mySpecificOscHook)

; SUPERCOLLIDER CODE: copy the code and evaluate in ScLang

"
n.sendMsg(\specific, 1,2,3)
"

; To get a list of all active hooks:

(osc:receive?)

; To clear a specific hook you set it to false:

(osc:receive "/specific" #f)

; To clear the default hook just pass false

(osc:receive #f)

; To clear all hooks call receive with no inputs

(osc:receive)

; To close the Osc connection call osc:close

(osc:close)






