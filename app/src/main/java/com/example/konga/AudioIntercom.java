package com.example.konga;

public class AudioIntercom {

    public long nativeInstance;
    public native int start();
    public native int stop();

    public AudioIntercom() {
        this.nativeInstance = 0;
    }
}
