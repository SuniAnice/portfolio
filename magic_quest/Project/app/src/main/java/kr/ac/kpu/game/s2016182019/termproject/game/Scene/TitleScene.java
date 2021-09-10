package kr.ac.kpu.game.s2016182019.termproject.game.Scene;

import android.view.MotionEvent;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.MainGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;
import kr.ac.kpu.game.s2016182019.termproject.game.object.ImageObject;

public class TitleScene extends Scene {
    private static final String TAG = TitleScene.class.getSimpleName();

    enum Layer {
        bg, COUNT
    }
    public static TitleScene scene;
    public void add(Layer layer, GameObject obj) {
        add(layer.ordinal(), obj);
    }
    @Override
    public void start() {
        super.start();
        transparent = false;
        int w = GameView.view.getWidth();
        int h = GameView.view.getHeight();
        initLayers(Layer.COUNT.ordinal());

        add(Layer.bg, new ImageObject(R.mipmap.title,w/2, h/2 , 1.25f));
    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if (e.getAction() == MotionEvent.ACTION_DOWN) {

            MainGame.get().push(new MainScene());
        }
        return super.onTouchEvent(e);
    }
}
