package com.example.konga;

import android.app.Application;

public class ApplicationIntercom extends Application {
    @Override
    public void onCreate() {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("intercom");
        super.onCreate();
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
