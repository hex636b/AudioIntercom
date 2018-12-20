package com.example.konga;

import android.app.Application;

public class ApplicationIntercom extends Application {
    @Override
    public void onCreate() {
        System.loadLibrary("intercom");
        System.loadLibrary("ffmpeg");
        super.onCreate();
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
