package com.example.konga;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";
    private Button mButtonStart;
    private Button mButtonStop;
    private Talkback mTalkback;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mButtonStart = (Button)findViewById(R.id.buttonStart);
        mButtonStop = (Button)findViewById(R.id.buttonStop);

        mButtonStart.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                if(mTalkback != null ){
                    mTalkback.stop();
                }
                mTalkback = new Talkback();
                mTalkback.start();
            }
        });


        mButtonStop.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                if(mTalkback != null){
                    mTalkback.stop();
                }
            }
        });

    }

}