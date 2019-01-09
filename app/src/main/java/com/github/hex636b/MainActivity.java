package com.github.hex636b;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";
    private static final int REQUEST_RECORD_AUDIO = 0x1;

    private Button mButtonStart;
    private Button mButtonStop;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mButtonStart = (Button)findViewById(R.id.buttonStart);
        mButtonStop = (Button)findViewById(R.id.buttonStop);

        mButtonStart.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {

                if(checkSelfPermission(android.Manifest.permission.RECORD_AUDIO)
                        != PackageManager.PERMISSION_GRANTED){
                    requestPermissions(
                            new String[]{android.Manifest.permission.RECORD_AUDIO},
                            REQUEST_RECORD_AUDIO);
                    return;
                }

                AudioRecord.start();
            }
        });


        mButtonStop.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {

                AudioRecord.stop();
            }
        });

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if(requestCode == REQUEST_RECORD_AUDIO && grantResults[0]==PackageManager.PERMISSION_GRANTED){
            mButtonStart.callOnClick();
        }else {
            Toast.makeText(this,"user denied RECORD_AUDIO", Toast.LENGTH_SHORT).show();
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }


}