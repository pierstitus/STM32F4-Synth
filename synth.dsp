import("music.lib");
//ml = library("music.lib");  // SR, ...
//fl = library("filter.lib");
oscillator = library("oscillator.lib");
maxmsp = library("maxmsp.lib");
fast = library("fast.lib");

halftime2fac(x) = 0.5^(1./(SR*x));

smooth(x) = maxmsp.line(x,2);

envdecay(c) = (max:_ * c) ~ _;

// vsliders to be used by embedded platform
acc_abs = vslider("v:accelerometer/acc_abs[style:knob]",1,0,4,0.01);
acc_x = vslider("v:accelerometer/acc_x[style:knob]",0,-1,1,0.01);
acc_y = vslider("v:accelerometer/acc_y[style:knob]",0,-1,1,0.01);
acc_z = vslider("v:accelerometer/acc_z[style:knob]",-1,-1,1,0.01);

// hsliders will be changed to static values before compiliation for embedded
note = hslider("v:[0]config/note[style:knob]",69,0,127,.01);
decay = hslider("v:[0]config/decay[style:knob]",0.7,0,1,0.01):halftime2fac;
bendRange = hslider("v:[0]config/bendRange[style:knob]",12,0,36,0.01);
minFreq = hslider("v:[0]config/minFreq[style:knob]",400,0,1000,1);
bodyFreq = hslider("v:[0]config/bodyFreq[style:knob]",500,0,1000,1);

filtQ = hslider("v:[1]config2/filtQ[style:knob]",2,0,10,0.01);
filt2Freq = hslider("v:[1]config2/filt2Freq[style:knob]",3000,0,10000,1);
filt2Q = hslider("v:[1]config2/filt2Q[style:knob]",2,0.01,10,0.01);
filt2level = hslider("v:[1]config2/filt2Level[style:knob]",2,0,50,0.01);


// sawtooth + square oscilator
oscss(freq, even_harm) = even_harm*saw-(1-even_harm)*square
with {
    square = oscillator.lf_squarewave(freq)*0.5;
    saw = oscillator.saw2(freq);
};

voice = vosc + perc <: filt, filt2 :> _ * level
with {
    even_harm = (acc_x^3+1)/2;
    pitchbend = acc_y^3;
    freq = fast.note2freq(note+pitchbend*bendRange);
    vosc = oscss(freq, even_harm);

    perc = max(-acc_z,0.02) * noise;

    l = abs(acc_abs-1):envdecay(decay);
    level = (l*0.25)^2;
    K = fast.K_f0(max(freq,minFreq)) + max(-1,0);
    filt = fast.LPF(K, filtQ);

    filt2lev = max((acc_x^3+1)/2, 0);
    filt2 = fast.BPF(fast.K_f0(filt2Freq*filt2lev+minFreq), filt2Q) * filt2level * filt2lev;
};

//Body Filter: a biquad filter with a normalized pick gain
bodyFilter = fast.BPF(fast.K_f0(bodyFreq),0.55);

mystereoizer(periodDuration) = _ <: _,widthdelay : stereopanner
with {
    W = 0.5; //hslider("v:Spat/spatial width", 0.5, 0, 1, 0.01);
    A = 0.5; //hslider("v:Spat/pan angle", 0.6, 0, 1, 0.01);
    widthdelay = delay(4096,W*periodDuration/2);
    stereopanner = _,_ : *(1.0-A), *(A);
};

stereo = mystereoizer(SR/440);

process = hgroup("synth",
    voice : bodyFilter : stereo);
