package kr.ac.kpu.game.s2016182019.termproject.game.object;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.RectF;

import kr.ac.kpu.game.s2016182019.termproject.framework.BoxCollidable;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.MainGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;


public class ImageObject implements GameObject, BoxCollidable {
    private float gy = -1;
    protected Bitmap bitmap;

    protected Rect srcRect = new Rect();
    protected RectF dstRect = new RectF();
    protected ImageObject() {}
    public ImageObject(int resId, float x, float y, float multi) {
        init(resId, x, y, multi);
    }

    public ImageObject(int resId, float x, float y, float multi, float gy) {
        init(resId, x, y, multi);
        this.gy = gy;
    }
    protected void init(int resId, float x, float y, float multi) {
        bitmap = GameBitmap.load(resId);
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        srcRect.set(0, 0, w, h);
        float l = x - w / 2 * GameView.MULTIPLIER * multi;
        float t = y - h / 2 * GameView.MULTIPLIER * multi;
        float r = x + w / 2 * GameView.MULTIPLIER * multi;
        float b = y + h / 2 * GameView.MULTIPLIER * multi;
        dstRect.set(l, t, r, b);
    }

    public float getRight() {
        return dstRect.right;
    }

    @Override
    public void update() {
        MainGame game = MainGame.get();
        if (gy > 0) {
            dstRect.offset(0, game.frameTime * 500);
            gy -= game.frameTime * 500;
        }
    }

    @Override
    public void draw(Canvas canvas) {
        canvas.drawBitmap(bitmap, srcRect, dstRect, null);
    }

    public float getDstWidth() {
        return dstRect.width();
    }
    public float getDstHeight() {
        return dstRect.height();
    }

    @Override
    public void getBoundingRect(RectF rect) {
        rect.set(dstRect);
    }

    public RectF getBoundingRect() {
        return dstRect;
    }
}
