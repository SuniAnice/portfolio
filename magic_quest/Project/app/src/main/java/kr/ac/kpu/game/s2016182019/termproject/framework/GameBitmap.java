package kr.ac.kpu.game.s2016182019.termproject.framework;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.RectF;
import android.util.DisplayMetrics;

import java.util.HashMap;

import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;


public class GameBitmap {
    private static HashMap<Integer, Bitmap> bitmaps = new HashMap<Integer, Bitmap>();


    public static Bitmap load(int resId) {
        Bitmap bitmap = bitmaps.get(resId);
        if (bitmap == null){
            Resources res = GameView.view.getResources();
            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inScaled = false;
            bitmap = BitmapFactory.decodeResource(res, resId, opts);
            bitmaps.put(resId, bitmap);
        }
        return bitmap;
    }

    protected final Bitmap bitmap;
    protected RectF dstRect = new RectF();

    public GameBitmap(int resId) {



        bitmap = load(resId);
    }

    public void draw(Canvas canvas, float x, float y) {
        int hw = getWidth() / 2;
        int hh = getHeight() / 2;

        float dl = x * 1.33f - hw * GameView.MULTIPLIER;
        float dt = y * 1.33f - hh * GameView.MULTIPLIER;
        float dr = x * 1.33f + hw * GameView.MULTIPLIER;
        float db = y * 1.33f + hh * GameView.MULTIPLIER;
        dstRect.set(dl, dt, dr, db);
        canvas.drawBitmap(bitmap, null, dstRect, null);
    }

    public void draw(Canvas canvas, float x, float y, float mul) {
        int hw = getWidth() / 2;
        int hh = getHeight() / 2;

        float dl = x*1.33f - hw * GameView.MULTIPLIER * mul;
        float dt = y*1.33f - hh * GameView.MULTIPLIER * mul;
        float dr = x*1.33f + hw * GameView.MULTIPLIER * mul;
        float db = y*1.33f + hh * GameView.MULTIPLIER * mul;
        dstRect.set(dl, dt, dr, db);
        canvas.drawBitmap(bitmap, null, dstRect, null);
    }

    public int getHeight() {
        return bitmap.getHeight();
    }

    public int getWidth() {
        return bitmap.getWidth();
    }

    public void getBoundingRect(float x, float y, RectF rect) {
        int hw = getWidth() / 2;
        int hh = getHeight() / 2;

        float dl = x*1.33f - hw * GameView.MULTIPLIER;
        float dt = y*1.33f - hh * GameView.MULTIPLIER;
        float dr = x*1.33f + hw * GameView.MULTIPLIER;
        float db = y*1.33f + hh * GameView.MULTIPLIER;
       rect.set(dl, dt, dr, db);
    }

    public void getBoundingRect(float x, float y, RectF rect, float mul) {
        int hw = getWidth() / 2;
        int hh = getHeight() / 2;

        float dl = x*1.33f - hw * GameView.MULTIPLIER * mul;
        float dt = y*1.33f - hh * GameView.MULTIPLIER * mul;
        float dr = x*1.33f + hw * GameView.MULTIPLIER * mul;
        float db = y*1.33f + hh * GameView.MULTIPLIER * mul;
        rect.set(dl, dt, dr, db);
    }
}
