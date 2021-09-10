package kr.ac.kpu.game.s2016182019.termproject.game.object;

import android.graphics.Canvas;
import android.graphics.RectF;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.BoxCollidable;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.BaseGame;

public class Block implements GameObject, BoxCollidable {

    private static final float SPEED = 500;
    public int x;
    public int y;
    public blockType type;
    public boolean boom = false;
    private GameBitmap bitmap;
    private boolean selected;
    public int bitmaps[] = {
            R.mipmap.red, R.mipmap.green, R.mipmap.blue, R.mipmap.black, R.mipmap.white, R.mipmap.sword, R.mipmap.shield
    };
    private GameBitmap highlight;
    private int tx;
    private int ty;
    public boolean isMoving = false;

    public enum blockType{
        Red, Green, Blue, Black, White, Sword, Shield
    }


    public Block(int x, int y, int index) {
        initialize(x, y, index);
    }

    public void initialize(int x, int y, int index) {
        this.x = x;
        this.y = y;
        blockType[] temp = blockType.values();
        this.type = temp[index];
        this.bitmap = new GameBitmap(bitmaps[index]);
        this.highlight = new GameBitmap(R.mipmap.highlight);
        this.tx = x;
        this.ty = y;
    }

    @Override
    public void update() {
        BaseGame game = BaseGame.get();
        float dx = (Math.abs(tx - x) + SPEED) * game.frameTime;
        float dy = (Math.abs(ty - y) + SPEED) * game.frameTime;
        // 위치 스왑
        if (tx == x && ty == y) {
            isMoving = false;
        }
        if (tx != x)
        {
            if (tx <= x) {
                dx = -dx;
            }
            x += dx;
            if ((dx > 0 && x > tx) || (dx < 0 && x < tx)) {
                x = tx;
            }
        }
        if (ty != y)
        {
            if (ty <= y) {
                dy = -dy;
            }
            y += dy;
            if ((dy > 0 && y > ty) || (dy < 0 && y < ty)) {
                y = ty;
            }
        }


    }

    @Override
    public void draw(Canvas canvas) {
        bitmap.draw(canvas, x, y);
        if (selected)
        {
            highlight.draw(canvas,x,y);
        }
    }

    @Override
    public void getBoundingRect(RectF rect) {
        bitmap.getBoundingRect(x, y, rect);
    }

    public void select(boolean b) {
        selected = b;
    }

    public void change(blockType type) {
        this.bitmap = new GameBitmap(bitmaps[type.ordinal()]);
        this.type = type;
    }

    public void moveto(int tx, int ty) {
        this.isMoving = true;
        this.tx = tx;
        this.ty = ty;
    }
}
