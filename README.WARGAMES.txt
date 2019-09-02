
Notes on AWS LEX mpeg voice manipulation to sound like Joshua a.k.a the WOPR from War Games...

Select a lex voice (One of the male voices works well) then apply the following command before playback:

```
sudo play response.mpeg stretch 1.2 8 lin 0.6 0.1 overdrive 12 12 synth sine fmo
```

This yeails a reasonably good result however on initial investigation I don't think this method can be used with AVS on the Pi.
