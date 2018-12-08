package com.example.konga;

import android.app.Application;

public class ApplicationTalkback extends Application {
    @Override
    public void onCreate() {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("talkback");
        super.onCreate();
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
