package kr.ac.kpu.game.s2016182019.termproject.game.Scene;

import android.view.MotionEvent;

import java.util.Random;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.BoxCollidable;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.MainGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;
import kr.ac.kpu.game.s2016182019.termproject.game.object.ImageObject;
import kr.ac.kpu.game.s2016182019.termproject.game.object.ItemUI;
import kr.ac.kpu.game.s2016182019.termproject.utils.CollisionHelper;

public class ItemScene extends Scene {

    enum Layer {
        bg, UI, COUNT
    }
    public static ItemScene scene;
    public void add(ItemScene.Layer layer, GameObject obj) {
        add(layer.ordinal(), obj);
    }

    @Override
    public void start() {
        super.start();
        transparent = true;
        int w = GameView.view.getWidth();
        int h = GameView.view.getHeight();
        initLayers(ItemScene.Layer.COUNT.ordinal());

        add(ItemScene.Layer.bg, new ImageObject(R.mipmap.itemscene,w/2, h/2 , 0.8f));

        Random r = new Random();
        add(Layer.UI, new ItemUI((int) ((w / 2 ) / 1.33f)- 500, 0, r.nextInt(ItemUI.items.length), (int) ((h/2 )/1.33f)+ 100));
        add(Layer.UI, new ItemUI((int) ((w / 2) / 1.33f), 0, r.nextInt(ItemUI.items.length), (int) ((h/2 )/1.33f)+ 100));
        add(Layer.UI, new ItemUI((int) ((w / 2 ) / 1.33f)+ 500, 0, r.nextInt(ItemUI.items.length), (int) ((h/2 )/1.33f)+ 100));


    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if (e.getAction() == MotionEvent.ACTION_DOWN) {

            for (GameObject obj :objectsAt(1)) {
                if (CollisionHelper.collides((BoxCollidable) obj, e.getX(), e.getY())){
                    ((ItemUI)obj).get();
                    MainGame.get().popScene();
                    MainGame.get().push(new ChooseScene());
                    break;
                }
            }


        }
        return super.onTouchEvent(e);
    }
}
