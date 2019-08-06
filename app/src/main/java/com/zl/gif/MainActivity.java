package com.zl.gif;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    //读写权限
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};
    //请求状态码
    private static int REQUEST_PERMISSION_CODE = 1;
    private GifNDKDecoder gifNDKDecoder;
    Bitmap bitmap;
    private ImageView imageView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = findViewById(R.id.image);
        // Example of a call to a native method

        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP) {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, PERMISSIONS_STORAGE, REQUEST_PERMISSION_CODE);
            }
        }
    }

    public void ndkLoadGif(View view) {
//        copyAssetAndWrite("demo.gif");
//        File file = new File(getCacheDir(), "demo.gif");
        File file = new File(Environment.getExternalStorageDirectory(), "demo.gif");
        gifNDKDecoder = GifNDKDecoder.load(file.getAbsolutePath());
        int width = gifNDKDecoder.getWidth(gifNDKDecoder.getGifPointer());
        int height = gifNDKDecoder.getHeight(gifNDKDecoder.getGifPointer());
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        int nextDelayTime = gifNDKDecoder.updateFrame(bitmap, gifNDKDecoder.getGifPointer());
        myHandler.sendEmptyMessageDelayed(1, nextDelayTime);
    }

    Handler myHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            int nextDelayTime = gifNDKDecoder.updateFrame(bitmap, gifNDKDecoder.getGifPointer());
            myHandler.sendEmptyMessageDelayed(1, nextDelayTime);
            imageView.setImageBitmap(bitmap);
        }
    };

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSION_CODE) {
            for (int i = 0; i < permissions.length; i++) {
                Log.i("MainActivity", "申请的权限为：" + permissions[i] + ",申请结果：" + grantResults[i]);
            }
        }
    }
}
