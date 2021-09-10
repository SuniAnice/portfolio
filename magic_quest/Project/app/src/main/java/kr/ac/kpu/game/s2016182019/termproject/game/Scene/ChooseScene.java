package kr.ac.kpu.game.s2016182019.termproject.game.Scene;

import android.util.Log;
import android.view.MotionEvent;

import java.util.Random;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.MainGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;
import kr.ac.kpu.game.s2016182019.termproject.game.UI.Text;
import kr.ac.kpu.game.s2016182019.termproject.game.object.Block;
import kr.ac.kpu.game.s2016182019.termproject.game.object.ImageObject;

public class ChooseScene extends Scene {
    private static final String TAG = ChooseScene.class.getSimpleName();
    private Text playerhp;
    private ImageObject health;

    enum Layer {
        bg, COUNT
    }
    public static ChooseScene scene;
    public void add(ChooseScene.Layer layer, GameObject obj) {
        add(layer.ordinal(), obj);
    }
    @Override
    public void start() {
        super.start();
        transparent = true;
        int w = GameView.view.getWidth();
        int h = GameView.view.getHeight();
        initLayers(TitleScene.Layer.COUNT.ordinal());

        health = new ImageObject(R.mipmap.health, 1200 * 1.33f, 900 * 1.33f, 1.f);
        playerhp = new Text(1500,875);
        playerhp.setNum(MainScene.scene.player.hp);

        add(Layer.bg, new ImageObject(R.mipmap.choose,w/2, h/2 , 0.8f));
        add(Layer.bg, playerhp);
        add(Layer.bg, health);
    }


    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if (e.getAction() == MotionEvent.ACTION_DOWN) {
            if (e.getY() > 200 * 1.33f && e.getY() < 900 * 1.33f) {
                if (e.getX() > 300 * 1.33f && e.getX() < 950 * 1.33f) {
                    Random r = new Random();
                    MainScene.scene.enemy.initialize(r.nextInt(3), MainScene.scene.enemy.wave++);
                    MainScene.scene.player.initialize();
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            if (MainScene.scene.board.blocks[i][j] != null) {
                                MainScene.scene.board.blocks[i][j].change(Block.blockType.values()[r.nextInt(7)]);
                            }
                        }
                    }
                    MainGame.get().popScene();
                }
                else if (e.getX() > 1050 * 1.33f && e.getX() < 1730 * 1.33f) {
                    if (MainScene.scene.player.hp != MainScene.scene.player.maxHp) {
                        MainScene.scene.player.hp = MainScene.scene.player.maxHp;
                        MainScene.scene.enemy.wave += 2;
                        playerhp.setNum(MainScene.scene.player.hp);
                    }
                }
            }
            Log.d(TAG, "x : " + e.getX() + "y : " + e.getY());
        }
        return true;
    }
}
