package kr.ac.kpu.game.s2016182019.termproject.game.Scene;

import android.view.MotionEvent;

import java.util.ArrayList;
import java.util.HashMap;

import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;
import kr.ac.kpu.game.s2016182019.termproject.game.object.Board;
import kr.ac.kpu.game.s2016182019.termproject.game.object.Enemy;
import kr.ac.kpu.game.s2016182019.termproject.game.object.Player;


public class MainScene extends Scene {

    // singleton
    public float frameTime;
    public static MainScene scene;


    private  boolean initialized;
    public Board board;
    public Player player;
    public Enemy enemy;



    private static ArrayList<ArrayList<GameObject>> layers;

    private static HashMap<Class, ArrayList<GameObject>> recycleBin = new HashMap<>();


    public kr.ac.kpu.game.s2016182019.termproject.framework.GameObject get(Class clazz) {
        ArrayList<GameObject> array = recycleBin.get(clazz);
        if (array == null || array.isEmpty()) return null;
        return array.remove(0);
    }

    @Override
    public void start() {
        scene = this;
        super.start();
        initResources();

    }

    public boolean initResources() {
        if (initialized) {
            return false;
        }
        int w = GameView.view.getWidth();
        int h = GameView.view.getHeight();

        initLayers(Layer.ENEMY_COUNT.ordinal());

        board = new Board();
        add(Layer.board, board);

        player = new Player();
        add(Layer.player, player);

        enemy = new Enemy();
        add(Layer.player, enemy);

        player.initialize();
        player.enemy = enemy;



        initialized = true;

        return true;
    }

    public ArrayList<GameObject> objectsAt(int layerIndex) {
        return layers.get(layerIndex);
    }

    public ArrayList<GameObject> objectsAt(Layer layer) {
        return objectsAt(layer.ordinal());
    }

    public enum Layer {
        bg,  player, board ,ui, controller, effect, ENEMY_COUNT
    }



    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getAction();
        if (action == MotionEvent.ACTION_DOWN) {
            if (!board.onTouchEvent(event)){
                player.onTouchEvent(event);
            }
            return true;
        }
        return false;
    }

    public void add(Layer layer, GameObject obj) {
        add(layer.ordinal(), obj);
    }

    public boolean handleBackKey() { return true; }
}
