package com.example.konga;

public class AudioIntercom {

    public long nativeInstance;
    public native int create();
    public native int start();
    public native int destroy();

    public AudioIntercom() {
        this.nativeInstance = 0;
    }
}
