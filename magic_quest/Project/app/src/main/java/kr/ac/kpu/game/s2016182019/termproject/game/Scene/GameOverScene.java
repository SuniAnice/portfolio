package kr.ac.kpu.game.s2016182019.termproject.game.Scene;

import android.view.MotionEvent;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.MainGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;
import kr.ac.kpu.game.s2016182019.termproject.game.UI.Text;
import kr.ac.kpu.game.s2016182019.termproject.game.object.ImageObject;

public class GameOverScene extends Scene {

    private Text scoretext;

    enum Layer {
        bg, COUNT
    }
    public static GameOverScene scene;
    public void add(GameOverScene.Layer layer, GameObject obj) {
        add(layer.ordinal(), obj);
    }
    @Override
    public void start() {
        super.start();
        transparent = true;
        int w = GameView.view.getWidth();
        int h = GameView.view.getHeight();
        initLayers(TitleScene.Layer.COUNT.ordinal());

        scoretext = new Text((int) (w / 2 / 1.33f), 0, h / 2 + 50);
        scoretext.setNum(MainScene.scene.player.score);
        add(Layer.bg, new ImageObject(R.mipmap.gameover,w/2, 0 , 0.8f, h/2));
        add(Layer.bg, scoretext);

    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if (e.getAction() == MotionEvent.ACTION_DOWN) {
            MainGame.get().popScene();
            MainGame.get().popScene();
        }
        return super.onTouchEvent(e);
    }
}
