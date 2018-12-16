package com.example.konga;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";
    private Button mButtonStart;
    private Button mButtonStop;
    private AudioIntercom audioIntercom;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mButtonStart = (Button)findViewById(R.id.buttonStart);
        mButtonStop = (Button)findViewById(R.id.buttonStop);

        mButtonStart.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                if(audioIntercom != null ){
                    audioIntercom.destroy();
                }
                audioIntercom = new AudioIntercom();
                audioIntercom.create();
                audioIntercom.start();
            }
        });


        mButtonStop.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                if(audioIntercom != null){
                    audioIntercom.destroy();
                }
            }
        });

    }

}