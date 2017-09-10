
# Common Music 3.9

This is a copy of CM/Grace 3.9 with a little hack that allowed me to
use it as a emacs scheme-mode inferior Scheme. It is pretty much the
standard CM-3.9, but it also listens to S-expressions from standard input.

## How to use

- Open .scm files you want to use with CM.
- Run M-x run-scheme (set also scheme-program-name, see below).
- CM shell opens up. If you want to have it in a new frame, set inferior-scheme-mode-hook (see below).
- Now CM accepts input from shell, or from emacs via scheme-mode evaluation.
- It is also possible to mix up input by opening files via CM GUI console.

All output from CM, including errors, are sent to CM console, which
is a graphical application. There's no easy way to grab that input back to
console since the real console support was removed from CM-3.9.

Ugly, yes, I know, but I can now write my notes in emacs and send them over
to CM evaluator. Emacs scheme-mode does all that stuff automagically, I just
basically defined only few things in my emacs.el:


```lisp
(setq scheme-program-name "~/bin/Grace")  ; <-- This is where I copied bin/Grace
(define-key scheme-mode-map (kbd "<C-return>") 'scheme-send-last-sexp)
(define-key scheme-mode-map (kbd "<C-M-return>") 'scheme-send-region)
(add-hook 'inferior-scheme-mode-hook (lambda ()
				       (split-window-below)))
```

In addition to this, I also did some small changes to the source in order to
compile it succesfully under Fedora26:

* Changed file chooser dialogs to JUCE from native (for some reason Gnome3 didn't like them)
* Patched brutally, without knowing what I'm doing 'lround' -> 'ljround' in juce/modules/juce_audio_formats/codecs/flac/libFLAC/lpc_flac.c (Hey, works for me!)
* Some tweaks to colors in the JUCE code editor (which I don't use anymore)

There's a little demonstration in "demo" directory. It's HTML5 video/audio, works propably only with recent firefox/chrome.

Common Music is amazing hidden gem, it can control real-time SuperCollider and MIDI, and
run old skool Common Lisp Music instruments and CSound as well, all under consistent and sold algorithmic
programming environment.

There seems to be a little bit of fluctuation around CM at the moment, se let's see where it goes.

Have fun!

