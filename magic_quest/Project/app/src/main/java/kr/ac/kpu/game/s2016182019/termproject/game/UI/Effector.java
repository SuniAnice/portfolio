package kr.ac.kpu.game.s2016182019.termproject.game.UI;

import android.graphics.Canvas;

import kr.ac.kpu.game.s2016182019.termproject.framework.AnimationGameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.BaseGame;
import kr.ac.kpu.game.s2016182019.termproject.game.Scene.MainScene;

public class Effector implements GameObject {

    private final AnimationGameBitmap bitmap;
    private final int x;
    private final int y;

    public float lifeTime = 0.9f;

    public Effector(AnimationGameBitmap bitmap, int x, int y, float lifeTime) {
        this.bitmap = bitmap;
        this.x = x;
        this.y = y;
        this.lifeTime = lifeTime;
    }


    @Override
    public void update() {
        BaseGame game = BaseGame.get();
        lifeTime -= game.frameTime;
        if (lifeTime < 0)
           MainScene.scene.remove(this);
    }

    @Override
    public void draw(Canvas canvas) {
        bitmap.draw(canvas, x, y);
    }
}
