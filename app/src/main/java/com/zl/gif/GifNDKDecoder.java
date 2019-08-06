package com.zl.gif;

import android.graphics.Bitmap;

public class GifNDKDecoder {

    static {
        System.loadLibrary("native-lib");
    }

    public GifNDKDecoder(long gifPointer) {
        this.gifPointer = gifPointer;
    }

    public static GifNDKDecoder load(String path) {
        long gifHander = loadGifNative(path);
        GifNDKDecoder gifNDKDecoder = new GifNDKDecoder(gifHander);
        return gifNDKDecoder;
    }

    //加载gif文件
    private static native long loadGifNative(String path);

    public static native int getWidth(long gifPointer);

    public static native int getHeight(long gifPointer);

    public static native int updateFrame(Bitmap bitmap, long gifPointer);

    public long getGifPointer() {
        return gifPointer;
    }

    private long gifPointer;
}
