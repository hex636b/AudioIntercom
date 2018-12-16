package com.example.konga;

public class Talkback {

    public long nativeInstance;
    public native int start();
    public native int stop();

    public Talkback() {
        this.nativeInstance = 0;
    }
}
