
```
 /\\\
---\\\---------
----\\\--------
----/\\\------- Common Music 3.9.0 console hack
---/--\\\------
--/----\\\-----
 /      \\\/
```

This is a copy of [CM/Grace 3.9](http://commonmusic.sourceforge.net/) with a little hack that allowed me to
use it as a emacs scheme-mode inferior Scheme. It is pretty much the
standard CM-3.9, but it also listens to S-expressions from standard input.

## How to use

Assuming Grace is compiled successfully from directory "cm", here's how to use it from emacs:

- Open .scm files you want to use with CM.
- Run M-x run-scheme (set also scheme-program-name, see below).
- CM shell opens up. If you want to have it in a new frame, set inferior-scheme-mode-hook (see below).
- Now CM accepts input from shell, or from emacs via scheme-mode evaluation.
- It is also possible to mix up input by opening files via CM GUI console, but that's not really
  encouraged. The console shell is a blocking thread that expects things to happen in order.
- By default, the Emacs scheme magic pipes expressions from all buffers to CM shell, so you can
  have many documents open at the same time.

Grace is a graphical application that pops up when run-scheme is executed, which is kinda annoying.
But you can minimize it and it shouldn't bother you anymore when you work on emacs. I didn't want
to get rid of it completely, because I'm still not sure if everything can be done from the Scheme
(MIDI/audio configuration etc.).

Ugly, yes, I know, but I can now write my notes in emacs and send them over
to CM evaluator. Emacs scheme-mode does all that stuff automagically, I just
basically defined only few things in my emacs.el:


```lisp
(setq scheme-program-name "~/bin/Grace")  ; <-- This is where I copied the compiled bin/Grace

(define-key scheme-mode-map (kbd "<C-return>") 'scheme-send-last-sexp)
(define-key scheme-mode-map (kbd "<C-M-return>") 'scheme-send-region)

(add-hook 'inferior-scheme-mode-hook (lambda ()
	(split-window-below)))
	
; S7 Scheme uses "macroexpand" instead of default "expand".
(defcustom scheme-macro-expand-command "(macroexpand %s)"
	"Template for macro-expanding a Scheme form. For Scheme 48 and Scsh use \",expand %s\"."
	:type 'string
	:group 'cmuscheme)
```

In addition to this, I also did some small changes:

* Changed file chooser dialogs to JUCE from native (for some reason Gnome3 didn't like them).
* Patched brutally, without knowing what I'm doing 'lround' -> 'ljround' in juce/modules/juce_audio_formats/codecs/flac/libFLAC/lpc_flac.c (Hey, works for me!).
* Some tweaks to colors in the JUCE code editor (which I don't use anymore).
* Prevent main console GUI popping up on errors.
* (quit) now works from the Scheme.
* Tried to make sure all notifications and errors go to stderr, and only clean S-expressions to stdout. Make sure to
  check out "View->Console Beep On Error" from CM GUI console (or Grace.xml) to prevent BEL signal appearing to stdout.


Here's a little asciinema demo (click to play):

<a href="https://asciinema.org/a/AhEdw4oXTZs7dWstM1xYh8U5Z?autoplay=1" target="_blank"><img src="https://asciinema.org/a/AhEdw4oXTZs7dWstM1xYh8U5Z.png" style="width: 600px;"/></a>
