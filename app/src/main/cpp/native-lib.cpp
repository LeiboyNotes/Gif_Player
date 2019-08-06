#include <jni.h>
#include <string>
#include "gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>
#include <malloc.h>
#define  argb(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)
typedef struct GifBean {
    int current_frame;//当前帧
    int tatal_frames;//总帧数
    int *delays;
}GifBean;

void drawFrame(GifFileType *pType, GifBean *pBean, AndroidBitmapInfo info, void *pVoid);

extern "C"
JNIEXPORT jlong JNICALL
Java_com_zl_gif_GifNDKDecoder_loadGifNative(JNIEnv *env, jclass type, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    int err;
    GifFileType *gifFileType = DGifOpenFileName(path, &err);


    //gif初始化
    DGifSlurp(gifFileType);

    //给GifBean分配内存
    GifBean *gifBean = (GifBean *)(malloc(sizeof(GifBean)));
    //清空内存
    memset(gifBean, 0, sizeof(GifBean));
    //给延时总时间数组分配内存
    gifBean->delays = (int *)(malloc(sizeof(int) * gifFileType->ImageCount));
    memset(gifBean->delays, 0, sizeof(int) * gifFileType->ImageCount);
    ExtensionBlock *ext;
    //赋值给gifBean
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
        //取出每一帧图像
        SavedImage frame = gifFileType->SavedImages[i];
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                ext = &frame.ExtensionBlocks[j];
                break;//找到图形扩展块跳出循环
            }
        }
        if (ext) {
            //拿到图形控制拓展块（延时时间）
            //小端模式先取2再取1
            //Bytes[0]保留字段
            //Bytes[1]低八位
            //Bytes[2]高八位
            int frame_delay = (ext->Bytes[2] << 8 | ext->Bytes[1]) * 10;
            gifBean->delays[i] = frame_delay;
        }
    }
    gifBean->tatal_frames = gifFileType->ImageCount;
    //方便后面获取宽高等信息用
    gifFileType->UserData = gifBean;


    env->ReleaseStringUTFChars(path_, path);
    return (jlong) (gifFileType);
}

void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *phixels) {
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    //当前帧图像信息
    GifImageDesc imageDesc = savedImage.ImageDesc;
    //图像首地址
    int *px = (int *) phixels;
    ColorMapObject *colorMapObject = imageDesc.ColorMap;
    if (colorMapObject == NULL) {
        colorMapObject = gifFileType->SColorMap;
    }
    //y方向偏移量
    px = (int *)((char *) px + info.stride * imageDesc.Top);
    //记录像素位置
    int pointPixel;
    GifByteType gifByteType;
    GifColorType gifColorType;
    //每一行首地址
    int *line;
    for (int y = imageDesc.Top; y < imageDesc.Top + imageDesc.Height; ++y) {
        line = px;
        for (int x = imageDesc.Left; x < imageDesc.Left + imageDesc.Width; ++x) {
            pointPixel = (y - imageDesc.Top) * imageDesc.Width + (x - imageDesc.Left);
            //拿到像素数据
            gifByteType = savedImage.RasterBits[pointPixel];
            //给像素赋予颜色
            if(colorMapObject !=NULL){
                gifColorType = colorMapObject->Colors[gifByteType];
                line[x] =argb(255,gifColorType.Red,gifColorType.Green,gifColorType.Blue);
            }

        }
        px = (int *)((char *)px + info.stride);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_zl_gif_GifNDKDecoder_getWidth(JNIEnv *env, jclass type, jlong gifPointer) {

    GifFileType *gifFileType = (GifFileType *) (gifPointer);
    return gifFileType->SWidth;


}extern "C"
JNIEXPORT jint JNICALL
Java_com_zl_gif_GifNDKDecoder_getHeight(JNIEnv *env, jclass type, jlong gifPointer) {

    GifFileType *gifFileType = (GifFileType *) (gifPointer);
    return gifFileType->SHeight;


}extern "C"
JNIEXPORT jint JNICALL
Java_com_zl_gif_GifNDKDecoder_updateFrame(JNIEnv *env, jclass type, jobject bitmap,
                                          jlong gifPointer) {

    GifFileType *gifFileType = (GifFileType *) (gifPointer);
    GifBean *gifBean = (GifBean *) (gifFileType->UserData);

    AndroidBitmapInfo info;
    //通过bitmap获取AndroidBitmapInfo
    AndroidBitmap_getInfo(env, bitmap, &info);
    void *phiels;//像素数组
    //锁定bitmap
    AndroidBitmap_lockPixels(env, bitmap, &phiels);
    //绘制一帧图像
    drawFrame(gifFileType, gifBean, info, phiels);
    //绘制当前帧后 +1
    gifBean->current_frame += 1;
    if (gifBean->current_frame >= gifBean->tatal_frames) {
        gifBean->current_frame = 0;
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    return gifBean->delays[gifBean->current_frame];
}